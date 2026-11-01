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
#include <ogfx/gui/ContextMenu.hpp>

namespace ogfx
{
	namespace gui
	{
		class ComboBox : public Widget
		{
			public:
				inline ComboBox(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				ComboBox& create(void);
				i32 addMenuItem(const String& text);
				bool removeMenuItem(const String& text);
				bool removeMenuItem(i32 id);
				// Removes every item; the ids of the next ones start from 0 again.
				void clearMenuItems(void);
				// Shows the item as the selected one (the item callback isn't called).
				// Returns false if there is no item with that id.
				bool setSelectedItem(i32 id);
				inline i32 getItemCount(void) const { return cast<i32>(m_menu.entries.size()); }
				void setItemCallback(ContextMenu::Instance::Callback callback);
				void setw(f32 w) override;
				Vec2 getContentMinSize(void) const override;
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMouseReleased(const Event& event) override;
				void onUpdate(void) override;
				void handleSignal(ostd::Signal& signal) override;
				inline ContextMenu::Entry getSelectedEntry(void) const { return m_selectedEntry; }

				OSTD_PARAM_GETSET(Color, TriangleIndicatorColor, m_triangleColor);
				OSTD_PARAM_GETSET(f32, TriangleIndicatorPadding, m_trianglePadding);

			private:
				String get_display_text(const String& entry);
				// Selects the first item (or none, when empty).
				void select_first(void);

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
