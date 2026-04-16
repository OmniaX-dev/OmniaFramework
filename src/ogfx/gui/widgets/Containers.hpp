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
					inline static const String Minimal { "minimal" };

					inline static constexpr i32 NoneValue = 0;
					inline static constexpr i32 FullValue = 1;
					inline static constexpr i32 MinimalValue = 2;
				};
				public:
					inline Panel(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					Panel& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseScrolled(const Event& event) override;
					void setTitlebarType(const String& type);
					String getTitlebarType(void) const;
					void setScrollOffset(const Vec2& offset) override;
					void addScrollOffset(const Vec2& offset) override;
					bool needsScroll(void) const override;
					inline void setBackGroundColor(const Color& color) { m_backgroundColor = color; }
					inline Color getBackgroundColor(void) { return m_backgroundColor; }
					inline String getTitle(void) const { return m_title; }
					inline void setTitle(const String& title) { m_title = title; }
					inline Vec2 getScrollOffset(void) const override { return m_scrollOffset; }
					inline f32 getTitlebarHeight(void) const { return m_titlebarHeight; }

				private:
					void draw_titlebar(BasicRenderer2D& gfx);

				private:
					String m_title { "Panel" };
					Vec2 m_scrollOffset { 0, 0 };

					Color m_titleColor { Colors::Black  };
					i32 m_titlebarType = TitleBarTypes::NoneValue;
					f32 m_titlebarHeight { 30 };
					i32 m_titlebarBorderWidth { 1 };
					i32 m_titlebarFontSize { 26 };
					Color m_titlebarColor { Colors::Transparent };
					Color m_titlebarBorderColor { Colors::Black };
					Rectangle m_basePadding { 0, 0, 0, 0 };
					i32 m_titleTextAlign { 0 };
					Vec2 m_scrollSpeed { 0.8f, 0.8f };
			};
		}
	}
}
