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

#include "CheckBox.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			CheckBox& CheckBox::create(const ostd::String& text)
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

			void CheckBox::applyTheme(const ostd::Stylesheet& theme)
			{
				setCheckBorderColor(getThemeValue<ostd::Color>(theme, "checkbox.checkBorderColor", ostd::Colors::White));
				setCheckBoxColor(getThemeValue<ostd::Color>(theme, "checkbox.checkBoxColor", ostd::Colors::White));
				setTextColor(getThemeValue<ostd::Color>(theme, "checkbox.textColor", ostd::Colors::Black));
				setBackGroundColor(getThemeValue<ostd::Color>(theme, "checkbox.backgroundColor", ostd::Colors::Transparent));
				setFontSize(getThemeValue<int32_t>(theme, "checkbox.fontSize", 28));
				m_borderRadius = getThemeValue<int32_t>(theme, "checkbox.borderRadius", 10);
				m_borderWidth = getThemeValue<int32_t>(theme, "checkbox.borderWidth", 2);
				m_showBorder = getThemeValue<bool>(theme, "checkbox.showBorder", false);
				m_borderColor = getThemeValue<ostd::Color>(theme, "checkbox.borderColor", ostd::Colors::White);
				enableBackground(getThemeValue<bool>(theme, "checkbox.showBackground", false));
				setPadding(getThemeValue<ostd::Rectangle>(theme, "checkbox.padding", { 5, 5, 5, 5 }));
				setMargin(getThemeValue<ostd::Rectangle>(theme, "checkbox.margin", { 0, 0, 0, 0 }));
				m_checkBorderRadius = getThemeValue<int32_t>(theme, "checkbox.checkBorderRadius", 5);
				m_checkBorderWidth = getThemeValue<int32_t>(theme, "checkbox.checkBorderWidth", 1);
			}

			void CheckBox::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_textChanged)
					__update_size(gfx);
				if (m_showBackground)
					gfx.fillRoundRect({ getGlobalPosition(), getSize() }, m_backgroundColor, m_borderRadius);
				if (m_showBorder)
					gfx.drawRoundRect({ getGlobalPosition(), getSize() }, m_borderColor, m_borderRadius, m_borderWidth);
				gfx.drawRoundRect({ getGlobalContentPosition(), m_checkSize }, getCheckBorderColor(), m_checkBorderRadius, m_checkBorderWidth);
				if (isChecked())
					gfx.fillRoundRect({ getGlobalContentPosition() + 5, m_checkSize - 10 }, getCheckBoxColor(), m_checkBorderRadius / 2);
				gfx.drawString(getText(), getGlobalContentPosition() + ostd::Vec2 { m_checkSize.x + m_spacing, 0 }, getTextColor(), getFontSize());
			}

			void CheckBox::onMouseReleased(const Event& event)
			{
				m_checked = !m_checked;
				setThemeQualifier("active", m_checked);
			}

			void CheckBox::setText(const ostd::String& text)
			{
				m_text = text;
				m_textChanged = true;
			}

			void CheckBox::__update_size(ogfx::BasicRenderer2D& gfx)
			{
				auto size = gfx.getStringDimensions(getText(), getFontSize());
				m_checkSize = { static_cast<float>(size.y), static_cast<float>(size.y) };
				size.x += m_spacing + m_checkSize.x;
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
