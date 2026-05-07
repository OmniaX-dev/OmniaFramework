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

#include <ostd/data/BaseObject.hpp>
#include <ostd/math/Geometry.hpp>
#include <ogfx/utils/Keycodes.hpp>

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
		public: enum class eScrollDirection { None = 0, Up, Down, Left, Right };
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
			Vec2 scrollAmount { 0 };
			gui::Widget* mousePressedOnWidget { nullptr };
			WindowCore& parentWindow;
	};
	class KeyEventData : public ostd::BaseObject
	{
		public: struct KeyModifiers
		{
			bool lshift { false };
			bool rshift { false };
			bool ctrl   { false };
			bool alt    { false };
			bool meta   { false };  // Win/Cmd/Super

			inline bool any(void) const { return rshift || lshift || ctrl || alt || meta; }
			inline bool anyShift(void) const { return lshift || rshift; }
			inline bool shiftOnly(void) const { return (lshift || rshift) && !ctrl && !alt && !meta; }
			inline bool ctrlOnly(void)  const { return ctrl && !lshift && !rshift && !alt && !meta; }
			inline bool metaOnly(void)  const { return meta && !lshift && !rshift && !alt && !ctrl; }
			inline bool altOnly(void)  const { return alt && !lshift && !rshift && !meta && !ctrl; }
			inline bool primaryOnly(void) const
			{
				#ifdef MAC_OS
					return metaOnly();
				#else
					return ctrlOnly();
				#endif
			}
			inline bool primaryShiftOnly(void) const
			{
			#ifdef MAC_OS
				return meta && anyShift() && !ctrl && !alt;
			#else
				return ctrl && anyShift() && !meta && !alt;
			#endif
			}
			inline bool primaryAltOnly(void) const
			{
			#ifdef MAC_OS
				return meta && alt && !ctrl && !anyShift();
			#else
				return ctrl && alt && !ctrl && !anyShift();
			#endif
			}
			inline bool shiftAltOnly(void) const
			{
				return alt && anyShift() && !ctrl && !meta;
			}
			inline bool primary(void) const
			{
			#ifdef MAC_OS
				return meta;
			#else
				return ctrl;
			#endif
			}
		};
		public: enum class eKeyEvent { Pressed = 0, Released, Text };
		public:
			inline KeyEventData(WindowCore& parent, i32 key, const String& _text, eKeyEvent evt, const KeyModifiers& mod) : parentWindow(parent), keyCode(key), text(_text), eventType(evt), modifiers(mod)
			{
				setTypeName("ogfx::KeyEventData");
				validate();
			}
			inline bool isCopy(void) const { return eventType == eKeyEvent::Pressed && modifiers.primaryOnly() && keyCode == KeyCode::C; }
			inline bool isCut(void) const { return eventType == eKeyEvent::Pressed && modifiers.primaryOnly() && keyCode == KeyCode::X; }
			inline bool isPaste(void) const { return eventType == eKeyEvent::Pressed && modifiers.primaryOnly() && keyCode == KeyCode::V; }
			inline bool isSelectAll(void) const { return eventType == eKeyEvent::Pressed && modifiers.primaryOnly() && keyCode == KeyCode::A; }
			inline bool isUndo(void) const { return eventType == eKeyEvent::Pressed && modifiers.primaryOnly() && keyCode == KeyCode::Z; }
			inline bool isRedo(void) const { return eventType == eKeyEvent::Pressed && ((modifiers.primaryShiftOnly() && keyCode == KeyCode::Z) || (modifiers.primaryOnly() && keyCode == KeyCode::Y)); }

		public:
			i32 keyCode;
			String text;
			eKeyEvent eventType;
			KeyModifiers modifiers;
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
