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

#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/ContextMenu.hpp>

namespace ogfx
{
	namespace gui
	{
		class ComboBox : public Widget
		{
			public:
				inline ComboBox(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				ComboBox& create(void);
				i32 addMenuItem(const String& text);
				bool removeMenuItem(const String& text);
				bool removeMenuItem(i32 id);
				void setItemCallback(ContextMenu::Instance::Callback callback);
				void setw(f32 w) override;
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMouseReleased(const Event& event) override;
				void handleSignal(ostd::Signal& signal) override;
				inline ContextMenu::Entry getSelectedEntry(void) const { return m_selectedEntry; }

				OSTD_PARAM_GETSET(Color, TriangleIndicatorColor, m_triangleColor);
				OSTD_PARAM_GETSET(f32, TriangleIndicatorPadding, m_trianglePadding);

			private:
				String get_display_text(const String& entry);

			private:
				bool m_dropDownShown { false };
				ContextMenu::Instance m_menu;
				i32 m_nextItemID { 0 };
				ContextMenu::Entry m_selectedEntry { "", -1 };
				String m_selectedEntryText { "" };  // Truncated text, not always valid, so can't be exposed

				Color m_triangleColor { "#008000FF" };
				f32 m_trianglePadding { 8.0f };
		};
	}
}
