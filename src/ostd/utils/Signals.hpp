/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2025  OmniaX-Dev

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
#include <ostd/data/BaseObject.hpp>



namespace ostd
{
	struct BuiltinSignals
	{
		inline static constexpr u32 NoSignal 				=	0x0000;

		/** Builtin Signals **/
		inline static constexpr u32 KeyPressed 			=	0x0001;
		inline static constexpr u32 KeyReleased 			=	0x0002;
		inline static constexpr u32 MousePressed 			=	0x0003;
		inline static constexpr u32 MouseReleased 			=	0x0004;
		inline static constexpr u32 MouseMoved 			=	0x0005;
		inline static constexpr u32 MouseDragged 			=	0x0006;
		inline static constexpr u32 TextEntered 			=	0x0007;
		inline static constexpr u32 MouseScrolled 			=	0x0008;

		inline static constexpr u32 OnGuiEvent				=	0x2001;
		inline static constexpr u32 FileDragAndDropped		=	0x2002;
		inline static constexpr u32 TextDragAndDropped		=	0x2003;

		inline static constexpr u32 BeforeSDLShutdown		=	0x3001;

		inline static constexpr u32 WindowResized 			=	0x1001;
		inline static constexpr u32 WindowClosed 			=	0x1002;
		inline static constexpr u32 WindowFocused 			=	0x1003;
		inline static constexpr u32 WindowLostFocus		=	0x1004;
		/*********************/

		inline static constexpr u32 CustomSignalBase 		=	0xFF0000;
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
			static void init(bool allow_reinit = false);
			static void handleDelegateSignals(void);
			static void emitSignal(u32 signal_id, u8 prio = Signal::Priority::RealTime, BaseObject& userData = BaseObject::InvalidRef());
			static void connect(BaseObject& object, u32 signal_id);
			static void disconnect(BaseObject& object, u32 signal_id);
			inline static u32 newCustomSignal(u32 sub_id) { return BuiltinSignals::CustomSignalBase + sub_id; }

		private:
			inline static stdumap<u32, stdvec<BaseObject*>> m_recievers;
			inline static stdvec<DelegateSignal> m_DelegateRecievers;
			inline static constexpr u16 __SIGNAL_BUFFER_START_SIZE { 2048 };
			inline static constexpr u16 __DELEGATE_SIGNALS_BUFFER_START_SIZE { 2048 };
			inline static bool m_initialized { false };
	};
}
