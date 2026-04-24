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

namespace ogfx
{
	namespace gui
	{
		class ContextMenu : public Rectangle
		{
			public: struct Entry
			{
				inline Entry(const String& t, const stdvec<Entry>& sub = {}) { text = t; submenus = sub; }
				String text { "" };
				stdvec<Entry> submenus;

				private:
					Vec2 size;
					void update_size(BasicRenderer2D& gfx, i32 fontSize);

					friend class ContextMenu;
			};
			public: struct Instance
			{
				stdvec<Entry> entries;
			};
			public:
				inline ContextMenu(WindowCore& window) : m_window(window) { create(); }
				ContextMenu& create(void);
				void applyTheme(const ostd::Stylesheet& theme);
				void draw(BasicRenderer2D& gfx);
				void onMouseReleased(const Event& event);
				void onMouseMoved(const Event& event);
				void onMousePressed(const Event& event);
				void onMouseScrolled(const Event& event);
				void show(void);
				void show(const Vec2& pos);
				void hide(void);
				void setInstance(const Instance& instance);
				void setFontSize(i32 size);

				inline bool isVisible(void) const { return m_visible; }
				inline i32 getFontSize(void) const { return m_fontSize; }

				OSTD_PARAM_GETSET(Rectangle, Padding, m_padding);
				OSTD_PARAM_GETSET(f32, ItemSpacing, m_spacing);
				OSTD_PARAM_GETSET(Color, BackgroundColor, m_backgroundColor);
				OSTD_PARAM_GETSET(Color, SelectionColor, m_selectionColor);
				OSTD_PARAM_GETSET(Color, SelectionTextColor, m_selectionTextColor);
				OSTD_PARAM_GETSET(Color, SeparatorLineColor, m_separatorLineColor);
				OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
				OSTD_PARAM_GETSET(Color, SubmenuIndicatorColor, m_submenuIndicatorColor);
				OSTD_PARAM_GETSET(Color, BorderColor, m_borderColor);

			private:
				void update_size(void);

			private:
				WindowCore& m_window;
				bool m_visible { false };
				Instance m_data;
				f32 m_entryHeight { 0 };
				Vec2 m_mousePos { 0, 0 };

				Rectangle m_padding { 16, 0, 16, 0 };
				f32 m_spacing { 8 };
				i32 m_fontSize { 18 };
				Color m_backgroundColor { "#6B0A1DFF" };
				Color m_selectionColor { "#DC143CFF" };
				Color m_selectionTextColor { "#F16A85FF" };
				Color m_separatorLineColor { "#700000FF" };
				Color m_textColor { "#F16A85FF" };
				Color m_submenuIndicatorColor { "#111111FF" };
				Color m_borderColor { "#400000FF" };


				friend class Instance;
		};
	}
}
