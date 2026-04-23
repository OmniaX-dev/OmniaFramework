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
#include <ogfx/gui/widgets/Scrollbar.hpp>

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			class ListView : public ScrollableWidget
			{
				public: class Item : public ostd::__i_stringeable
				{
					public:
						inline Item(ListView& parent) { m_parent = &parent; }
						void setText(const String& text);
						void setFontSize(i32 fontSize);

						inline String toString(void) const override { return m_text; }
						inline bool isValid(void) const { return m_parent != nullptr && m_text.new_trim() != ""; }

						inline Vec2 getDimensions(void) const { return m_dimensions; }
						inline i32 getFontSize(void) const { return m_fontSize; }
						inline String getText(void) const { return m_text; }

						OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
						OSTD_PARAM_GETSET(Rectangle, Padding, m_padding);

					private:
						void update_dimensions(void);

					private:
						Vec2 m_dimensions { 0, 0 };
						ListView* m_parent { nullptr };
						String m_text { "" };
						i32 m_fontSize { 16 };
						Color m_textColor { 30, 30, 30 };
						Rectangle m_padding { 5, 5, 20, 0 };

				};
				public:
					inline ListView(WindowCore& window) : ScrollableWidget(window) { create(); }
					ListView& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					Rectangle getContentExtents(void) const override;
					void refresh(void) override;

				private:
					stdvec<Item> m_list;
			};
		}
	}
}
