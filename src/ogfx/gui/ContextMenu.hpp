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
				inline Entry(const String& t, const stdvec<Entry>& sub = {}) { text = t; }
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
				void draw(BasicRenderer2D& gfx);
				void onMouseReleased(const Event& event);
				void onMouseMoved(const Event& event);
				void show(void);
				void show(const Vec2& pos);
				void hide(void);
				void setInstance(const Instance& instance);
				void setFontSize(i32 size);

				inline bool isVisible(void) const { return m_visible; }
				inline i32 getFontSize(void) const { return m_fontSize; }

			private:
				void update_size(void);

			private:
				WindowCore& m_window;
				bool m_visible { false };
				Instance m_data;
				f32 m_entryHeight { 0 };
				Rectangle m_padding { 10, 10, 10, 10 };
				Vec2 m_mousePos;

				i32 m_fontSize { 18 };

				friend class Instance;
		};
	}
}
