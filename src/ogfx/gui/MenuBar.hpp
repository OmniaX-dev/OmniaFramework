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
		class Window;
		class MenuBar : public Rectangle
		{
			public: struct Menu
			{
				String label;
				ContextMenu::Instance instance;
			};

			public:
				inline MenuBar(Window& window) : m_window(window) { create(); }
				MenuBar& create(void);
				MenuBar& addMenu(const String& label, const ContextMenu::Instance& instance);
				MenuBar& addMenu(const Menu& menu);
				void clearMenus(void);

				void onWindowResized(const Event& event);
				void applyTheme(const ostd::Stylesheet& theme);
				void draw(BasicRenderer2D& gfx);
				void update(void);
				void onMousePressed(const Event& event);
				void onMouseMoved(const Event& event);

				inline bool isVisible(void) const { return m_visible; }
				inline void show(bool v = true) { m_visible = v; }
				inline void hide(void) { show(false); m_active = false; m_openIndex = -1; }
				inline bool isActive(void) const { return m_active; }

				OSTD_PARAM_GETSET(f32, Height, m_height);
				OSTD_PARAM_GETSET(Rectangle, ItemPadding, m_itemPadding);
				OSTD_PARAM_GETSET(i32, FontSize, m_fontSize);
				OSTD_PARAM_GETSET(Color, BackgroundColor, m_backgroundColor);
				OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
				OSTD_PARAM_GETSET(Color, SelectionColor, m_selectionColor);
				OSTD_PARAM_GETSET(Color, SelectionTextColor, m_selectionTextColor);
				OSTD_PARAM_GETSET(Color, BorderColor, m_borderColor);
				OSTD_BOOL_PARAM_GETSET_E(Border, m_showBorder);

			private:
				struct Slot
				{
					String label;
					ContextMenu::Instance instance;
					Rectangle rect;  // screen-space, recomputed on layout
				};

				void recompute_layout(void);
				i32  slot_index_at(const Vec2& mousePos) const;
				void open_menu(i32 index);
				void close_menu(void);

			private:
				Window& m_window;
				stdvec<Slot> m_slots;

				i32 m_openIndex { -1 };
				i32 m_hoveredIndex { -1 };
				bool m_active { false };
				bool m_visible { false };

				f32 m_height { 26 };
				Rectangle m_itemPadding { 12, 0, 12, 0 };  // x = left, y = top, w = right, h = bottom
				i32 m_fontSize { 16 };
				bool m_showBorder { false };
				Color m_backgroundColor { "#6B0A1DFF" };
				Color m_textColor { "#F16A85FF" };
				Color m_selectionColor { "#DC143CFF" };
				Color m_selectionTextColor { "#F16A85FF" };
				Color m_borderColor { "#400000FF" };
		};
	}
}
