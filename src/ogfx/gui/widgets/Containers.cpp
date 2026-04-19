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
#include "../Window.hpp"

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
				setPadding(getThemeValue<Rectangle>(theme, "panel.padding", { 15, 15, 15, 15 }));
				setMargin(getThemeValue<Rectangle>(theme, "panel.margin", { 0, 0, 0, 0 }));
				setScrollSpeed(getThemeValue<f32>(theme, "panel.scrollSpeed", 0.8f));
				setScrollSmoothFactor(getThemeValue<f32>(theme, "panel.scrollSmoothFactor", 0.7f));
				m_titleColor = getThemeValue<Color>(theme, "panel.titleColor", Colors::Black);
				m_titlebarColor = getThemeValue<Color>(theme, "panel.titlebarColor", Colors::Transparent);
				m_titlebarBorderColor = getThemeValue<Color>(theme, "panel.titlebarBorderColor", Colors::Black);
				m_titlebarHeight = getThemeValue<f32>(theme, "panel.titlebarHeight", 30);
				m_titlebarBorderWidth = getThemeValue<i32>(theme, "panel.titlebarBorderWidth", 1);
				m_titlebarFontSize = getThemeValue<i32>(theme, "panel.titlebarFontSize", 26);
				m_titleTextAlign = getThemeValue<i32>(theme, "panel.titlebarTextAlign", cast<i32>(WindowCore::eTextAlign::Left));
				setTitlebarType(getThemeValue<String>(theme, "panel.titlebarType", TitleBarTypes::None));
			}

			void Panel::afterDraw(ogfx::BasicRenderer2D& gfx)
			{
				draw_titlebar(gfx);
				drawScrollbars(gfx);
			}

			void Panel::setTitlebarType(const String& type)
			{
				String t = type.new_toLower().trim();
				if (t == TitleBarTypes::None)
				{
					setContentOffset({ 0, 0 });
					updateScrollbarsSize();
					m_titlebarType = TitleBarTypes::NoneValue;
				}
				else if (t == TitleBarTypes::Minimal)
				{
					setContentOffset({ 0, m_titlebarHeight });
					updateScrollbarsSize();
					m_titlebarType = TitleBarTypes::MinimalValue;
				}
				else if (t == TitleBarTypes::Full)
				{
					setContentOffset({ 0, m_titlebarHeight });
					updateScrollbarsSize();
					m_titlebarType = TitleBarTypes::FullValue;
				}
			}

			String Panel::getTitlebarType(void) const
			{
				switch (m_titlebarType)
				{
					case TitleBarTypes::FullValue: return TitleBarTypes::Full;
					case TitleBarTypes::MinimalValue: return TitleBarTypes::Minimal;
					case TitleBarTypes::NoneValue: return TitleBarTypes::None;
					default: return TitleBarTypes::None;
				}
			}

			void Panel::draw_titlebar(BasicRenderer2D& gfx)
			{
				f32 br = cast<f32>(getBorderWidth());
				br /= 2;
				Rectangle titleBarBounds {
					getGlobalPosition() + Vec2 { br, br },
					Vec2 { getw(), m_titlebarHeight } - Vec2 { br * 2, br * 2 }
				};
				switch (m_titlebarType)
				{
					case TitleBarTypes::FullValue:
					{
						gfx.outlinedRoundRect(titleBarBounds, m_titlebarColor, m_titlebarColor, { cast<f32>(getBorderRadius()), cast<f32>(getBorderRadius()), 0, 0 }, getBorderWidth());
						gfx.drawLine({ getGlobalPosition() + Vec2 { 0, m_titlebarHeight - 2 }, getGlobalPosition() + Vec2 { getGlobalBounds().w, m_titlebarHeight - 2 } }, m_titlebarBorderColor, m_titlebarBorderWidth);
						if (m_titleTextAlign == cast<i32>(WindowCore::eTextAlign::Center))
							gfx.drawCenteredString(m_title, titleBarBounds, m_titleColor, m_titlebarFontSize);
						else
							gfx.drawString(m_title, titleBarBounds.getPosition() + Vec2 { 10, 5 }, m_titleColor, m_titlebarFontSize);
						break;
					}
					case TitleBarTypes::MinimalValue:
					{
						if (m_titleTextAlign == cast<i32>(WindowCore::eTextAlign::Center))
							gfx.drawCenteredString(m_title, titleBarBounds, m_titleColor, m_titlebarFontSize);
						else
							gfx.drawString(m_title, titleBarBounds.getPosition() + Vec2 { 10, 5 }, m_titleColor, m_titlebarFontSize);
						break;
					}
					case TitleBarTypes::NoneValue:
					default: break;
				}
			}




			TabPanel& TabPanel::create(void)
			{
				setPadding({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::TabPanel");
				disableDrawBox();
				disableFocus();
				enableStopEvents();
				validate();
				return *this;
			}

			void TabPanel::applyTheme(const ostd::Stylesheet& theme)
			{

			}

			void TabPanel::onMouseReleased(const Event& event)
			{

			}

			void TabPanel::onDraw(ogfx::BasicRenderer2D& gfx)
			{

			}

			Panel& TabPanel::addTab(const String& title)
			{
				m_tabs.push_back(std::make_unique<Panel>(getWindow()));
				auto& tab = *m_tabs.back();
				// Initialization code here
				return tab;
			}

			bool TabPanel::removeTab(Panel& tab)
			{
				auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const std::unique_ptr<Panel>& p) { return p.get() == &tab; });
				if (it == m_tabs.end())
					return false;
				if (m_currentTab == it->get())
					prepare_for_current_tab_removal();
				m_tabs.erase(it);
				return true;
			}

			bool TabPanel::removeTab(i32 index)
			{
				if (index < 0 || index >= (i32)m_tabs.size())
					return false;
				auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const std::unique_ptr<Panel>& p) { return p.get() == m_currentTab; });
				if (std::distance(m_tabs.begin(), it) == index)
					prepare_for_current_tab_removal();
				m_tabs.erase(m_tabs.begin() + index);
				return true;
			}

			bool TabPanel::removeTab(const String& title)
			{
				auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const std::unique_ptr<Panel>& p) { return p->getTitle() == title; });
				if (it == m_tabs.end())
					return false;
				m_tabs.erase(it);
				return true;
			}

			bool TabPanel::setCurrentTab(Panel& tab)
			{
				auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const std::unique_ptr<Panel>& p) { return p.get() == &tab; });
				if (it == m_tabs.end())
					return false;
				m_currentTab = it->get();
				return true;
			}

			bool TabPanel::setCurrentTab(i32 index)
			{
				if (index < 0 || index >= (i32)m_tabs.size())
					return false;
				m_currentTab = m_tabs[index].get();
				return true;
			}

			void TabPanel::prepare_for_current_tab_removal(void)
			{
				if (m_currentTab == nullptr)
					return;
				if (m_tabs.size() < 2)
				{
					m_currentTab = nullptr;
					return;
				}
				auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const std::unique_ptr<Panel>& p) { return p.get() == m_currentTab; });

				if (it == m_tabs.end())
					return;  // shouldn't happen but defensive

				if (it != m_tabs.begin())
					m_currentTab = (it - 1)->get();  // tab to the left
				else
					m_currentTab = (it + 1)->get();  // first tab being removed, go right
			}
		}
	}
}
