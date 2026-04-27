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
			setTypeName("ogfx::gui::widgets::ToolBar");
			auto& win = cast<Window&>(getWindow());
			w = win.getWindowWidth();
			h = m_height;
			disableBorder();
			setAsStatusBar(statusbar);
			hide();
			if (isStatusBar())
				setPosition(0, win.getWindowHeight() - m_height);
			else if (win.isMenuBarVisible())
				setPosition(0, win.getMenuBar().getHeight());
			else
				setPosition(0, 0);
			return *this;
		}

		widgets::Button& ToolBar::addButton(const String& iconPath, const String& text, EventCallback callback)
		{
			static f32 pad = 0;
			m_buttons.push_back({ getWindow() });
			auto& btn = m_buttons.back();
			btn.addThemeID("tool_button");
			btn.setIcon(iconPath);
			btn.setIconSize({ m_height, m_height });
			btn.setText(text);
			btn.setCallback(ogfx::gui::Widget::eCallback::ActionPerformed, callback);
			btn.enableIconOnly();
			addWidget(btn, { m_height * (m_buttons.size() - 1) + pad, 0 });
			pad += 15;
			return btn;
		}

		void ToolBar::onWindowResized(const Event& event)
		{
			auto& win = cast<Window&>(getWindow());
			w = win.getWindowWidth();
			h = m_height;
			f32 offset_y = 0;
			if (isStatusBar())
				offset_y = win.getWindowHeight() - m_height;
			else if (win.getMenuBar().isVisible())
				offset_y += win.getMenuBar().geth();
			sety(offset_y);
		}

		void ToolBar::applyTheme(const ostd::Stylesheet& theme)
		{
			auto& win = cast<Window&>(getWindow());
			setHeight(theme.get<f32>("toolbar.height", getHeight(), {}, {}));
			setFontSize(theme.get<i32>("toolbar.fontSize", getFontSize(), {}, {}));
			setBackgroundColor(theme.get<Color>("toolbar.backgroundColor", getBackgroundColor(), {}, {}));
			setTextColor(theme.get<Color>("toolbar.textColor", getTextColor(), {}, {}));
			setBorderColor(theme.get<Color>("toolbar.borderColor", getBorderColor(), {}, {}));
			enableBottomBorder(theme.get<bool>("toolbar.showBorder", isBottomBorderEnabled(), {}, {}));
			setSize(win.getWindowWidth(), m_height);
			disableBorder();
		}

		void ToolBar::onDraw(BasicRenderer2D& gfx)
		{
			gfx.fillRect(*this, m_backgroundColor);

			// Bottom border line
			if (isBottomBorderEnabled())
			{
				f32 lineOffset = (isStatusBar() ? 0.0f : m_height);
				gfx.drawLine({ { getx(), gety() + lineOffset }, { getx() + getw(), gety() + lineOffset } }, m_borderColor);
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
			{
				btn.setIconSize({ m_height, m_height });
			}
		}
	}
}
