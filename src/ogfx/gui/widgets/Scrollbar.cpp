/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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

#include <algorithm>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// ScrollBar
		// ============================================================================

		ScrollBar::ScrollBar(Window& window, eOrientation orientation) : Widget({ 0, 0, 0, 0 }, window), m_orientation(orientation)
		{
			create();
		}

		ScrollBar& ScrollBar::create(void)
		{
			setPadding({ 0, 0, 0, 0 });
			setMargin({ 0, 0, 0, 0 });
			setTypeName("ogfx::gui::ScrollBar");
			disableChildren();
			disableFocus();
			enableBackground(false);
			enableBorder(false);
			enableTopMost(true);
			// Positioned relative to the owner's frame, not its scrolled content.
			enableIgnoreScroll(true);
			// The owner draws it on top of its content (drawScrollbars()).
			enableManualDraw(true);
			setVisible(false);
			setStylesheetCategoryName("scrollbar");
			// Global mouse signals keep a drag going wherever the cursor is.
			connectSignal(ostd::BuiltinSignals::MouseMoved);
			connectSignal(ostd::BuiltinSignals::MouseReleased);
			validate();
			return *this;
		}

		void ScrollBar::applyTheme(const ostd::Stylesheet& theme)
		{
			m_thickness = std::max(1.0f, getThemeValue<f32>(theme, "width", m_thickness));
			m_thumbColor = getThemeValue<Color>(theme, "thumb.color", m_thumbColor);
			m_thumbBorderRadius = getThemeValue<f32>(theme, "thumb.borderRadius", m_thumbBorderRadius);
			m_thumbBorderColor = getThemeValue<Color>(theme, "thumb.borderColor", m_thumbBorderColor);
			m_thumbShowBorder = getThemeValue<bool>(theme, "thumb.showBorder", m_thumbShowBorder);
			m_thumbInset = getThemeValue<f32>(theme, "thumb.inset", m_thumbInset);
			m_minThumbLength = getThemeValue<f32>(theme, "thumb.minLength", m_minThumbLength);
			m_trackColor = getThemeValue<Color>(theme, "track.color", m_trackColor);
		}

		void ScrollBar::setRange(f32 contentLength, f32 viewportLength, f32 position)
		{
			m_contentLength = std::max(0.0f, contentLength);
			m_viewportLength = std::max(0.0f, viewportLength);
			m_position = std::clamp(position, 0.0f, getMaxScrollPosition());
		}

		f32 ScrollBar::track_length(void) const
		{
			return isVertical() ? geth() : getw();
		}

		f32 ScrollBar::thumb_length(void) const
		{
			const f32 track = track_length();
			if (m_contentLength <= 0.0f || m_viewportLength >= m_contentLength)
				return track;
			const f32 proportional = track * (m_viewportLength / m_contentLength);
			return std::clamp(proportional, std::min(m_minThumbLength, track), track);
		}

		f32 ScrollBar::thumb_offset(void) const
		{
			const f32 maxPos = getMaxScrollPosition();
			if (maxPos <= 0.0f)
				return 0.0f;
			return (track_length() - thumb_length()) * std::clamp(m_position / maxPos, 0.0f, 1.0f);
		}

		Rectangle ScrollBar::getGlobalThumbBounds(void) const
		{
			const Vec2 g = getGlobalPosition();
			const f32 offset = thumb_offset();
			const f32 length = thumb_length();
			if (isVertical())
				return { g.x + m_thumbInset, g.y + offset, std::max(0.0f, getw() - (m_thumbInset * 2)), length };
			return { g.x + offset, g.y + m_thumbInset, length, std::max(0.0f, geth() - (m_thumbInset * 2)) };
		}

		f32 ScrollBar::local_axis_position(f32 globalX, f32 globalY) const
		{
			const Vec2 g = getGlobalPosition();
			return isVertical() ? (globalY - g.y) : (globalX - g.x);
		}

		void ScrollBar::drag_to(f32 localAxisPos)
		{
			const f32 freeSpace = track_length() - thumb_length();
			if (freeSpace <= 0.0f)
				return;
			const f32 t = std::clamp((localAxisPos - m_grabOffset) / freeSpace, 0.0f, 1.0f);
			const f32 newPosition = t * getMaxScrollPosition();
			if (newPosition == m_position)
				return;
			m_position = newPosition;
			if (m_scrollCallback)
				m_scrollCallback(m_position);
		}

		void ScrollBar::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			gfx.fillRect({ getGlobalPosition(), getSize() }, m_trackColor);
			const Rectangle thumb = getGlobalThumbBounds();
			if (m_thumbShowBorder)
				gfx.outlinedRoundRect(thumb, m_thumbColor, m_thumbBorderColor, m_thumbBorderRadius, 1);
			else
				gfx.fillRoundRect(thumb, m_thumbColor, m_thumbBorderRadius);
		}

		void ScrollBar::onMousePressed(const Event& event)
		{
			if (event.mouse->button != MouseEventData::eButton::Left)
				return;
			const f32 local = local_axis_position(event.mouse->position_x, event.mouse->position_y);
			const f32 offset = thumb_offset();
			const f32 length = thumb_length();
			if (local >= offset && local <= offset + length)
				m_grabOffset = local - offset;  // grab the thumb where it was clicked
			else
			{
				// Track click: jump so the thumb is centered on the cursor, then drag.
				m_grabOffset = length * 0.5f;
				drag_to(local);
			}
			m_dragging = true;
			event.handle();  // the owner must not treat this as a click on its content
		}

		void ScrollBar::handleSignal(ostd::Signal& signal)
		{
			if (!m_dragging || !signal.userData)
				return;
			auto& med = cast<MouseEventData&>(*signal.userData);
			if (signal.ID == ostd::BuiltinSignals::MouseMoved)
				drag_to(local_axis_position(med.position_x, med.position_y));
			else if (signal.ID == ostd::BuiltinSignals::MouseReleased)
				m_dragging = false;
		}


		// ============================================================================
		// ScrollableWidget
		// ============================================================================

		ScrollableWidget& ScrollableWidget::create(void)
		{
			setTypeName("ogfx::gui::ScrollableWidget");
			enableVScroll(true);
			enableHScroll(true);
			addWidget(m_vScrollbar);
			addWidget(m_hScrollbar);
			m_vScrollbar.setScrollCallback([this](f32 position) {
				m_scrollVelocity.y = 0;
				setScrollOffset({ m_scrollOffset.x, -(position + getScrollArea().y) });
			});
			m_hScrollbar.setScrollCallback([this](f32 position) {
				m_scrollVelocity.x = 0;
				setScrollOffset({ -(position + getScrollArea().x), m_scrollOffset.y });
			});
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

		void ScrollableWidget::onUpdate(void)
		{
			m_smoothScrollTimer.update();
			// Content can change at any time (children moved, rows added, ...).
			refresh_scrollbars();
		}

		void ScrollableWidget::layoutChildren(void)
		{
			Widget::layoutChildren();
			refresh_scrollbars();
		}

		f32 ScrollableWidget::bar_inset(void) const
		{
			// Keep the bars inside the widget's border.
			return isBorderEnabled() ? cast<f32>(std::max(0, getBorderWidth())) : 0.0f;
		}

		f32 ScrollableWidget::v_bar_extent(void) const
		{
			return m_vVisible ? (m_vScrollbar.getThickness() + bar_inset()) : 0.0f;
		}

		f32 ScrollableWidget::h_bar_extent(void) const
		{
			return m_hVisible ? (m_hScrollbar.getThickness() + bar_inset()) : 0.0f;
		}

		Rectangle ScrollableWidget::getContentBounds(void) const
		{
			Rectangle cb = Widget::getContentBounds();
			const Vec2 size = getSize();
			// The viewport ends where a visible scrollbar begins (or at the padding,
			// whichever comes first): bars never cover content.
			if (m_vVisible)
				cb.w = std::min(cb.w, (size.x - v_bar_extent()) - cb.x);
			if (m_hVisible)
				cb.h = std::min(cb.h, (size.y - h_bar_extent()) - cb.y);
			cb.w = std::max(0.0f, cb.w);
			cb.h = std::max(0.0f, cb.h);
			return cb;
		}

		Rectangle ScrollableWidget::getGlobalPureContentBounds(void) const
		{
			Rectangle r = Widget::getGlobalPureContentBounds();
			const Vec2 size = getSize();
			const Vec2 off = getContentOffset();
			if (m_vVisible)
				r.w = std::max(0.0f, std::min(r.w, size.x - v_bar_extent() - off.x));
			if (m_hVisible)
				r.h = std::max(0.0f, std::min(r.h, size.y - h_bar_extent() - off.y));
			return r;
		}

		Rectangle ScrollableWidget::getScrollArea(void) const
		{
			const Rectangle ext = getContentExtents();
			const Rectangle view = getContentBounds();
			const f32 left   = std::min(0.0f, ext.x);
			const f32 top    = std::min(0.0f, ext.y);
			const f32 right  = std::max(view.w, ext.x + ext.w);
			const f32 bottom = std::max(view.h, ext.y + ext.h);
			return { left, top, right - left, bottom - top };
		}

		void ScrollableWidget::refresh_scrollbars(void)
		{
			if (m_refreshing)
				return;
			m_refreshing = true;

			// Showing one bar shrinks the viewport, which can make the other one
			// necessary; and the new viewport can move anchored children. Iterate
			// until stable (two passes are enough in practice; the cap guards
			// against content that keeps reacting to the viewport size).
			for (i32 pass = 0; pass < 4; pass++)
			{
				const Rectangle full = Widget::getContentBounds();  // without any bar
				const Rectangle ext = getContentExtents();
				const f32 vSpace = m_vScrollbar.getThickness() + bar_inset();
				const f32 hSpace = m_hScrollbar.getThickness() + bar_inset();
				const Vec2 size = getSize();
				// Viewport size for a given combination of bars.
				auto l_viewW = [&](bool withV) { return withV ? std::min(full.w, (size.x - vSpace) - full.x) : full.w; };
				auto l_viewH = [&](bool withH) { return withH ? std::min(full.h, (size.y - hSpace) - full.y) : full.h; };
				// Content sticking out on either side of the viewport needs scrolling.
				auto l_overflows = [](f32 extBegin, f32 extLength, f32 viewLength) {
					const f32 begin = std::min(0.0f, extBegin);
					const f32 end = std::max(viewLength, extBegin + extLength);
					return (end - begin) > viewLength + 0.5f;
				};

				bool needV = isVScrollEnabled() && l_overflows(ext.y, ext.h, l_viewH(false));
				bool needH = isHScrollEnabled() && l_overflows(ext.x, ext.w, l_viewW(false));
				if (needV && !needH)
					needH = isHScrollEnabled() && l_overflows(ext.x, ext.w, l_viewW(true));
				if (needH && !needV)
					needV = isVScrollEnabled() && l_overflows(ext.y, ext.h, l_viewH(true));

				if (needV == m_vVisible && needH == m_hVisible)
					break;
				m_vVisible = needV;
				m_hVisible = needH;
				// Viewport changed: re-anchor the children against it.
				m_appliedBarExtents = { v_bar_extent(), h_bar_extent() };
				Widget::layoutChildren();
			}
			// Same visibility but different bar thickness (e.g. new theme).
			const Vec2 barExtents { v_bar_extent(), h_bar_extent() };
			if (barExtents != m_appliedBarExtents)
			{
				m_appliedBarExtents = barExtents;
				Widget::layoutChildren();
			}

			m_refreshing = false;
			sync_scrollbars();
		}

		void ScrollableWidget::sync_scrollbars(void)
		{
			m_scrollOffset = clamp_scroll_offset(m_scrollOffset);

			const Vec2 size = getSize();
			const Vec2 off = getContentOffset();
			const f32 inset = bar_inset();
			const f32 vT = m_vScrollbar.getThickness();
			const f32 hT = m_hScrollbar.getThickness();
			const Rectangle area = getScrollArea();
			const Rectangle view = getContentBounds();

			m_vScrollbar.setVisible(m_vVisible);
			if (m_vVisible)
			{
				const f32 top = std::max(off.y, inset);
				const f32 bottom = size.y - inset - (m_hVisible ? hT : 0.0f);
				m_vScrollbar.setBounds(size.x - inset - vT, top, vT, std::max(0.0f, bottom - top));
				m_vScrollbar.setRange(area.h, view.h, -m_scrollOffset.y - area.y);
			}

			m_hScrollbar.setVisible(m_hVisible);
			if (m_hVisible)
			{
				const f32 left = std::max(off.x, inset);
				const f32 right = size.x - inset - (m_vVisible ? vT : 0.0f);
				m_hScrollbar.setBounds(left, size.y - inset - hT, std::max(0.0f, right - left), hT);
				m_hScrollbar.setRange(area.w, view.w, -m_scrollOffset.x - area.x);
			}
		}

		Vec2 ScrollableWidget::clamp_scroll_offset(const Vec2& offset) const
		{
			// The visible window in content coordinates is [-offset, -offset + view];
			// it must stay inside the scroll area. Without a bar, the axis doesn't scroll.
			const Rectangle area = getScrollArea();
			const Rectangle view = getContentBounds();
			Vec2 result { 0, 0 };
			if (m_hVisible)
				result.x = std::clamp(offset.x, view.w - (area.x + area.w), -area.x);
			if (m_vVisible)
				result.y = std::clamp(offset.y, view.h - (area.y + area.h), -area.y);
			return result;
		}

		void ScrollableWidget::drawScrollbars(ogfx::BasicRenderer2D& gfx)
		{
			if (m_vVisible)
				m_vScrollbar.__draw(gfx);
			if (m_hVisible)
				m_hScrollbar.__draw(gfx);
		}

		void ScrollableWidget::updateScrollbarsSize(void)
		{
			refresh_scrollbars();
		}

		void ScrollableWidget::resetScroll(bool horizontal, bool vertical, bool propagate)
		{
			if (vertical)
			{
				m_scrollOffset.y = 0;
				m_scrollVelocity.y = 0;
			}
			if (horizontal)
			{
				m_scrollOffset.x = 0;
				m_scrollVelocity.x = 0;
			}
			m_scrollOffset = clamp_scroll_offset(m_scrollOffset);
			if (propagate)
			{
				for (auto& w : getChildren().getWidgets())
				{
					if (w == nullptr) continue;
					w->resetScroll(horizontal, vertical, propagate);
				}
			}
		}

		bool ScrollableWidget::isMouseInsideVScrollbar(void) const
		{
			return m_vVisible && (m_vScrollbar.isMouseInside() || m_vScrollbar.isDragging());
		}

		bool ScrollableWidget::isMouseInsideHScrollbar(void) const
		{
			return m_hVisible && (m_hScrollbar.isMouseInside() || m_hScrollbar.isDragging());
		}

		void ScrollableWidget::onMouseScrolled(const Event& event)
		{
			f32 dx = event.mouse->scrollAmount.x;
			f32 dy = event.mouse->scrollAmount.y;
			// The vertical wheel scrolls horizontally while hovering the horizontal bar
			// (or when there is nothing to scroll vertically).
			if (dx == 0 && m_hVisible && (m_hScrollbar.isMouseInside() || !m_vVisible))
			{
				dx = dy;
				dy = 0;
			}
			bool used = false;
			if (dy != 0 && m_vVisible)
			{
				m_scrollVelocity.y += m_scrollSpeed * dy * m_scrollSpeedMultiplier;
				used = true;
			}
			if (dx != 0 && m_hVisible)
			{
				m_scrollVelocity.x += m_scrollSpeed * dx * m_scrollSpeedMultiplier;
				used = true;
			}
			// Only consume the wheel if we could scroll, so an enclosing scrollable
			// widget gets it otherwise.
			if (!used)
				return;
			if (m_smoothScrollTimer.isStopped())
				m_smoothScrollTimer.restart();
			event.handle();
		}

		void ScrollableWidget::setScrollOffset(const Vec2& offset)
		{
			m_scrollOffset = clamp_scroll_offset(offset);
			sync_scrollbars();
		}

		void ScrollableWidget::addScrollOffset(const Vec2& offset)
		{
			setScrollOffset(m_scrollOffset + offset);
		}

		bool ScrollableWidget::needsVScroll(void) const
		{
			return m_vVisible;
		}

		bool ScrollableWidget::needsHScroll(void) const
		{
			return m_hVisible;
		}

		void ScrollableWidget::onWidgetAdded(Widget& child)
		{
			// Keep the scrollbars last, so they are hit-tested before the content.
			removeWidget(m_vScrollbar);
			removeWidget(m_hScrollbar);
			addWidget(m_vScrollbar, { 0, 0 }, true);
			addWidget(m_hScrollbar, { 0, 0 }, true);
		}

		f32 ScrollableWidget::getVScrollbarSize(void) const
		{
			return m_vVisible ? m_vScrollbar.getThickness() : 0.0f;
		}

		f32 ScrollableWidget::getHScrollbarSize(void) const
		{
			return m_hVisible ? m_hScrollbar.getThickness() : 0.0f;
		}
	}
}
