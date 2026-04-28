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

#include "RootWidget.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../Window.hpp"

namespace ogfx
{
	namespace gui
	{
		RootWidget::RootWidget(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window)
		{
			setRootChild();
			setSize(cast<f32>(window.getWindowWidth()), cast<f32>(window.getWindowHeight()));
			setStylesheetCategoryName("window");
			setTypeName("ogfx::gui::RootWidget");
		}

		void RootWidget::onWindowResized(const Event& event)
		{
			Window& win = cast<Window&>(getWindow()); //TODO: Potentially unsage?
			f32 offset_y = 0;
			if (win.m_menubar.isVisible())
				offset_y += win.m_menubar.geth();
			if (win.m_toolbar.isVisible())
				offset_y += win.m_toolbar.geth();
			sety(offset_y);
			if (win.m_statusbar.isVisible())
				offset_y += win.m_statusbar.geth();
			setSize(cast<f32>(event.windowResized->new_width), cast<f32>(event.windowResized->new_height) - offset_y);
		}

		void RootWidget::applyTheme(const ostd::Stylesheet& theme)
		{
			setTooltipBackgroundColor(getThemeValue<Color>(theme, "tooltip.backgroundColor", getTooltipBackgroundColor()));
			setTooltipBorderColor(getThemeValue<Color>(theme, "tooltip.borderColor", getTooltipBorderColor()));
			setTooltipTextColor(getThemeValue<Color>(theme, "tooltip.textColor", getTooltipTextColor()));
			setTooltipBorderWidth(getThemeValue<i32>(theme, "tooltip.borderWidth", getTooltipBorderWidth()));
			setTooltipFontSize(getThemeValue<i32>(theme, "tooltip.fontSize", getTooltipFontSize()));
		}

		void RootWidget::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			gfx.fillRect({ 0, 0, cast<f32>(getWindow().getWindowWidth()), cast<f32>(getWindow().getWindowHeight()) }, getBackgroundColor());
		}
	}
}
