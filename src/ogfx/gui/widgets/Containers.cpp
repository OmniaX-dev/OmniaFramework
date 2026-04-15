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

#include "Containers.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			Panel& Panel::create(void)
			{
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Panel");
				disableDrawBox();
				disableFocus();
				enableStopEvents();
				validate();
				return *this;
			}

			void Panel::applyTheme(const ostd::Stylesheet& theme)
			{
				setBackGroundColor(getThemeValue<Color>(theme, "panel.backgroundColor", Colors::Gray));
				setBorderRadius(getThemeValue<i32>(theme, "panel.borderRadius", 8));
				setBorderWidth(getThemeValue<i32>(theme, "panel.borderWidth", 2));
				enableBorder(getThemeValue<bool>(theme, "panel.showBorder", true));
				enableBackground(getThemeValue<bool>(theme, "panel.showBackground", true));
				setBorderColor(getThemeValue<Color>(theme, "panel.borderColor", Colors::Black));
				m_titleColor = getThemeValue<Color>(theme, "panel.titleColor", Colors::Black);
				m_showTitle = getThemeValue<bool>(theme, "panel.showTitle", false);
				m_titleHeight = getThemeValue<f32>(theme, "panel.titleHeight", 30);
				m_titleType = getThemeValue<String>(theme, "panel.titlebarType", TitleBarTypes::None);
				setPadding(getThemeValue<Rectangle>(theme, "panel.padding", { 15, 15, 15, 15 }));
			}

			void Panel::onDraw(ogfx::BasicRenderer2D& gfx)
			{
			}
		}
	}
}
