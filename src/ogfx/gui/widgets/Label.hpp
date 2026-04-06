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
					inline Label(WindowCore& window, const ostd::String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					Label& create(const ostd::String& text);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void setText(const ostd::String& text);
					inline ostd::String getText(void) const { return m_text; }
					inline ostd::Color getColor(void) const { return m_color; }
					inline void setColor(const ostd::Color& color) { m_color = color; }
					inline int32_t getFontSize(void) const { return m_fontSize; }
					inline void setFontSize(int32_t fontSize) { m_fontSize = fontSize; }
					inline void setBackGroundColor(const ostd::Color& color) { m_backgroundColor = color; }
					inline ostd::Color getBackgroundColor(void) { return m_backgroundColor; }
					inline void enableBackground(bool enable = true) { m_showBackground = enable; }
					inline bool isBackgoundEnabled(void) const { return m_showBackground; }

				private:
					void __update_size(ogfx::BasicRenderer2D& gfx);

				private:
					ostd::String m_text { "" };
					bool m_textChanged { false };
					int32_t m_fontSize { 20 };
					ostd::Color m_color { 255, 255, 255 };
					bool m_showBackground { false };
					ostd::Color m_backgroundColor { ostd::Colors::Transparent };

				private:
			};
		}
	}
}
