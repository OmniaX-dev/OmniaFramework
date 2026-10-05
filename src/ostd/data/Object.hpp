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

#pragma once

#include <ostd/io/Serial.hpp>
#include <functional>
#include <atomic>

namespace ostd
{
	class OutputHandlerBase;

	// The original, unmodified BaseObject/signal design, preserved verbatim for projects that
	// already depend on its exact behavior. Not thread-safe. New projects should use ostd::Object
	// (below) instead - see its doc comment for why this one is being phased out.
	namespace legacy
	{
		struct Signal;
		class BaseObject : public I_stringeable, public serial::I_serializable
		{
			public: using SignalCallback = std::function<void(Signal&)>;
			public:
				BaseObject(const BaseObject& copy);
				inline virtual ~BaseObject(void) = default;
				virtual BaseObject& operator=(const BaseObject& copy);
				inline void setSignalCallback(SignalCallback callback) { callback_signal = callback; }

				virtual inline u64 getID(void) const { return m_uid; }
				virtual inline void setID(u64 id) { m_uid = id; }

				virtual inline bool isValid(void) const { return !isInvalid(); }
				virtual inline bool isInvalid(void) const { return !m_valid || m_oid == 0; }
				virtual inline void invalidate(void) { m_valid = false; }
				virtual inline void validate(void) { m_valid = true; }
				virtual inline void setValid(bool valid) { m_valid = valid; }

				inline u64 getCompareOID(void) const { return m_oid; }
				inline bool compareByOID(const BaseObject& other) const { return m_oid == other.m_oid; }

				inline static BaseObject& InvalidRef(void) { return BaseObject::s_invalid_obj; }

				inline void setTypeName(const String& tn) { m_typeName = tn; }
				inline String getTypeName(void) const { return m_typeName; }
				String getObjectHeaderString(void) const;

				inline bool signalsEnabled(void) { return m_signalsEnabled; }
				inline void enableSignals(bool e = true) { m_signalsEnabled = e; }

				virtual inline String toString(void) const override { return getObjectHeaderString(); };
				virtual void print(bool newLine = true, OutputHandlerBase* __destination = nullptr) const;

				virtual inline Serial::Stream serialize(void) const override { return { 0 }; };
				virtual inline bool deserialize(const Serial::Stream& data) override { return false; };

				virtual inline void handleSignal(Signal& signal) {  }
				void connectSignal(u32 signal_id);
				void __handle_signal(Signal& signal);

			protected:
				inline BaseObject(void) { m_uid = -1; m_valid = false; m_oid = BaseObject::s_next_oid++; }
			private:
				inline BaseObject(bool __valid) { m_uid = -1; m_valid = __valid; m_oid = 0; }

			private:
				u64 m_uid;
				u64 m_oid;
				bool m_valid;
				String m_typeName;
				bool m_signalsEnabled { true };
				SignalCallback callback_signal { nullptr };

				inline static u64 s_next_oid { 1024 };
				static BaseObject s_invalid_obj;
		};
	}

	struct Signal;

	// Modern replacement for legacy::BaseObject. Thread-safe for its own state (validity/enabled
	// flags are atomic), and pairs with the new ostd::SignalHandler (see Signals.hpp) which is
	// safe to connect/disconnect/emit from multiple threads concurrently.
	//
	// Deliberate differences from legacy::BaseObject (not oversights):
	//  - One identity (getID()) instead of two (uid+oid). It is assigned once at construction -
	//    including copy/move construction, which each get a FRESH id - and never changes via
	//    either assignment operator. If you need a separate, user-settable "external" id, add it
	//    in your derived class; the base stays minimal on purpose.
	//  - setTypeName()/getTypeName() use ostd::String, same as legacy - Object owns its own copy,
	//    so there's no lifetime requirement on what you pass in (a literal, a temporary, anything).
	//    Typical type names are short enough that String's small-string optimization keeps this
	//    allocation-free in practice anyway.
	//  - No stored signal callback. Connecting with a callback (see SignalHandler::connect) keeps
	//    that callback in the signal system's own connection record instead of inside every
	//    Object - the only thing Object itself carries for signals is the virtual handleSignal()
	//    hook, which costs nothing extra (the vtable pointer is already paid for).
	//  - No InvalidRef()-style mutable shared sentinel (that pattern is itself a threading hazard
	//    - a shared object handed out by non-const reference). Invalid(void) below returns a
	//    const& to an immutable sentinel instead - safe to read from any thread, since nothing can
	//    ever write to it.
	class Object : public I_stringeable, public serial::I_serializable
	{
		public: using SignalCallback = std::function<void(Signal&)>;
		public:
			inline Object(void) : m_id(s_next_id++) {  }
			inline Object(const Object& copy) : m_valid(copy.isValid()), m_signalsEnabled(copy.signalsEnabled()), m_id(s_next_id++), m_typeName(copy.m_typeName) {  }
			inline Object(Object&& move) noexcept : m_valid(move.isValid()), m_signalsEnabled(move.signalsEnabled()), m_id(s_next_id++), m_typeName(std::move(move.m_typeName)) { move.invalidate(); }
			virtual ~Object(void);

			inline Object& operator=(const Object& copy) { m_valid.store(copy.isValid()); m_signalsEnabled.store(copy.signalsEnabled()); m_typeName = copy.m_typeName; return *this; }
			inline Object& operator=(Object&& move) noexcept { m_valid.store(move.isValid()); m_signalsEnabled.store(move.signalsEnabled()); m_typeName = std::move(move.m_typeName); move.invalidate(); return *this; }

			inline bool operator==(const Object& other) const { return m_id == other.m_id; }
			inline bool operator!=(const Object& other) const { return m_id != other.m_id; }

			virtual inline u64 getID(void) const { return m_id; }

			virtual inline bool isValid(void) const { return m_valid.load(std::memory_order_relaxed); }
			virtual inline bool isInvalid(void) const { return !isValid(); }
			virtual inline void invalidate(void) { m_valid.store(false, std::memory_order_relaxed); }
			virtual inline void validate(void) { m_valid.store(true, std::memory_order_relaxed); }
			virtual inline void setValid(bool valid) { m_valid.store(valid, std::memory_order_relaxed); }

			inline bool signalsEnabled(void) const { return m_signalsEnabled.load(std::memory_order_relaxed); }
			inline void enableSignals(bool e = true) { m_signalsEnabled.store(e, std::memory_order_relaxed); }

			inline void setTypeName(const String& tn) { m_typeName = tn; }
			inline String getTypeName(void) const { return m_typeName; }
			String getObjectHeaderString(void) const;

			// A permanently-invalid, immutable, shared sentinel - safe to read from any thread
			// since nothing can ever mutate it. Use as a default where you'd otherwise need a
			// null reference, e.g. `void foo(const Object& data = Object::Invalid())`.
			static const Object& Invalid(void);

			virtual inline String toString(void) const override { return getObjectHeaderString(); }
			virtual void print(bool newLine = true, OutputHandlerBase* __destination = nullptr) const;

			virtual inline Serial::Stream serialize(void) const override { return { 0 }; }
			virtual inline bool deserialize(const Serial::Stream& data) override { return false; }

			// Called by SignalHandler during dispatch, always before any callback given to
			// connect() for this connection - override in a subclass as the no-callback way to
			// react to signals, exactly like legacy's handleSignal().
			virtual inline void handleSignal(Signal& signal) {  }

			// Convenience for the common case: connect *this by raw pointer (see Signals.hpp's
			// ostd::SignalHandler for the full API, including the shared_ptr-tracked overload).
			void connectSignal(u32 signal_id, SignalCallback cb = nullptr);

		private:
			inline Object(bool valid) : m_valid(valid), m_id(0) {  } // id 0 is reserved for Invalid() - never issued otherwise

		private:
			std::atomic<bool> m_valid { true };
			std::atomic<bool> m_signalsEnabled { true };
			const u64 m_id;
			String m_typeName { "" };

			inline static std::atomic<u64> s_next_id { 1024 };
	};
}
