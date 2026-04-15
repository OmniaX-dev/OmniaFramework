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
		namespace widgets
		{

			class Label : public Widget
			{
				public:
					inline Label(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
					inline Label(WindowCore& window, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					Label& create(const String& text);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void setText(const String& text);
					inline String getText(void) const { return m_text; }
					inline ostd::Color getColor(void) const { return m_color; }
					inline void setColor(const ostd::Color& color) { m_color = color; }
					inline i32 getFontSize(void) const { return m_fontSize; }
					inline void setFontSize(i32 fontSize) { m_fontSize = fontSize; }

				private:
					void __update_size(ogfx::BasicRenderer2D& gfx);

				private:
					String m_text { "" };
					bool m_textChanged { false };
					i32 m_fontSize { 20 };
					ostd::Color m_color { 255, 255, 255 };
			};
		}
	}
}
