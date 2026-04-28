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
		class RootWidget : public Widget
		{
			public:
				RootWidget(WindowCore& window);
				void onWindowResized(const Event& event) override;
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				OSTD_PARAM_GETSET(Color, TooltipBackgroundColor, m_tooltipBackgroundColor);
				OSTD_PARAM_GETSET(Color, TooltipBorderColor, m_tooltipBorderColor);
				OSTD_PARAM_GETSET(Color, TooltipTextColor, m_tooltipTextColor);
				OSTD_PARAM_GETSET(i32, TooltipBorderWidth, m_tooltipBorderWidth);
				OSTD_PARAM_GETSET(i32, TooltipFontSize, m_tooltipFontSize);

			private:
				Color m_tooltipBackgroundColor { "#FFF7D6FF" };
				Color m_tooltipBorderColor { 50, 50, 50 };
				Color m_tooltipTextColor { 50, 50, 50 };
				i32 m_tooltipBorderWidth { 1 };
				i32 m_tooltipFontSize { 20 };
		};
	}
}
