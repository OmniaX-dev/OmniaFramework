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

#include "Scrollbar.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			VerticalScrollBar& VerticalScrollBar::create(void)
			{
				setPadding({ 0, 0, 0, 0 });
				setMargin({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::Label");
				disableChildren();
				enableBackground(true);
				enableBorder(false);
				enableTopMost(true);
				enableIgnoreScroll(true);
				setStylesheetCategoryName("scrollbar");
				validate();
				return *this;
			}

			void VerticalScrollBar::applyTheme(const ostd::Stylesheet& theme)
			{
				w = getThemeValue<f32>(theme, "width", 15);
				m_thumbColor = getThemeValue<Color>(theme, "thumb.color", { 120, 120, 120 });
				m_thumbBorderRadius = getThemeValue<f32>(theme, "thumb.borderRadius", 16);
				m_thumbBorderColor = getThemeValue<Color>(theme, "thumb.borderColor", { 150, 150, 150 });
				m_thumbShowBorder = getThemeValue<bool>(theme, "thumb.showBorder", true);
				m_trackColor = getThemeValue<Color>(theme, "track.color", { 70, 70, 70 });
			}

			void VerticalScrollBar::afterDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (!getParent()->needsVScroll())
					return;
				gfx.fillRoundRect(getGlobalBounds() - m_correctionOffset, m_trackColor, m_trackBorderRadii);
				gfx.outlinedRoundRect(m_thumbGlobalBounds, m_thumbColor, m_thumbBorderColor, m_thumbBorderRadius, 1);
			}

			void VerticalScrollBar::onMouseDragged(const Event& event)
			{
				if (!m_mousePressed)
					return;

				if (isMouseInsideThumb({ event.mouse->position_x, event.mouse->position_y }))
					m_mouseDragged = true;

				if (m_mouseDragged)
				{
					auto bounds = getGlobalBounds() - m_correctionOffset;
					f32 newThumbY = (event.mouse->position_y - bounds.y) - m_dragGrabOffset;
					set_thumb_y(newThumbY);
				}
			}

			void VerticalScrollBar::onMousePressed(const Event& event)
			{
				m_mousePressed = true;
				m_dragGrabOffset = event.mouse->position_y - m_thumbGlobalBounds.y;
			}

			void VerticalScrollBar::onMouseReleased(const Event& event)
			{
				m_mousePressed = false;
				if (event.mouse->mousePressedOnWidget != this)
					return;
				if (!m_mouseDragged)
				{
					auto bounds = getGlobalBounds() - m_correctionOffset;
					f32 newThumbY = (event.mouse->position_y - bounds.y);
					newThumbY -= m_thumbHeight / 2.0f;
					set_thumb_y(newThumbY);
				}
				m_mouseDragged = false;
			}

			void VerticalScrollBar::onUpdate(void)
			{
				update_thumb();
			}

			bool VerticalScrollBar::isMouseInsideThumb(const Vec2& mouse_pos)
			{
				return m_thumbGlobalBounds.contains(mouse_pos, true);
			}

			void VerticalScrollBar::update_thumb(void)
			{
				x = getParent()->getSize().x - w;
				y = 0;
				h = getParent()->getGlobalPureContentBounds().h;

				auto ext = getParent()->getContentExtents();
				auto cont = getParent()->getContentBounds();
				auto scrollOfset = getParent()->getScrollOffset();

				// Thumb size is proportional to visible/total ratio
				f32 visibleRatio = cont.h / ext.h;  // e.g. 0.4 means 40% visible
				m_thumbHeight = std::max(20.0f, geth() * visibleRatio);  // min 20px for usability

				// Thumb position is proportional to scroll progress
				f32 scrollProgress = -scrollOfset.y / (ext.h - cont.h);  // 0 to 1
				m_thumbY = gety() + ((geth() - m_thumbHeight) * scrollProgress);

				auto bounds = getGlobalBounds() - m_correctionOffset;
				m_thumbGlobalBounds = { bounds.x + 2, bounds.y + gety() + m_thumbY, getw() - 4, m_thumbHeight };
			}

			void VerticalScrollBar::set_thumb_y(f32 thumby)
			{
				if (!getParent()->needsVScroll())
					return;
				auto bounds = getGlobalBounds() - m_correctionOffset;
				auto ext = getParent()->getContentExtents();
				auto cont = getParent()->getContentBounds();
				thumby = std::clamp(thumby, 0.0f, h - m_thumbHeight);
				m_thumbY = thumby;
				f32 scrollProgress = (bounds.y + gety() + m_thumbY - bounds.y) / (bounds.h - m_thumbHeight);
				f32 maxScroll = -(ext.h - cont.h);
				auto parentScrollOffset = getParent()->getScrollOffset();
				parentScrollOffset.y = maxScroll * scrollProgress;
				getParent()->setScrollOffset(parentScrollOffset);
			}






			HorizontalScrollbar& HorizontalScrollbar::create(void)
			{
				setPadding({ 0, 0, 0, 0 });
				setMargin({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::Label");
				disableChildren();
				enableBackground(true);
				enableBorder(false);
				enableTopMost(true);
				enableIgnoreScroll(true);
				setStylesheetCategoryName("scrollbar");
				validate();
				return *this;
			}

			void HorizontalScrollbar::applyTheme(const ostd::Stylesheet& theme)
			{
				h = getThemeValue<f32>(theme, "width", 15);
				m_thumbColor = getThemeValue<Color>(theme, "thumb.color", { 120, 120, 120 });
				m_thumbBorderRadius = getThemeValue<f32>(theme, "thumb.borderRadius", 16);
				m_thumbBorderColor = getThemeValue<Color>(theme, "thumb.borderColor", { 150, 150, 150 });
				m_thumbShowBorder = getThemeValue<bool>(theme, "thumb.showBorder", true);
				m_trackColor = getThemeValue<Color>(theme, "track.color", { 70, 70, 70 });
			 }

			void HorizontalScrollbar::afterDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (!getParent()->needsHScroll())
					return;
				gfx.fillRoundRect(getGlobalBounds() - m_correctionOffset, m_trackColor, m_trackBorderRadii);
				gfx.outlinedRoundRect(m_thumbGlobalBounds, m_thumbColor, m_thumbBorderColor, m_thumbBorderRadius, 1);
			}

			void HorizontalScrollbar::onMouseDragged(const Event& event)
			{
				if (!m_mousePressed)
					return;

				if (isMouseInsideThumb({ event.mouse->position_x, event.mouse->position_y }))
					m_mouseDragged = true;

				if (m_mouseDragged)
				{
					auto bounds = getGlobalBounds() - m_correctionOffset;
					f32 newThumbX = (event.mouse->position_x - bounds.x) - m_dragGrabOffset;
					set_thumb_x(newThumbX);
				}
			}

			void HorizontalScrollbar::onMousePressed(const Event& event)
			{
				m_mousePressed = true;
				m_dragGrabOffset = event.mouse->position_x - m_thumbGlobalBounds.x;
			}

			void HorizontalScrollbar::onMouseReleased(const Event& event)
			{
				m_mousePressed = false;
				if (event.mouse->mousePressedOnWidget != this)
					return;
				if (!m_mouseDragged)
				{
					auto bounds = getGlobalBounds() - m_correctionOffset;
					f32 newThumbX = (event.mouse->position_x - bounds.x);
					newThumbX -= m_thumbWidth / 2.0f;
					set_thumb_x(newThumbX);
				}
				m_mouseDragged = false;
			}

			void HorizontalScrollbar::onUpdate(void)
			{
				update_thumb();
			}

			bool HorizontalScrollbar::isMouseInsideThumb(const Vec2& mouse_pos)
			{
				return m_thumbGlobalBounds.contains(mouse_pos, true);
			}

			void HorizontalScrollbar::update_thumb(void)
			{
				x = 0;
				y = getParent()->getSize().y - h;
				w = getParent()->getGlobalBounds().w - getParent()->getVScrollbarSize();

				auto ext = getParent()->getContentExtents();
				auto cont = getParent()->getContentBounds();
				auto scrollOfset = getParent()->getScrollOffset();

				// Thumb size is proportional to visible/total ratio
				f32 visibleRatio = cont.w / ext.w;  // e.g. 0.4 means 40% visible
				m_thumbWidth = std::max(20.0f, getw() * visibleRatio);  // min 20px for usability

				// Thumb position is proportional to scroll progress
				f32 scrollProgress = -scrollOfset.x / (ext.w - cont.w);  // 0 to 1
				m_thumbX = getx() + ((getw() - m_thumbWidth) * scrollProgress);

				auto bounds = getGlobalBounds() - m_correctionOffset;
				m_thumbGlobalBounds = { bounds.x + 2 + m_thumbX, bounds.y + gety(), m_thumbWidth, geth() - 4 };
				m_thumbGlobalBounds = { bounds.x + getx() + m_thumbX, bounds.y + 2, m_thumbWidth, geth() - 4 };
			}

			void HorizontalScrollbar::set_thumb_x(f32 thumbx)
			{
				if (!getParent()->needsHScroll())
					return;
				auto bounds = getGlobalBounds() - m_correctionOffset;
				auto ext = getParent()->getContentExtents();
				auto cont = getParent()->getContentBounds();
				thumbx = std::clamp(thumbx, 0.0f, w - m_thumbWidth);
				m_thumbX = thumbx;
				f32 scrollProgress = (bounds.x + getx() + m_thumbX - bounds.x) / (bounds.w - m_thumbWidth);
				f32 maxScroll = -(ext.w - cont.w);
				auto parentScrollOffset = getParent()->getScrollOffset();
				parentScrollOffset.x = maxScroll * scrollProgress;
				getParent()->setScrollOffset(parentScrollOffset);
			}






			ScrollableWidget& ScrollableWidget::create(void)
			{
				setTypeName("ogfx::gui::widgets::ScrollableWidget");
				enableVScroll(true);
				enableHScroll(true);
				m_vScrollbar.enableManualDraw(true);
				addWidget(m_vScrollbar);
				m_hScrollbar.setMargin({ 0, 0, 0, 0 });
				m_hScrollbar.enableManualDraw(true);
				addWidget(m_hScrollbar);
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
				updateScrollbarsSize();
				validate();
				return *this;
			}

			void ScrollableWidget::onUpdate(void)
			{
				m_smoothScrollTimer.update();
				if (!needsVScroll() && m_vScrollbarAdded)
				{
					removeWidget(m_vScrollbar);
					m_vScrollbarAdded = false;
				}
				else if (!m_vScrollbarAdded)
				{
					addWidget(m_vScrollbar);
					m_vScrollbarAdded = true;
				}
				if (!needsHScroll() && m_hScrollbarAdded)
				{
					removeWidget(m_hScrollbar);
					m_hScrollbarAdded = false;
				}
				else if (!m_hScrollbarAdded)
				{
					addWidget(m_hScrollbar);
					m_hScrollbarAdded = true;
				}
			}

			void ScrollableWidget::drawScrollbars(ogfx::BasicRenderer2D& gfx)
			{
				m_vScrollbar.__draw(gfx);
				m_hScrollbar.__draw(gfx);
			}

			void ScrollableWidget::updateScrollbarsSize(void)
			{
				m_vScrollbar.setMargin({ 0, getPureContentBounds().y, 0, 0 });
			}

			void ScrollableWidget::resetScroll(bool horizontal, bool vertical, bool propagate)
			{
				if (vertical)
					m_scrollOffset.y = 0;
				if (horizontal)
					m_scrollOffset.x = 0;
				if (propagate)
				{
					for (auto& w : getChildren().getWidgets())
					{
						if (w == nullptr) continue;
						w->resetScroll(horizontal, vertical, propagate);
					}
				}
			}

			void ScrollableWidget::onMouseScrolled(const Event& event)
			{
				if (isVScrollEnabled())
				{
					bool mouseInsideHScrollbar = m_hScrollbar.isMouseInside() && needsHScroll();
					if (std::abs(event.mouse->scrollAmount.y) > 0 && !mouseInsideHScrollbar)
					{
						m_scrollVelocity.y += m_scrollSpeed * event.mouse->scrollAmount.y * m_scrollSpeedMultiplier;
						if (m_smoothScrollTimer.isStopped())
							m_smoothScrollTimer.restart();
						event.handle();
					}
					else if (std::abs(event.mouse->scrollAmount.y) > 0)
					{
						m_scrollVelocity.x += m_scrollSpeed * event.mouse->scrollAmount.y * m_scrollSpeedMultiplier;
						if (m_smoothScrollTimer.isStopped())
							m_smoothScrollTimer.restart();
						event.handle();
					}
				}
				if (isHScrollEnabled())
				{
					if (std::abs(event.mouse->scrollAmount.x) > 0)
					{
						m_scrollVelocity.x += m_scrollSpeed * event.mouse->scrollAmount.x * m_scrollSpeedMultiplier;
						if (m_smoothScrollTimer.isStopped())
							m_smoothScrollTimer.restart();
						event.handle();
					}
				}
			}

			void ScrollableWidget::setScrollOffset(const Vec2& offset)
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

			void ScrollableWidget::addScrollOffset(const Vec2& offset)
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

			bool ScrollableWidget::needsVScroll(void) const
			{
				return isVScrollEnabled() && getContentExtents().h > getContentBounds().h;
			}

			bool ScrollableWidget::needsHScroll(void) const
			{
				return isHScrollEnabled() && getContentExtents().w > getContentBounds().w;
			}

			void ScrollableWidget::onWidgetAdded(Widget& child)
			{
				removeWidget(m_vScrollbar);
				removeWidget(m_hScrollbar);
				addWidget(m_vScrollbar, { 0, 0 }, true);
				addWidget(m_hScrollbar, { 0, 0 }, true);
			}

			f32 ScrollableWidget::getVScrollbarSize(void) const
			{
				if (!isVScrollEnabled())
					return 0;
				if (!needsVScroll())
					return 0;
				return m_vScrollbar.getw();
			}

			f32 ScrollableWidget::getHScrollbarSize(void) const
			{
				if (!isHScrollEnabled())
					return 0;
				if (!needsHScroll())
					return 0;
				return m_hScrollbar.geth();
			}
		}
	}
}
