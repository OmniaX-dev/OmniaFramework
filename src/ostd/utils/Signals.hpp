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

#include <ostd/data/Types.hpp>
#include <ostd/data/Object.hpp>
#include <memory>



namespace ostd
{
	// The original, unmodified signal system, preserved verbatim for projects that already
	// depend on its exact behavior. Not thread-safe - connect/disconnect/emitSignal must all
	// happen on the same thread. New projects should use ostd::SignalHandler (below) instead.
	namespace legacy
	{
		struct BuiltinSignals
		{
			inline static constexpr u32 NoSignal                 =    0x0000;

			/** Builtin Signals **/
			inline static constexpr u32 KeyPressed             =    0x0001;
			inline static constexpr u32 KeyReleased             =    0x0002;
			inline static constexpr u32 MousePressed             =    0x0003;
			inline static constexpr u32 MouseReleased             =    0x0004;
			inline static constexpr u32 MouseMoved             =    0x0005;
			inline static constexpr u32 MouseDragged             =    0x0006;
			inline static constexpr u32 TextEntered             =    0x0007;
			inline static constexpr u32 MouseScrolled             =    0x0008;

			inline static constexpr u32 OnGuiEvent                =    0x2001;
			inline static constexpr u32 FileDragAndDropped        =    0x2002;
			inline static constexpr u32 TextDragAndDropped        =    0x2003;

			inline static constexpr u32 BeforeSDLShutdown        =    0x3001;

			inline static constexpr u32 WindowResized             =    0x1001;
			inline static constexpr u32 WindowClosed             =    0x1002;
			inline static constexpr u32 WindowFocused             =    0x1003;
			inline static constexpr u32 WindowLostFocus        =    0x1004;
			/*********************/

			inline static constexpr u32 CustomSignalBase         =    0xFF0000;
		};

		struct Signal
		{
			struct Priority
			{
				inline static constexpr u8 RealTime = 0;
				inline static constexpr u8 Normal = 1;
			};
			const u8 priority;
			const u32 ID;
			bool handled { false };
			BaseObject& userData;

			inline Signal(u32 id, BaseObject& _userData = BaseObject::InvalidRef(), u8 prio = Signal::Priority::Normal) : priority(prio), ID(id), userData(_userData) {  }
		};

		class SignalHandler
		{
			private: struct DelegateSignal
			{
				u32 id;
				BaseObject& ud;
				inline DelegateSignal(u32 _id, BaseObject& _ud) : id(_id), ud(_ud) {  }
			};
			public:
				static void handleDelegateSignals(void);
				static void emitSignal(u32 signal_id, u8 prio = Signal::Priority::RealTime, BaseObject& userData = BaseObject::InvalidRef());
				static void connect(BaseObject& object, u32 signal_id);
				static void disconnect(BaseObject& object, u32 signal_id);
				inline static u32 newCustomSignal(u32 sub_id) { return BuiltinSignals::CustomSignalBase + sub_id; }

			private:
				inline static stdumap<u32, stdvec<BaseObject*>>& receivers(void)
				{
					static stdumap<u32, stdvec<BaseObject*>> m = [] {
						stdumap<u32, stdvec<BaseObject*>> map;
						map.reserve(__SIGNAL_BUFFER_START_SIZE);
						return map;
					}();
					return m;
				}

				inline static stdvec<DelegateSignal>& delegateReceivers(void)
				{
					static stdvec<DelegateSignal> v = [] {
						stdvec<DelegateSignal> vec;
						vec.reserve(__DELEGATE_SIGNALS_BUFFER_START_SIZE);
						return vec;
					}();
					return v;
				}

			private:
				inline static constexpr u16 __SIGNAL_BUFFER_START_SIZE { 2048 };
				inline static constexpr u16 __DELEGATE_SIGNALS_BUFFER_START_SIZE { 2048 };
		};
	}

	struct BuiltinSignals
	{
		inline static constexpr u32 NoSignal                  = 0x0000;

		inline static constexpr u32 KeyPressed                = 0x0001;
		inline static constexpr u32 KeyReleased               = 0x0002;
		inline static constexpr u32 MousePressed              = 0x0003;
		inline static constexpr u32 MouseReleased             = 0x0004;
		inline static constexpr u32 MouseMoved                = 0x0005;
		inline static constexpr u32 MouseDragged              = 0x0006;
		inline static constexpr u32 TextEntered               = 0x0007;
		inline static constexpr u32 MouseScrolled             = 0x0008;

		inline static constexpr u32 OnGuiEvent                = 0x2001;
		inline static constexpr u32 FileDragAndDropped        = 0x2002;
		inline static constexpr u32 TextDragAndDropped        = 0x2003;

		inline static constexpr u32 BeforeSDLShutdown         = 0x3001;

		inline static constexpr u32 WindowResized             = 0x1001;
		inline static constexpr u32 WindowClosed              = 0x1002;
		inline static constexpr u32 WindowFocused             = 0x1003;
		inline static constexpr u32 WindowLostFocus           = 0x1004;

		inline static constexpr u32 CustomSignalBase          = 0xFF0000;
	};

	struct Signal
	{
		struct Priority
		{
			inline static constexpr u8 RealTime = 0;
			inline static constexpr u8 Normal = 1;
		};
		const u8 priority;
		const u32 ID;
		bool handled { false };
		Object* userData { nullptr }; // nullptr means "no data" - no sentinel object needed

		inline Signal(u32 id, Object* _userData = nullptr, u8 prio = Signal::Priority::Normal) : priority(prio), ID(id), userData(_userData) {  }
	};

	// Thread-safe replacement for legacy::SignalHandler. connect/disconnect/emitSignal/
	// handleDelegateSignals may all be called concurrently from any thread.
	//
	// Two ways to connect, chosen by overload:
	//  - connect(Object&, ...)                     - the default. Zero-overhead, same shape as
	//    legacy. Safe against other threads connecting/disconnecting/emitting at the same time,
	//    but NOT against another thread destroying this exact Object at the exact moment a
	//    dispatch on a third thread is already calling into it - the usual "don't destroy
	//    something while something else might be using it" rule still applies, same as it always
	//    implicitly has.
	//  - connect(const std::shared_ptr<Object>&, ...) - for the rare case where you specifically
	//    need the stronger guarantee: the object is tracked by a weak_ptr, and dispatch calls
	//    weak_ptr::lock() to get a temporary, ref-counted, guaranteed-safe-to-use handle before
	//    calling in - if the object's already gone, that receiver is silently skipped instead of
	//    risking a use-after-free. Requires the object to already be shared_ptr-owned.
	// See examples/ObjectSignalExamples.cpp for both in use.
	class SignalHandler
	{
		public:
			static void handleDelegateSignals(void);
			static void emitSignal(u32 signal_id, u8 prio = Signal::Priority::RealTime, Object* userData = nullptr);
			static void connect(Object& object, u32 signal_id, Object::SignalCallback cb = nullptr);
			static void connect(const std::shared_ptr<Object>& object, u32 signal_id, Object::SignalCallback cb = nullptr);
			static void disconnect(Object& object, u32 signal_id);
			// Purges every connection referencing this object, across every signal id - called
			// automatically by ~Object(), so forgetting to disconnect() can no longer leave a
			// dangling raw-pointer entry behind.
			static void disconnectAll(Object& object);
			inline static u32 newCustomSignal(u32 sub_id) { return BuiltinSignals::CustomSignalBase + sub_id; }
	};
}
