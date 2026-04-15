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
		namespace widgets
		{
			class Panel : public Widget
			{
				public: struct TitleBarTypes
				{
					inline static const String None { "none" };
					inline static const String Full { "full" };
					inline static const String Minimal { "monimal" };
				};
				public:
					inline Panel(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					Panel& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void setTitlebarType(const String& type);
					inline void setBackGroundColor(const Color& color) { m_backgroundColor = color; }
					inline Color getBackgroundColor(void) { return m_backgroundColor; }
					inline String getTitlebarType(void) const { return m_titleType; }

				private:
					Color m_titleColor { 0, 0, 0 };
					bool m_showTitle { false };
					String m_titleType = TitleBarTypes::None;
					f32 m_titleHeight { 30 };
			};
		}
	}
}
