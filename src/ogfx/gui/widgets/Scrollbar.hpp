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

#pragma once

#include <ogfx/gui/widgets/Widget.hpp>
#include <ostd/utils/Time.hpp>
#include <functional>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// ScrollBar — a track with a draggable thumb, for one axis.
		//
		// It knows nothing about widgets or offsets: its owner describes the scroll
		// state with setRange(contentLength, viewportLength, position), where
		// position is how far the viewport is from the start of the content
		// (0 .. contentLength - viewportLength). When the user drags the thumb or
		// clicks the track, the scroll callback receives the new position.
		//
		// While dragging, the bar follows the mouse through the window's global
		// mouse signals, so the drag keeps working when the cursor leaves the bar
		// (or the window area of the owning widget).
		// ============================================================================
		class ScrollBar : public Widget
		{
			public:
				using ScrollCallback = std::function<void(f32 newPosition)>;

			public:
				ScrollBar(Window& window, eOrientation orientation);
				ScrollBar& create(void);
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMousePressed(const Event& event) override;
				void handleSignal(ostd::Signal& signal) override;

				void setRange(f32 contentLength, f32 viewportLength, f32 position);
				inline f32 getContentLength(void) const { return m_contentLength; }
				inline f32 getViewportLength(void) const { return m_viewportLength; }
				inline f32 getScrollPosition(void) const { return m_position; }
				inline f32 getMaxScrollPosition(void) const { return std::max(0.0f, m_contentLength - m_viewportLength); }

				inline void setScrollCallback(ScrollCallback callback) { m_scrollCallback = std::move(callback); }
				inline eOrientation getOrientation(void) const { return m_orientation; }
				inline bool isVertical(void) const { return m_orientation == eOrientation::Vertical; }
				inline bool isDragging(void) const { return m_dragging; }
				// Thickness across the scroll axis (theme: scrollbar.width).
				inline f32 getThickness(void) const { return m_thickness; }
				// Thumb rectangle in global coordinates.
				Rectangle getGlobalThumbBounds(void) const;

			private:
				f32 track_length(void) const;
				f32 thumb_length(void) const;
				f32 thumb_offset(void) const;
				f32 local_axis_position(f32 globalX, f32 globalY) const;
				void drag_to(f32 localAxisPos);

			private:
				eOrientation m_orientation;
				f32 m_contentLength { 0.0f };
				f32 m_viewportLength { 0.0f };
				f32 m_position { 0.0f };
				ScrollCallback m_scrollCallback { nullptr };

				bool m_dragging { false };
				f32 m_grabOffset { 0.0f };

				f32 m_thickness { 15.0f };
				f32 m_minThumbLength { 20.0f };
				f32 m_thumbInset { 2.0f };
				f32 m_thumbBorderRadius { 16.0f };
				Color m_trackColor { 70, 70, 70 };
				Color m_thumbColor { 120, 120, 120 };
				Color m_thumbBorderColor { 150, 150, 150 };
				bool m_thumbShowBorder { true };
		};


		// ============================================================================
		// ScrollableWidget — base for widgets whose content can be larger than the
		// widget itself (Panel, TreeView, DetailList, ...).
		//
		// Model:
		//   - The content area (getContentBounds()) is the VIEWPORT: the space left
		//     after padding, content offset and any visible scrollbar. Scrollbars
		//     reserve their space, they never cover content. Anchored children are
		//     laid out against the viewport.
		//   - The scroll area is the bounding box of the content (getContentExtents(),
		//     in content-frame coordinates, possibly extending left of / above the
		//     origin) united with the viewport itself.
		//   - A scrollbar is shown exactly when the scroll area exceeds the viewport
		//     along its axis (and scrolling is enabled on that axis). Hidden
		//     scrollbars take no space and receive no input.
		//   - getScrollOffset() is the translation applied to the content; it is
		//     always kept inside the valid range.
		// ============================================================================
		class ScrollableWidget : public Widget
		{
			public:
				inline ScrollableWidget(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				ScrollableWidget& create(void);
				virtual void onUpdate(void) override;
				void layoutChildren(void) override;

				// Viewport (local): content area minus the visible scrollbars.
				Rectangle getContentBounds(void) const override;
				// Global clip/hit area for children: from the content offset to the
				// scrollbars (padding included, scrollbars excluded).
				Rectangle getGlobalPureContentBounds(void) const override;
				// Content extents united with the viewport, in content-frame coordinates.
				Rectangle getScrollArea(void) const;

				void drawScrollbars(ogfx::BasicRenderer2D& gfx);
				// Re-evaluates scrollbar visibility/geometry right away (it also happens
				// automatically every frame and whenever the widget is resized).
				void updateScrollbarsSize(void);
				void resetScroll(bool horizontal = true, bool vertical = true, bool propagate = true) override;
				bool isMouseInsideVScrollbar(void) const;
				bool isMouseInsideHScrollbar(void) const;
				inline bool isMouseInsideAnyScrollbar(void) const { return isMouseInsideVScrollbar() || isMouseInsideHScrollbar(); }
				virtual void onMouseScrolled(const Event& event) override;
				virtual void setScrollOffset(const Vec2& offset) override;
				virtual void addScrollOffset(const Vec2& offset) override;
				virtual bool needsVScroll(void) const override;
				virtual bool needsHScroll(void) const override;
				virtual void onWidgetAdded(Widget& child) override;
				virtual f32 getVScrollbarSize(void) const override;
				virtual f32 getHScrollbarSize(void) const override;
				inline Vec2 getScrollOffset(void) const override { return m_scrollOffset; }
				inline void setScrollSpeed(f32 speed) { m_scrollSpeed = speed; }
				inline f32 getScrollSpeed(void) const { return m_scrollSpeed; }
				inline void setScrollSmoothFactor(f32 speed) { m_scrollSmoothFactor = std::clamp(speed, 0.0f, 1.0f); }
				inline f32 getScrollSmoothFactor(void) const { return m_scrollSmoothFactor; }
				inline ScrollBar& getVScrollbar(void) { return m_vScrollbar; }
				inline ScrollBar& getHScrollbar(void) { return m_hScrollbar; }

			private:
				void refresh_scrollbars(void);
				void sync_scrollbars(void);
				Vec2 clamp_scroll_offset(const Vec2& offset) const;
				// Space each visible scrollbar occupies at the widget's edge, border included.
				f32 v_bar_extent(void) const;
				f32 h_bar_extent(void) const;
				f32 bar_inset(void) const;

			private:
				Vec2 m_scrollOffset { 0, 0 };
				ScrollBar m_vScrollbar { getWindow(), eOrientation::Vertical };
				ScrollBar m_hScrollbar { getWindow(), eOrientation::Horizontal };
				bool m_vVisible { false };
				bool m_hVisible { false };
				bool m_refreshing { false };
				Vec2 m_appliedBarExtents { 0, 0 };  // bar space the children were laid out with
				ostd::StepTimer m_smoothScrollTimer;
				f32 m_scrollSpeed { 0.8f };
				Vec2 m_scrollVelocity { 0.0f, 0.0f };
				f32 m_scrollSmoothFactor { 0.7f };
				f32 m_scrollSpeedMultiplier { 15.0f };
		};
	}
}
