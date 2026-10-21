/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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
#include <ogfx/gui/widgets/LayoutContainers.hpp>
#include <deque>

namespace ogfx
{
	namespace gui
	{
		class Window;
		// Buttons added with addButton() are arranged left to right by an internal
		// HBoxContainer (see getButtonBox()). Widgets added with addWidget() (e.g.
		// labels in a status bar) are regular children and can use anchors.
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

				// The bar's width always follows the window; only its height can change.
				inline void setw(f32 ww) override {  }
				inline void seth(f32 hh) override { setHeight(hh); }

				inline bool isStatusBar(void) const { return m_isStatusBar; }
				inline void setAsStatusBar(bool set = true) { m_isStatusBar = set; }

				// Re-reads the window size / menubar visibility and repositions the bar.
				void updateBounds(void);

				void setHeight(f32 height);
				inline f32 getHeight(void) const { return m_height; }

				inline HBoxContainer& getButtonBox(void) { return m_buttonBox; }
				inline void setButtonSpacing(f32 spacing) { m_buttonBox.setSpacing(spacing); }
				inline f32 getButtonSpacing(void) const { return m_buttonBox.getSpacing(); }
				OSTD_BOOL_PARAM_GETSET_E_NEG(ButtonText, m_disableButtonText);
				OSTD_BOOL_PARAM_GETSET_E(BottomBorder, m_bottomBorder);

			private:
				void refresh_size(void);
				void set_bar_size(f32 ww, f32 hh);

			private:
				HBoxContainer m_buttonBox { getWindow() };
				bool m_disableButtonText { true };
				std::deque<Button> m_buttons;
				bool m_isStatusBar { false };

				f32 m_height { 26 };
				bool m_bottomBorder { false };
		};
	}
}
