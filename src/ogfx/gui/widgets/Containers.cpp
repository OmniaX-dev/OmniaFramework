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
				setBackGroundColor(getThemeValue<ostd::Color>(theme, "panel.backgroundColor", ostd::Colors::Gray));
				m_borderRadius = getThemeValue<int32_t>(theme, "panel.borderRadius", 8);
				m_borderWidth = getThemeValue<int32_t>(theme, "panel.borderWidth", 2);
				m_showBorder = getThemeValue<bool>(theme, "panel.showBorder", true);
				m_borderColor = getThemeValue<ostd::Color>(theme, "panel.borderColor", ostd::Colors::Black);
				m_titleColor = getThemeValue<ostd::Color>(theme, "panel.titleColor", ostd::Colors::Black);
				m_showTitle = getThemeValue<bool>(theme, "panel.showTitle", false);
				m_titleHeight = getThemeValue<float>(theme, "panel.titleHeight", 30);
				m_titleType = getThemeValue<ostd::String>(theme, "panel.titleHeight", "text");
				setPadding(getThemeValue<ostd::Rectangle>(theme, "panel.padding", { 15, 15, 15, 15 }));
			}

			void Panel::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_showBorder)
					gfx.outlinedRoundRect({ getGlobalPosition(), getSize() }, getBackgroundColor(), m_borderColor, m_borderRadius, m_borderWidth);
				else
					gfx.fillRoundRect({ getGlobalPosition(), getSize() }, getBackgroundColor(), m_borderRadius);
			}
		}
	}
}
