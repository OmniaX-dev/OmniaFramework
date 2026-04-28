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
		class RadioButtonGroup;
		class RadioButton : public Widget
		{
			public:
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onMouseReleased(const Event& event) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void setText(const String& text);
				inline String getText(void) const { return m_text; }
				inline bool isSelected(void) const { return m_selected; }
				OSTD_PARAM_GETSET(i32, OuterCircleBorderWidth, m_outerCircleBorderWidth);
				OSTD_PARAM_GETSET(Color, InnerCircleColor, m_innerCircleColor);
				OSTD_PARAM_GETSET(Color, OuterCircleColor, m_outerCircleColor);

			private:
				inline RadioButton(WindowCore& window, RadioButtonGroup& group, const String& text) : Widget({ 0, 0, 0, 0 }, window) { create(group, text); }
				RadioButton& create(RadioButtonGroup& group, const String& text);
				void __update_size(ogfx::BasicRenderer2D& gfx);
				void __set_selected(bool selected);

			private:
				RadioButtonGroup* m_radioGroup { nullptr };
				Vec2 m_circleSize { 0, 0 };
				bool m_selected { false };
				String m_text { "" };
				bool m_textChanged { false };
				f32 m_spacing { 6 };
				i32 m_outerCircleBorderWidth { 1 };
				Color m_innerCircleColor { 255, 255, 255 };
				Color m_outerCircleColor { 255, 255, 255 };

				friend class RadioButtonGroup;
		};
		class RadioButtonGroup
		{
			public:
				RadioButtonGroup(void);
				RadioButton& addButton(Widget& parent, const String& text, const Vec2& position = { 0, 0 });
				inline void setSelectionChangedCallback(std::function<void(RadioButton& previous, RadioButton& sender)> callback) { callback_onSelectionChanged = std::move(callback); }

			private:
				void set_selected(RadioButton& sender);
				bool is_button_present(RadioButton& button);
				void unselect_all(void);

			private:
				stdvec<std::unique_ptr<RadioButton>> m_buttons;
				RadioButton* m_selected { nullptr };
				std::function<void(RadioButton& previous, RadioButton& sender)> callback_onSelectionChanged { nullptr };

				friend class RadioButton;
		};
	}
}
