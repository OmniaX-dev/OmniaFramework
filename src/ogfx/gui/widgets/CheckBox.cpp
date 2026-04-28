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
		CheckBox& CheckBox::create(const String& text)
		{
			setText(text);
			setPadding({ 5, 5, 5, 5 });
			setTypeName("ogfx::gui::CheckBox");
			disableChildren();
			enableBackground(false);
			setStylesheetCategoryName("checkbox");
			validate();
			return *this;
		}

		void CheckBox::applyTheme(const ostd::Stylesheet& theme)
		{
			setCheckBorderColor(getThemeValue<Color>(theme, "checkBorderColor", Colors::White));
			setCheckBoxColor(getThemeValue<Color>(theme, "checkBoxColor", Colors::White));
			m_checkBorderRadius = getThemeValue<i32>(theme, "checkBorderRadius", 5);
			m_checkBorderWidth = getThemeValue<i32>(theme, "checkBorderWidth", 1);
		}

		void CheckBox::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			if (m_textChanged)
				__update_size(gfx);
			gfx.drawRoundRect({ getGlobalContentPosition(), m_checkSize }, getCheckBorderColor(), m_checkBorderRadius, m_checkBorderWidth);
			if (isChecked())
				gfx.fillRoundRect({ getGlobalContentPosition() + 5, m_checkSize - 10 }, getCheckBoxColor(), m_checkBorderRadius / 2.0f);
			gfx.drawString(getText(), getGlobalContentPosition() + Vec2 { m_checkSize.x + m_spacing, 0 }, getTextColor(), getFontSize());
		}

		void CheckBox::onMouseReleased(const Event& event)
		{
			if (!isMouseInside())
				return;
			setChecked(!isChecked());
		}

		void CheckBox::setText(const String& text)
		{
			m_text = text;
			m_textChanged = true;
		}

		void CheckBox::setChecked(bool checked)
		{
			if (m_checked == checked)
				return;
			m_checked = checked;
			setThemeQualifier("active", m_checked);
			if (callback_onStateChanged)
				callback_onStateChanged(*this, m_checked);
		}

		void CheckBox::__update_size(ogfx::BasicRenderer2D& gfx)
		{
			auto size = gfx.getStringDimensions(getText(), getFontSize());
			m_checkSize = { cast<f32>(size.y), cast<f32>(size.y) };
			size.x += m_spacing + m_checkSize.x;
			size.x += getPadding().left();
			size.x += getPadding().right();
			size.y += getPadding().top();
			size.y += getPadding().bottom();
			setSize({ cast<f32>(size.x), cast<f32>(size.y) });
			m_textChanged = false;
		}
	}
}
