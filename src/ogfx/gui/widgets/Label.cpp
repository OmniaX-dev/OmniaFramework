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

#include "Label.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			Label& Label::create(const ostd::String& text)
			{
				setText(text);
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Label");
				disableDrawBox();
				disableChildren();
				enableBackground(false);
				validate();
				return *this;
			}

			void Label::applyTheme(const ostd::Stylesheet& theme)
			{
				setColor(getThemeValue<ostd::Color>(theme, "label.textColor", ostd::Colors::White));
				setBackGroundColor(getThemeValue<ostd::Color>(theme, "label.backgroundColor", ostd::Colors::Transparent));
				setFontSize(getThemeValue<int32_t>(theme, "label.fontSize", 20));
				m_borderRadius = getThemeValue<int32_t>(theme, "label.borderRadius", 10);
				m_borderWidth = getThemeValue<int32_t>(theme, "label.borderWidth", 2);
				m_showBorder = getThemeValue<bool>(theme, "label.showBorder", false);
				m_borderColor = getThemeValue<ostd::Color>(theme, "label.borderColor", ostd::Colors::White);
				enableBackground(getThemeValue<bool>(theme, "label.showBackground", false));
				setPadding(getThemeValue<ostd::Rectangle>(theme, "label.padding", { 5, 5, 5, 5 }));
				setMargin(getThemeValue<ostd::Rectangle>(theme, "label.margin", { 0, 0, 0, 0 }));
			}

			void Label::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_textChanged)
					__update_size(gfx);
				if (m_showBackground)
					gfx.fillRoundRect({ getGlobalPosition(), getSize() }, m_backgroundColor, m_borderRadius);
				if (m_showBorder)
					gfx.drawRoundRect({ getGlobalPosition(), getSize() }, m_borderColor, m_borderRadius, m_borderWidth);
				gfx.drawString(getText(), getGlobalPosition() + ostd::Vec2 { getPadding().left(), getPadding().top() }, getColor(), getFontSize());
			}

			void Label::setText(const ostd::String& text)
			{
				m_text = text;
				m_textChanged = true;
			}

			void Label::__update_size(ogfx::BasicRenderer2D& gfx)
			{
				auto size = gfx.getStringSize(getText(), getFontSize());
				size.x += getPadding().left();
				size.x += getPadding().right();
				size.y += getPadding().top();
				size.y += getPadding().bottom();
				setSize({ static_cast<float>(size.x), static_cast<float>(size.y) });
				m_textChanged = false;
			}
		}
	}
}
