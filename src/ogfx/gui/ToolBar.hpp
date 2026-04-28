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
				ToolBar(Window& window, bool statusbar = false);
				ToolBar& create(bool statusbar = false);
				Button& addButton(const String& iconPath, const String& text = "", EventCallback callback = nullptr);
				void onWindowResized(const Event& event) override;
				void applyTheme(const ostd::Stylesheet& theme) override;
				void afterDraw(BasicRenderer2D& gfx) override;
				void onUpdate(void) override;

				inline void setw(f32 ww) override {  }
				inline void seth(f32 hh) override { h = hh; refresh_size(); }

				inline bool isStatusBar(void) const { return m_isStatusBar; }
				inline void setAsStatusBar(bool set = true) { m_isStatusBar = set; }

				OSTD_PARAM_GETSET(f32, Height, m_height);
				OSTD_BOOL_PARAM_GETSET_E_NEG(ButtonText, m_disableButtonText);
				OSTD_BOOL_PARAM_GETSET_E(BottomBorder, m_bottomBorder);

			private:
				void refresh_size(void);

			private:
				bool m_disableButtonText { true };
				std::deque<Button> m_buttons;
				bool m_isStatusBar { false };

				f32 m_height { 26 };
				bool m_bottomBorder { false };
		};
	}
}
