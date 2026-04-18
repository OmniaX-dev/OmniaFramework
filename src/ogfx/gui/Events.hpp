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

#include <ostd/data/BaseObject.hpp>

namespace ogfx
{
	class WindowCore;
	namespace gui
	{
		class Widget;
	}
	class WindowResizedData : public ostd::BaseObject
	{
		public:
			inline WindowResizedData(WindowCore& parent, i32 _oldx, i32 _oldy, i32 _newx, i32 _newy) : parentWindow(parent), old_width(_oldx), old_height(_oldy), new_width(_newx), new_height(_newy)
			{
				setTypeName("ogfx::WindowResizedData");
				validate();
			}

		public:
			i32 new_width;
			i32 new_height;
			i32 old_width;
			i32 old_height;
			WindowCore& parentWindow;
	};
	class MouseEventData : public ostd::BaseObject
	{
		public: enum class eButton { None = 0, Left, Middle, Right };
		public: enum class eScrollDirection { None = 0, Up, Down };
		public:
			inline MouseEventData(WindowCore& parent, f32 mousex, f32 mousey, eButton btn) : parentWindow(parent), position_x(mousex), position_y(mousey), button(btn)
			{
				setTypeName("ogfx::MouseEventData");
				validate();
			}

		public:
			f32 position_x;
			f32 position_y;
			eButton button;
			eScrollDirection scroll { eScrollDirection::None };
			f32 scrollAmount { 0 };
			gui::Widget* mousePressedOnWidget { nullptr };
			WindowCore& parentWindow;
	};
	class KeyEventData : public ostd::BaseObject
	{
		public: enum class eKeyEvent { Pressed = 0, Released, Text };
		public:
			inline KeyEventData(WindowCore& parent, i32 key, char _text, eKeyEvent evt) : parentWindow(parent), keyCode(key), text(_text), eventType(evt)
			{
				setTypeName("ogfx::KeyEventData");
				validate();
			}

		public:
			i32 keyCode;
			char text;
			eKeyEvent eventType;
			WindowCore& parentWindow;
	};
	class DropEventData : public ostd::BaseObject
	{
		public: enum class eDropType { None = 0, File, Text, InApp };
		public:
			inline DropEventData(WindowCore& parent, eDropType type) : parentWindow(parent), dropType(type)
			{
				setTypeName("ogfx::DropEventData");
				validate();
			}

		public:
			eDropType dropType;
			WindowCore& parentWindow;
			String textOrFilePath { "" };
			ostd::BaseObject* userObject { nullptr };
	};
	namespace gui
	{
		class Event
		{
			public:
				inline Event(WindowCore& _window) : window(_window), drop(window, DropEventData::eDropType::None) {  }
				inline void handle(void) const { m_handled = true; }
				inline bool isHandled(void) const { return m_handled; }

			public:
				WindowCore& window;
				WindowResizedData* windowResized { nullptr };
				MouseEventData* mouse { nullptr };
				KeyEventData* keyboard { nullptr };
				DropEventData drop;
				u32 __original_signal_id { 0 };

			private:
				mutable bool m_handled { false };
		};
	}
}
