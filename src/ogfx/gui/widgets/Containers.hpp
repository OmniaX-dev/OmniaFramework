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
#include <ogfx/gui/widgets/Scrollbar.hpp>
#include <ostd/utils/Time.hpp>

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			class Panel : public ScrollableWidget
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
					inline Panel(WindowCore& window) : ScrollableWidget(window) { create(); }
					Panel& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMousePressed(const Event& event) override;
					void onMouseReleased(const Event& event) override;
					void onMouseDragged(const Event& event) override;
					void onWindowResized(const Event& event) override;
					void setTitlebarType(const String& type);
					String getTitlebarType(void) const;
					inline String getTitle(void) const { return m_title; }
					inline void setTitle(const String& title) { m_title = title; }
					inline f32 getTitlebarHeight(void) const { return m_titlebarHeight; }
					OSTD_BOOL_PARAM_GETSET_I(Draggable, m_draggable);

				private:
					void draw_titlebar(BasicRenderer2D& gfx);

				private:
					String m_title { "Panel" };
					Vec2 m_mousePos { 0, 0 };
					bool m_mousePressed { false };
					bool m_draggable { false };
					Rectangle m_titleBarBounds;

					Color m_titleColor { Colors::Black  };
					i32 m_titlebarType = TitleBarTypes::NoneValue;
					f32 m_titlebarHeight { 30 };
					i32 m_titlebarBorderWidth { 1 };
					i32 m_titlebarFontSize { 26 };
					Color m_titlebarColor { Colors::Transparent };
					Color m_titlebarBorderColor { Colors::Black };
					i32 m_titleTextAlign { 0 };
			};
			class TabPanel : public Widget
			{
				public:
					inline TabPanel(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					TabPanel& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onMousePressed(const Event& event) override;
					void onMouseScrolled(const Event& event) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					Panel& addTab(const String& title);
					bool removeTab(Panel& tab);
					bool removeTab(i32 index);
					bool removeTab(const String& title);
					bool setCurrentTab(Panel& tab);
					bool setCurrentTab(i32 index, bool ignore_same_tab = false);
					void setTabBarHeight(f32 height);
					bool isMouseInsideTabBar(const Vec2& mousePos);
					inline void refreshCurrentTab(void) { setCurrentTab(m_currentTabIndex, true); }

					inline f32 getTabBarHeight(void) const { return m_tabBarHeight; }
					OSTD_PARAM_GETSET(Color, TabBarBackgroundColor, m_tabBarBackgroundColor);
					OSTD_PARAM_GETSET(Color, TabBarBorderColor, m_tabBarBorderColor);
					OSTD_PARAM_GETSET(f32, TabBarSidePadding, m_tabSidePadding);
					OSTD_PARAM_GETSET(i32, TabBarBorderWidth, m_tabBarBorderWidth);

				private:
					void prepare_for_current_tab_removal(void);
					void draw_tabs(ogfx::BasicRenderer2D& gfx);

				private:
					stdvec<std::unique_ptr<Panel>> m_tabs;
					stdvec<Rectangle> m_tabBoundsList;
					Panel* m_currentTab { nullptr };
					i32 m_currentTabIndex { -1 };
					f32 m_tabScrollOffset { 0.0f };
					f32 m_totalTabWidth { 0.0f };
					Rectangle m_tabBarBorderRadii { 0, 0, 0, 0 };

					f32 m_tabBarHeight { 35 };
					Color m_tabBarBackgroundColor { 120, 120, 120 };
					Color m_tabBarBorderColor { 170, 170, 170 };
					f32 m_tabSidePadding { 20 };
					i32 m_tabBarBorderWidth { 2 };
			};
		}
	}
}
