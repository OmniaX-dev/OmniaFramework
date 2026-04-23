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
#include <ostd/utils/Time.hpp>

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			class VerticalScrollBar : public Widget
			{
				public:
					inline VerticalScrollBar(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					VerticalScrollBar& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseDragged(const Event& event) override;
					void onMousePressed(const Event& event) override;
					void onMouseReleased(const Event& event) override;
					void onUpdate(void) override;
					bool isMouseInsideThumb(const Vec2& mouse_pos);

					inline void setx(f32 xx) override {  }
					inline void sety(f32 yy) override {  }
					inline void setw(f32 ww) override {  }
					inline void seth(f32 hh) override {  }

				private:
					void update_thumb(void);
					void set_thumb_y(f32 thumby);

				private:
					f32 m_thumbHeight { 0 };
					f32 m_thumbY { 0 };
					Rectangle m_thumbGlobalBounds { 0, 0, 0, 0 };
					bool m_mousePressed { false };
					bool m_mouseDragged { false };
					f32 m_dragGrabOffset { 0 };
					Rectangle m_correctionOffset { 1, 1, 0, 0 };

					Rectangle m_trackBorderRadii { 0, 0, 0, 0 };
					f32 m_thumbBorderRadius { 16 };
					Color m_trackColor { 70, 70, 70 };
					Color m_thumbColor { 120, 120, 120 };
					Color m_thumbBorderColor { 150, 150, 150 };
					bool m_thumbShowBorder { true };
			};
			class HorizontalScrollbar : public Widget
			{
				public:
					inline HorizontalScrollbar(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					HorizontalScrollbar& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseDragged(const Event& event) override;
					void onMousePressed(const Event& event) override;
					void onMouseReleased(const Event& event) override;
					void onUpdate(void) override;
					bool isMouseInsideThumb(const Vec2& mouse_pos);

					inline void setx(f32 xx) override {  }
					inline void sety(f32 yy) override {  }
					inline void setw(f32 ww) override {  }
					inline void seth(f32 hh) override {  }

				private:
					void update_thumb(void);
					void set_thumb_x(f32 thumbx);

				private:
					f32 m_thumbWidth { 0 };
					f32 m_thumbX { 0 };
					Rectangle m_thumbGlobalBounds { 0, 0, 0, 0 };
					bool m_mousePressed { false };
					bool m_mouseDragged { false };
					f32 m_dragGrabOffset { 0 };
					Rectangle m_correctionOffset { 0, 0, 0, 0 };

					Rectangle m_trackBorderRadii { 0, 0, 0, 0 };
					f32 m_thumbBorderRadius { 16 };
					Color m_trackColor { 70, 70, 70 };
					Color m_thumbColor { 120, 120, 120 };
					Color m_thumbBorderColor { 150, 150, 150 };
					bool m_thumbShowBorder { true };
			};
			class ScrollableWidget : public Widget
			{
				public:
					inline ScrollableWidget(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
					ScrollableWidget& create(void);
					virtual void onUpdate(void) override;
					void drawScrollbars(ogfx::BasicRenderer2D& gfx);
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

				private:
					String m_title { "Panel" };
					Vec2 m_scrollOffset { 0, 0 };
					VerticalScrollBar m_vScrollbar { getWindow() };
					HorizontalScrollbar m_hScrollbar { getWindow() };
					ostd::StepTimer m_smoothScrollTimer;
					f32 m_scrollSpeed { 0.8f };
					Vec2 m_scrollVelocity { 0.0f, 0.0f };
					f32 m_scrollSmoothFactor { 0.7f };
					f32 m_scrollSpeedMultiplier { 15.0f };
					bool m_hScrollbarAdded { false };
					bool m_vScrollbarAdded { false };
			};
		}
	}
}
