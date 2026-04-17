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

#include "Button.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			Button& Button::create(const String& text)
			{
				setText(text);
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Button");
				disableDrawBox();
				disableChildren();
				enableBackground(false);
				validate();
				return *this;
			}

			void Button::applyTheme(const ostd::Stylesheet& theme)
			{
				setTextColor(getThemeValue<Color>(theme, "button.textColor", Colors::White));
				setBackGroundColor(getThemeValue<Color>(theme, "button.backgroundColor", Colors::Transparent));
				setFontSize(getThemeValue<i32>(theme, "button.fontSize", 20));
				setBorderRadius(getThemeValue<i32>(theme, "button.borderRadius", 10));
				setBorderWidth(getThemeValue<i32>(theme, "button.borderWidth", 2));
				enableBorder(getThemeValue<bool>(theme, "button.showBorder", false));
				setBorderColor(getThemeValue<Color>(theme, "button.borderColor", Colors::White));
				enableBackground(getThemeValue<bool>(theme, "button.showBackground", false));
				setPadding(getThemeValue<Rectangle>(theme, "button.padding", { 5, 5, 5, 5 }));
				setMargin(getThemeValue<Rectangle>(theme, "button.margin", { 0, 0, 0, 0 }));
				m_useBackgroundGradient = getThemeValue<bool>(theme, "button.useBackgroundGradient", false);
				m_backgroundGradient = getThemeValue<ColorGradient>(theme, "button.backgroundGradient", m_backgroundGradient);
				if (m_useBackgroundGradient && isBackgoundEnabled())
					enableBackground(false);
			}

			void Button::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_textChanged)
					__update_size(gfx);
				if (m_useBackgroundGradient)
					gfx.fillGradientRect(getGlobalBounds(), m_backgroundGradient);
				gfx.drawString(getText(), getGlobalContentPosition(), getTextColor(), getFontSize());
			}

			void Button::setText(const String& text)
			{
				m_text = text;
				m_textChanged = true;
			}

			void Button::__update_size(ogfx::BasicRenderer2D& gfx)
			{
				auto size = gfx.getStringDimensions(getText(), getFontSize());
				size.x += getPadding().left();
				size.x += getPadding().right();
				size.y += getPadding().top();
				size.y += getPadding().bottom();
				setSize({ cast<f32>(size.x), cast<f32>(size.y) });
				m_textChanged = false;
			}
		}
	}
}
