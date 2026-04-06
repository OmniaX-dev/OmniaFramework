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

#include <ostd/string/String.hpp>

namespace ostd
{
	class Stylesheet;
}
namespace ogfx
{
	class BasicRenderer2D;
	class WindowCore;
	namespace gui
	{
		class Event;
		class Widget;
		class WidgetManager
		{
			public:
				WidgetManager(WindowCore& window, Widget& owner);
				bool hasWidget(Widget& widget);
				bool requestFocus(Widget& widget);
				bool addWidget(Widget& widget);
				Widget* focusNext(void);

				void draw(ogfx::BasicRenderer2D& gfx);
				void update(void);
				void onThemeApplied(const ostd::Stylesheet& theme);
				void onMousePressed(const Event& event);
				void onMouseReleased(const Event& event);
				void onMouseMoved(const Event& event);
				void onMouseDragged(const Event& event);
				void onKeyPressed(const Event& event);
				void onKeyReleased(const Event& event);
				void onTextEntered(const Event& event);
				void onWindowClosed(const Event& event);
				void onWindowResized(const Event& event);
				void onWindowFocused(const Event& event);
				void onWindowFocusLost(const Event& event);

				inline int32_t widgetCount(void) const { return m_widgetList.size(); }
				inline WindowCore& getWindow(void) { return m_window; }

			private:
				void processDragAndDrop(Widget* widget, const Event& event);

			private:
				WindowCore& m_window;
				Widget& m_owner;
				std::vector<Widget*> m_widgetList;
				Widget* m_focused { nullptr };
				bool m_tabNavigationEnabled { true };
				Widget* m_mousePressedOnWidget { nullptr };
		};
	}
}
