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
#include <ogfx/gui/widgets/Button.hpp>
#include <deque>

namespace ogfx
{
	namespace gui
	{
		class Window;
		class ToolBar : public Widget
		{
			public:
				ToolBar(Window& window);
				ToolBar& create(void);
				widgets::Button& addButton(const String& iconPath, const String& text = "", EventCallback callback = nullptr);
				void onWindowResized(const Event& event) override;
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(BasicRenderer2D& gfx) override;
				void onUpdate(void) override;
				void onMousePressed(const Event& event) override;
				void onMouseMoved(const Event& event) override;

				inline void setw(f32 ww) override {  }
				inline void seth(f32 hh) override { h = hh; refresh_size(); }

				inline bool isVisible(void) const { return m_visible; }
				inline void show(bool v = true) { m_visible = v; }
				inline void hide(void) { show(false); }

				OSTD_PARAM_GETSET(f32, Height, m_height);
				OSTD_PARAM_GETSET(i32, FontSize, m_fontSize);
				OSTD_PARAM_GETSET(Color, BackgroundColor, m_backgroundColor);
				OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
				OSTD_PARAM_GETSET(Color, BorderColor, m_borderColor);
				OSTD_BOOL_PARAM_GETSET_E_NEG(ButtonText, m_disableButtonText);

			private:
				void refresh_size(void);

			private:
				bool m_visible { true };
				bool m_disableButtonText { true };
				std::deque<widgets::Button> m_buttons;

				f32 m_height { 26 };
				i32 m_fontSize { 16 };
				Color m_backgroundColor { "#6B0A1DFF" };
				Color m_textColor { "#F16A85FF" };
				Color m_borderColor { "#400000FF" };
		};
	}
}
