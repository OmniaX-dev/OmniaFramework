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

#include "ToolBar.hpp"
#include "../render/BasicRenderer.hpp"
#include "Window.hpp"

namespace ogfx
{
	namespace gui
	{
		ToolBar::ToolBar(Window& window, bool statusbar) : Widget({ 0, 0, 0, 0 }, window)
		{
			create(statusbar);
		}

		ToolBar& ToolBar::create(bool statusbar)
		{
			setRootChild();
			setStylesheetCategoryName("toolbar");
			setTypeName("ogfx::gui::ToolBar");
			disableBorder();
			setAsStatusBar(statusbar);
			hide();
			m_buttonBox.setSpacing(0);
			addWidget(m_buttonBox);
			m_buttonBox.setAnchorPreset(eAnchorPreset::FullRect);
			updateBounds();
			return *this;
		}

		Button& ToolBar::addButton(const String& iconPath, const String& text, EventCallback callback)
		{
			m_buttons.push_back({ getWindow() });
			auto& btn = m_buttons.back();
			btn.addThemeID("tool_button");
			btn.setIcon(iconPath);
			btn.setText(text);
			btn.setSizeMode(eSizeMode::LayoutManaged);
			btn.setCallback(ogfx::gui::Widget::eCallback::ActionPerformed, callback);
			btn.enableIconOnly();
			btn.enableExpandIcon();
			btn.setMinSize({ m_height, m_height });
			m_buttonBox.addWidget(btn);
			return btn;
		}

		void ToolBar::onWindowResized(const Event& event)
		{
			updateBounds();
		}

		void ToolBar::updateBounds(void)
		{
			auto& win = getWindow();
			set_bar_size(cast<f32>(win.getWindowWidth()), m_height);
			f32 offset_y = 0;
			if (isStatusBar())
				offset_y = win.getWindowHeight() - m_height;
			else if (win.isMenuBarVisible())
				offset_y += win.getMenuBar().geth();
			setPosition(0, offset_y);
		}

		void ToolBar::setHeight(f32 height)
		{
			if (height == m_height && height == geth())
				return;
			m_height = height;
			refresh_size();
			updateBounds();
			getWindow().updateRootWidgetBounds();
		}

		void ToolBar::applyTheme(const ostd::Stylesheet& theme)
		{
			setHeight(getThemeValue<f32>(theme, "height", getHeight()));
			enableBottomBorder(isBorderEnabled());
			disableBorder();
		}

		void ToolBar::afterDraw(BasicRenderer2D& gfx)
		{
			// Bottom border line
			if (isBottomBorderEnabled())
			{
				f32 lineOffset = (isStatusBar() ? 0.0f : m_height);
				gfx.drawLine({ { getx(), gety() + lineOffset }, { getx() + getw(), gety() + lineOffset } }, getBorderColor());
			}
		}

		void ToolBar::onUpdate(void)
		{
			bool iconsOnly = !isButtonTextEnabled();
			for (auto& btn : m_buttons)
				btn.enableIconOnly(iconsOnly);
		}

		void ToolBar::refresh_size(void)
		{
			for (auto& btn : m_buttons)
				btn.setMinSize({ m_height, m_height });
		}

		void ToolBar::set_bar_size(f32 ww, f32 hh)
		{
			// Bypass the public setw()/seth() overrides (width is locked).
			Widget::setw(ww);
			Widget::seth(hh);
		}
	}
}
