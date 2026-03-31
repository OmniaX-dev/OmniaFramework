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
	class WindowResizedData : public ostd::BaseObject
	{
		public:
			inline WindowResizedData(WindowCore& parent, int32_t _oldx, int32_t _oldy, int32_t _newx, int32_t _newy) : parentWindow(parent), old_width(_oldx), old_height(_oldy), new_width(_newx), new_height(_newy)
			{
				setTypeName("ogfx::WindowResizedData");
				validate();
			}

		public:
			int32_t new_width;
			int32_t new_height;
			int32_t old_width;
			int32_t old_height;
			WindowCore& parentWindow;
	};
	class MouseEventData : public ostd::BaseObject
	{
		public: enum class eButton { None = 0, Left, Middle, Right };
		public:
			inline MouseEventData(WindowCore& parent, float mousex, float mousey, eButton btn) : parentWindow(parent), position_x(mousex), position_y(mousey), button(btn)
			{
				setTypeName("ogfx::MouseEventData");
				validate();
			}

		public:
			float position_x;
			float position_y;
			eButton button;
			WindowCore& parentWindow;
	};
	class KeyEventData : public ostd::BaseObject
	{
		public: enum class eKeyEvent { Pressed = 0, Released, Text };
		public:
			inline KeyEventData(WindowCore& parent, int32_t key, char _text, eKeyEvent evt) : parentWindow(parent), keyCode(key), text(_text), eventType(evt)
			{
				setTypeName("ogfx::KeyEventData");
				validate();
			}

		public:
			int32_t keyCode;
			char text;
			eKeyEvent eventType;
			WindowCore& parentWindow;
	};
}
