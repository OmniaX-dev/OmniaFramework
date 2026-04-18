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
				allowScroll(true);
				m_scrollbar.setMargin({ 0, getTitlebarHeight(), 0, 0 });
				m_scrollbar.enableManualDraw(true);
				addWidget(m_scrollbar);
				m_smoothScrollTimer.create(60.0f, [&](f64 dt) -> void {
					f32 stepX = m_scrollVelocity.x * (1.0f - m_scrollSmoothFactor);
					f32 stepY = m_scrollVelocity.y * (1.0f - m_scrollSmoothFactor);

					addScrollOffset({ stepX, stepY });
					m_scrollVelocity -= { stepX, stepY };

					if (std::abs(m_scrollVelocity.x) < 0.5f) m_scrollVelocity.x = 0;
					if (std::abs(m_scrollVelocity.y) < 0.5f) m_scrollVelocity.y = 0;
				}, true);

				m_smoothScrollTimer.setStopCondition([&](void) -> bool {
					return m_scrollVelocity.x == 0 && m_scrollVelocity.y == 0;
				});
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
				m_scrollSpeed = getThemeValue<f32>(theme, "panel.scrollSpeed", 0.8f);
				m_scrollSmoothFactor = std::clamp(getThemeValue<f32>(theme, "panel.scrollSmoothFactor", 0.7f), 0.0f, 1.0f);
				m_basePadding = getPadding();
				m_titleColor = getThemeValue<Color>(theme, "panel.titleColor", Colors::Black);
				m_titlebarColor = getThemeValue<Color>(theme, "panel.titlebarColor", Colors::Transparent);
				m_titlebarBorderColor = getThemeValue<Color>(theme, "panel.titlebarBorderColor", Colors::Black);
				m_titlebarHeight = getThemeValue<f32>(theme, "panel.titlebarHeight", 30);
				m_titlebarBorderWidth = getThemeValue<i32>(theme, "panel.titlebarBorderWidth", 1);
				m_titlebarFontSize = getThemeValue<i32>(theme, "panel.titlebarFontSize", 26);
				m_titleTextAlign = getThemeValue<i32>(theme, "panel.titlebarTextAlign", cast<i32>(WindowCore::eTextAlign::Left));
				setTitlebarType(getThemeValue<String>(theme, "panel.titlebarType", TitleBarTypes::None));
			}

			void Panel::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				// if (isScrollAllowed())
				//     gfx.fillRect(getContentExtents() + Rectangle { getGlobalPosition(), 0, 0 }, { 80, 0, 0, 30 });
			}

			void Panel::onUpdate(void)
			{
				m_smoothScrollTimer.update();
			}

			void Panel::afterDraw(ogfx::BasicRenderer2D& gfx)
			{
				draw_titlebar(gfx);
				m_scrollbar.__draw(gfx);
			}

			void Panel::onMouseScrolled(const Event& event)
			{
				if (!isScrollAllowed())
					return;

				if (std::abs(event.mouse->scrollAmount.y) > 0)
					m_scrollVelocity.y += m_scrollSpeed * event.mouse->scrollAmount.y * 15.0f;
				if (std::abs(event.mouse->scrollAmount.x) > 0)
					m_scrollVelocity.x += m_scrollSpeed * event.mouse->scrollAmount.x * 15.0f;

				if (m_smoothScrollTimer.isStopped())
					m_smoothScrollTimer.restart();

				event.handle();
			}

			void Panel::setTitlebarType(const String& type)
			{
				String t = type.new_toLower().trim();
				if (t == TitleBarTypes::None)
				{
					setPadding(m_basePadding);
					m_titlebarType = TitleBarTypes::NoneValue;
				}
				else if (t == TitleBarTypes::Minimal)
				{
					setPadding(m_basePadding + Rectangle { 0, m_titlebarHeight, 0, 0 });
					m_titlebarType = TitleBarTypes::MinimalValue;
				}
				else if (t == TitleBarTypes::Full)
				{
					setPadding(m_basePadding + Rectangle { 0, m_titlebarHeight, 0, 0 });
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

			void Panel::setScrollOffset(const Vec2& offset)
			{
				auto ext = getContentExtents();
				auto cont = getContentBounds();
				f32 maxScrollY = -(ext.h - cont.h);
				f32 maxScrollX = -(ext.w - cont.w);

				m_scrollOffset = offset;

				if (m_scrollOffset.y < maxScrollY) m_scrollOffset.y = maxScrollY;
				if (m_scrollOffset.y > 0) m_scrollOffset.y = 0;
				if (m_scrollOffset.x < maxScrollX) m_scrollOffset.x = maxScrollX;
				if (m_scrollOffset.x > 0) m_scrollOffset.x = 0;
			}

			void Panel::addScrollOffset(const Vec2& offset)
			{
				auto ext = getContentExtents();
				auto cont = getContentBounds();
				f32 maxScrollY = -(ext.h - cont.h);
				f32 maxScrollX = -(ext.w - cont.w);

				m_scrollOffset += offset;

				if (m_scrollOffset.y < maxScrollY) m_scrollOffset.y = maxScrollY;
				if (m_scrollOffset.y > 0) m_scrollOffset.y = 0;
				if (m_scrollOffset.x < maxScrollX) m_scrollOffset.x = maxScrollX;
				if (m_scrollOffset.x > 0) m_scrollOffset.x = 0;
			}

			bool Panel::needsScroll(void) const
			{
				return isScrollAllowed() && getContentExtents().h > getContentBounds().h;
			}

			void Panel::onWidgetAdded(Widget& child)
			{
				removeWidget(m_scrollbar);
				addWidget(m_scrollbar, { 0, 0 }, true);
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
		}
	}
}
