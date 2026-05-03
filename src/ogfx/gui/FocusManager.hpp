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

#include <ogfx/gui/widgets/Widget.hpp>

namespace ogfx
{
	namespace gui
	{
		class Window;
		class FocusManager
		{
			public:
				FocusManager(Window& window);
				bool requestFocus(Widget& widget, Widget& root);
				bool requestFocus(Widget& widget);
				Widget* focusNext(void);

				bool registerTabIndex(Widget& widget, i32 index);
				void unregisterTabIndex(Widget& widget);

				void clearFocus(void);

				void onKeyReleased(const Event& event);

				inline Widget* getFocused(void) { return m_focused; }
				OSTD_BOOL_PARAM_GETSET_E(TabNavigation, m_tabNavigationEnabled);

			private:
				Window& m_window;
				Widget* m_focused { nullptr };
				bool m_tabNavigationEnabled { true };
				stdumap<Widget*, i32> m_tabIndexMap;
		};
	}
}
