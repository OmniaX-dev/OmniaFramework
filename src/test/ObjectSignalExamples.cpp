/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

	This file is part of OmniaFramework.

	OmniaFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	OmniaFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with OmniaFramework.  If not, see <https://www.gnu.org/licenses/>.
*/

// Worked examples for ostd::Object + ostd::SignalHandler (the modern replacement for
// ostd::legacy::BaseObject / ostd::legacy::SignalHandler). Not a unit test - a reference you can
// read top to bottom, or build and run to see it for yourself:
//
//   cmake --build bin --target ostd_test
//
// (after swapping which file is active in TEST_SOURCE_FILES in the top-level CMakeLists.txt -
// this file is listed there commented out, next to the other test/demo entries).

#include <ostd/data/Object.hpp>
#include <ostd/utils/Signals.hpp>
#include <iostream>
#include <memory>

using ostd::Object;
using ostd::Signal;
using ostd::SignalHandler;

// A custom signal id, the same way you'd declare one today with BuiltinSignals::CustomSignalBase.
static const u32 PriceChanged = SignalHandler::newCustomSignal(1);

// ============================================================================================
// Example 1: subclassing Object and overriding handleSignal() - the direct equivalent of
// subclassing legacy::BaseObject. Nothing about this path changes shape from legacy; it is still
// the "no callback, no shared_ptr, just inherit and override" default.
// ============================================================================================
class PriceDisplay : public Object
{
	public:
		inline PriceDisplay(const char* label) { setTypeName(label); }

		void handleSignal(Signal& signal) override
		{
			if (signal.ID != PriceChanged)
				return;
			// signal.userData is an Object* (not a reference bound to a shared sentinel like
			// legacy's BaseObject& userData) - nullptr just means "the emitter passed nothing",
			// no InvalidRef()-style object to compare against.
			if (signal.userData)
				std::cout << getTypeName() << ": price update from " << signal.userData->getTypeName() << "\n";
		}
};

void example_1_basic_subclass_connection(void)
{
	std::cout << "\n--- Example 1: subclass + handleSignal(), connected by raw reference ---\n";

	PriceDisplay ticker("ticker-display");
	PriceDisplay dashboard("dashboard-widget");

	// The default, frictionless path: connect(Object&, signal_id). Internally this stores a raw
	// Object*, same cost as legacy's connect(). Safe to call concurrently with other connects,
	// disconnects and emits on other threads - just not if THIS exact object is being destroyed
	// by another thread at the exact same moment something is mid-dispatch into it (see Example 3
	// for when that matters and what to do about it).
	SignalHandler::connect(ticker, PriceChanged);
	SignalHandler::connect(dashboard, PriceChanged);

	Object source;
	source.setTypeName("exchange-feed");
	SignalHandler::emitSignal(PriceChanged, Signal::Priority::RealTime, &source);

	// You don't have to remember to do this - ~Object() does it automatically - but calling it
	// explicitly frees the registry slot immediately instead of waiting for destruction.
	SignalHandler::disconnect(ticker, PriceChanged);
	SignalHandler::disconnect(dashboard, PriceChanged);
}

// ============================================================================================
// Example 2: connecting with a callback instead of subclassing - the equivalent of legacy's
// setSignalCallback(), except the callback is no longer stored ON the Object (that's the whole
// point of the footprint reduction). It lives in the connection record inside SignalHandler.
//
// Ordering guarantee: if the connected object ALSO overrides handleSignal(), that virtual call
// always runs first, and the callback (if any) always runs after it - never instead of it.
// ============================================================================================
void example_2_callback_connection(void)
{
	std::cout << "\n--- Example 2: callback connection (handleSignal still runs first) ---\n";

	Object sensor;
	sensor.setTypeName("thermostat");

	// sensor doesn't override handleSignal() here - Object's default is a no-op - so the
	// callback is the only thing that actually reacts. If it DID override handleSignal(), that
	// would fire first, then this callback, every time.
	SignalHandler::connect(sensor, PriceChanged, [](Signal& signal) {
		std::cout << "callback saw signal " << signal.ID << "\n";
	});

	SignalHandler::emitSignal(PriceChanged);
	SignalHandler::disconnect(sensor, PriceChanged);
}

// ============================================================================================
// Example 3: the shared_ptr-tracked connection - the ONE thing legacy BaseObject could not do
// safely. Use this specifically when an object connected to a signal might be destroyed on a
// DIFFERENT thread than the one emitting that signal, at a time you can't fully control or
// predict. The default (Example 1's) raw-pointer path is safe against other threads racing
// connect()/disconnect()/emitSignal() against each other, but not against this one specific
// scenario - that's what this overload is for.
// ============================================================================================
void example_3_shared_ptr_tracked_connection(void)
{
	std::cout << "\n--- Example 3: shared_ptr-tracked connection ---\n";

	// The object must already be shared_ptr-owned - that's the one real requirement of this path.
	auto worker = std::make_shared<PriceDisplay>("background-worker");

	// Note: this is SignalHandler::connect(shared_ptr<Object>, ...) - a different overload from
	// Example 1's connect(Object&, ...). Object itself needs no special support (no
	// enable_shared_from_this, nothing) - we just capture a std::weak_ptr from the shared_ptr you
	// already have.
	SignalHandler::connect(worker, PriceChanged, [](Signal&) {
		std::cout << "tracked worker handled the signal\n";
	});

	SignalHandler::emitSignal(PriceChanged); // worker is alive - this reaches it

	worker.reset(); // destroy the only owner - the registry now holds an expired weak_ptr

	// Safe: dispatch calls weak_ptr::lock(), sees the object is gone, and silently skips this
	// receiver instead of calling through a dangling pointer. No disconnect() call was needed.
	SignalHandler::emitSignal(PriceChanged);
	std::cout << "emitted again after destroying worker - no crash, receiver was just skipped\n";

	// What this does NOT give you: if two threads were racing - one destroying `worker` while
	// another was simultaneously CALLING emitSignal() - the lock() either succeeds (and keeps the
	// object alive for the duration of that one call, even if the destroying thread is already
	// trying to tear it down) or fails cleanly. There is no window where a partially-destroyed
	// object gets called into. That's the actual guarantee this path buys you over Example 1's.
}

// ============================================================================================
// Example 4: Object::Invalid() - the replacement for legacy's BaseObject::InvalidRef(). Same
// purpose (a permanently-invalid default for a reference parameter, so you don't need a nullable
// type everywhere), but it's a const&, so it's safe to share across threads - nothing can ever
// write through it, unlike legacy's mutable sentinel.
// ============================================================================================
void example_4_invalid_sentinel(void)
{
	std::cout << "\n--- Example 4: Object::Invalid() ---\n";

	auto describe = [](const Object& data) {
		if (data.isInvalid())
			std::cout << "no data provided\n";
		else
			std::cout << "data: " << data.getTypeName() << "\n";
	};

	describe(Object::Invalid()); // "no data provided"

	Object real;
	real.setTypeName("actual-payload");
	describe(real); // "data: actual-payload"
}

int main()
{
	example_1_basic_subclass_connection();
	example_2_callback_connection();
	example_3_shared_ptr_tracked_connection();
	example_4_invalid_sentinel();
	return 0;
}
