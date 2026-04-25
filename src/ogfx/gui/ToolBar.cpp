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
		ToolBar::ToolBar(Window& window) : Widget({ 0, 0, 0, 0 }, window)
		{
			create();
		}

		ToolBar& ToolBar::create(void)
		{
			setRootChild();
			setStylesheetCategoryName("toolbar");
			setTypeName("ogfx::gui::widgets::ToolBar");
			auto& win = cast<Window&>(getWindow());
			setSize(win.getWindowWidth(), m_height);
			if (win.isMenuBarVisible())
				setPosition(0, win.getMenuBar().getHeight());
			else
				setPosition(0, 0);
			return *this;
		}

		void ToolBar::onWindowResized(const Event& event)
		{
			auto& win = cast<Window&>(getWindow());
			setSize(win.getWindowWidth(), m_height);
			f32 offset_y = 0;
			if (win.getMenuBar().isVisible())
				offset_y += win.getMenuBar().geth();
			sety(offset_y);
		}

		void ToolBar::applyTheme(const ostd::Stylesheet& theme)
		{
			auto& win = cast<Window&>(getWindow());
			setHeight(theme.get<f32>("toolbar.height", getHeight(), {}, {}));
			setItemPadding(theme.get<Rectangle>("toolbar.itemPadding", getItemPadding(), {}, {}));
			setFontSize(theme.get<i32>("toolbar.fontSize", getFontSize(), {}, {}));
			setBackgroundColor(theme.get<Color>("toolbar.backgroundColor", getBackgroundColor(), {}, {}));
			setTextColor(theme.get<Color>("toolbar.textColor", getTextColor(), {}, {}));
			setBorderColor(theme.get<Color>("toolbar.borderColor", getBorderColor(), {}, {}));
			setSize(win.getWindowWidth(), m_height);
		}

		void ToolBar::onDraw(BasicRenderer2D& gfx)
		{
			if (!m_visible) return;

			gfx.fillRect(*this, m_backgroundColor);

			// Bottom border line
			gfx.drawLine({ { getx(), gety() + m_height }, { getx() + getw(), gety() + m_height } }, m_borderColor);
		}

		void ToolBar::onUpdate(void)
		{
			if (!m_visible) return;
		}

		void ToolBar::onMousePressed(const Event& event)
		{
			if (!m_visible) return;

			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };

			// event.handle();
		}

		void ToolBar::onMouseMoved(const Event& event)
		{
			if (!m_visible) return;

			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };

		}
	}
}
