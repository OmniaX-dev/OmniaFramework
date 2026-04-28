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

#include "RadioButton.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		RadioButton& RadioButton::create(RadioButtonGroup& group, const String& text)
		{
			m_radioGroup = &group;
			setText(text);
			setPadding({ 5, 5, 5, 5 });
			setTypeName("ogfx::gui::RadioButton");
			disableChildren();
			enableBackground(false);
			setStylesheetCategoryName("radioButton");
			validate();
			return *this;
		}

		void RadioButton::applyTheme(const ostd::Stylesheet& theme)
		{
			setInnerCircleColor(getThemeValue<Color>(theme, "innerCircleColor", m_innerCircleColor));
			setOuterCircleColor(getThemeValue<Color>(theme, "outerCircleColor", m_outerCircleColor));
			m_outerCircleBorderWidth = getThemeValue<i32>(theme, "outerCircleBorderWidth", m_outerCircleBorderWidth);
		}

		void RadioButton::onMouseReleased(const Event& event)
		{
			if (!isMouseInside())
				return;
			if (m_radioGroup == nullptr)
				return;
			m_radioGroup->set_selected(*this);
		}

		void RadioButton::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			if (m_textChanged)
				__update_size(gfx);
			gfx.drawCircle({ getGlobalContentPosition() + Vec2 { 0, 4 }, m_circleSize }, getOuterCircleColor(), getOuterCircleBorderWidth());
			if (isSelected())
				gfx.fillCircle({ getGlobalContentPosition() + Vec2 { 3, 7 }, m_circleSize - 6 }, getInnerCircleColor());
			gfx.drawString(getText(), getGlobalContentPosition() + Vec2 { m_circleSize.x + m_spacing, 0 }, getTextColor(), getFontSize());
		}

		void RadioButton::setText(const String& text)
		{
			m_text = text;
			m_textChanged = true;
		}

		void RadioButton::__update_size(ogfx::BasicRenderer2D& gfx)
		{
			auto size = gfx.getStringDimensions(getText(), getFontSize());
			m_circleSize = { cast<f32>(size.y - 6), cast<f32>(size.y - 6) };
			size.x += m_spacing + m_circleSize.x;
			size.x += getPadding().left();
			size.x += getPadding().right();
			size.y += getPadding().top();
			size.y += getPadding().bottom();
			setSize({ cast<f32>(size.x), cast<f32>(size.y) });
			m_textChanged = false;
		}

		void RadioButton::__set_selected(bool selected)
		{
			if (m_selected == selected)
				return;
			m_selected = selected;
			setThemeQualifier("active", m_selected);
		}




		RadioButtonGroup::RadioButtonGroup(void)
		{

		}

		RadioButton& RadioButtonGroup::addButton(Widget& parent, const String& text, const Vec2& position)
		{
			m_buttons.push_back(std::unique_ptr<RadioButton>(
				new RadioButton(parent.getWindow(), *this, text)
			));
			auto& btn = *m_buttons.back();
			parent.addWidget(btn, position);
			if (m_buttons.size() == 1)
				set_selected(btn);
			return btn;
		}

		void RadioButtonGroup::set_selected(RadioButton& sender)
		{
			if (is_button_present(sender))
			{
				if (m_selected != nullptr && m_selected == &sender)
					return;
				unselect_all();
				auto previous = m_selected;
				m_selected = &sender;
				m_selected->__set_selected(true);
				if (previous != nullptr && callback_onSelectionChanged != nullptr)
					callback_onSelectionChanged(*previous, sender);
				return;
			}
		}

		bool RadioButtonGroup::is_button_present(RadioButton& button)
		{
			for (auto& btn : m_buttons)
				if (btn.get() == &button)
					return true;
			return false;
		}

		void RadioButtonGroup::unselect_all(void)
		{
			for (auto& btn : m_buttons)
				btn->__set_selected(false);
		}
	}
}
