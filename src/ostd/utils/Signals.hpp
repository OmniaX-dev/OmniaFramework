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
#include <unordered_map>
#include <vector>

namespace ostd
{
	struct BuiltinSignals
	{
		inline static constexpr uint32_t NoSignal 				=	0x0000;

		/** Builtin Signals **/
		inline static constexpr uint32_t KeyPressed 			=	0x0001;
		inline static constexpr uint32_t KeyReleased 			=	0x0002;
		inline static constexpr uint32_t MousePressed 			=	0x0003;
		inline static constexpr uint32_t MouseReleased 			=	0x0004;
		inline static constexpr uint32_t MouseMoved 			=	0x0005;
		inline static constexpr uint32_t MouseDragged 			=	0x0006;
		inline static constexpr uint32_t TextEntered 			=	0x0007;

		inline static constexpr uint32_t OnGuiEvent				=	0x2001;

		inline static constexpr uint32_t BeforeSDLShutdown		=	0x3001;

		inline static constexpr uint32_t WindowResized 			=	0x1001;
		inline static constexpr uint32_t WindowClosed 			=	0x1002;
		inline static constexpr uint32_t WindowFocused 			=	0x1003;
		inline static constexpr uint32_t WindowLostFocus		=	0x1004;
		/*********************/

		inline static constexpr uint32_t CustomSignalBase 		=	0xFF0000;
	};

	struct Signal
	{
		struct Priority
		{
			inline static constexpr uint8_t RealTime = 0;
			inline static constexpr uint8_t Normal = 1;
		};
		const uint8_t priority;
		const uint32_t ID;
		bool handled { false };
		BaseObject& userData;

		inline Signal(uint32_t id, BaseObject& _userData = BaseObject::InvalidRef(), uint8_t prio = Signal::Priority::Normal) : priority(prio), ID(id), userData(_userData) {  }
	};

	class SignalHandler
	{
		private: struct DelegateSignal
		{
			uint32_t id;
			BaseObject& ud;
			inline DelegateSignal(uint32_t _id, BaseObject& _ud) : id(_id), ud(_ud) {  }
		};
		public:
			static void init(bool allow_reinit = false);
			static void handleDelegateSignals(void);
			static void emitSignal(uint32_t signal_id, uint8_t prio = Signal::Priority::RealTime, BaseObject& userData = BaseObject::InvalidRef());
			static void connect(BaseObject& object, uint32_t signal_id);
			inline static uint32_t newCustomSignal(uint32_t sub_id) { return BuiltinSignals::CustomSignalBase + sub_id; }

		private:
			inline static std::unordered_map<uint32_t, std::vector<BaseObject*>> m_recievers;
			inline static std::vector<DelegateSignal> m_DelegateRecievers;
			inline static constexpr uint16_t __SIGNAL_BUFFER_START_SIZE { 2048 };
			inline static constexpr uint16_t __DELEGATE_SIGNALS_BUFFER_START_SIZE { 2048 };
			inline static bool m_initialized { false };
	};
}
