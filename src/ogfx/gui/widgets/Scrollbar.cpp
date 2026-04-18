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
				disableDrawBox();
				disableChildren();
				enableBackground(true);
				enableBorder(false);
				enableTopMost(true);
				allowIgnoreScroll(true);
				validate();
				return *this;
			}

			void VerticalScrollBar::applyTheme(const ostd::Stylesheet& theme)
			{
				w = getThemeValue<f32>(theme, "scrollbar.width", 15);
				m_thumbColor = getThemeValue<Color>(theme, "scrollbar.thumb.color", { 120, 120, 120 });
				m_thumbBorderRadius = getThemeValue<f32>(theme, "scrollbar.thumb.borderRadius", 16);
				m_thumbBorderColor = getThemeValue<Color>(theme, "scrollbar.thumb.borderColor", { 150, 150, 150 });
				m_thumbShowBorder = getThemeValue<bool>(theme, "scrollbar.thumb.showBorder", true);
				m_trackColor = getThemeValue<Color>(theme, "scrollbar.track.color", { 70, 70, 70 });
				m_trackBorderRadii = getThemeValue<Rectangle>(theme, "scrollbar.track.borderRadii", { 0, 0, 10, 0 });
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

				if (is_mouse_in_thumb({ event.mouse->position_x, event.mouse->position_y }))
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

			void VerticalScrollBar::update_thumb(void)
			{
				x = getParent()->getSize().x - w;
				y = 0;
				h = getParent()->getGlobalContentBounds().h;

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

			bool VerticalScrollBar::is_mouse_in_thumb(const Vec2& mouse_pos)
			{
				return m_thumbGlobalBounds.contains(mouse_pos, true);
			}






			HorizontalScrollbar& HorizontalScrollbar::create(void)
			{
				setPadding({ 0, 0, 0, 0 });
				setMargin({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::Label");
				disableDrawBox();
				disableChildren();
				enableBackground(true);
				enableBorder(false);
				enableTopMost(true);
				allowIgnoreScroll(true);
				validate();
				return *this;
			}

			void HorizontalScrollbar::applyTheme(const ostd::Stylesheet& theme)
			{
				h = getThemeValue<f32>(theme, "scrollbar.width", 15);
				m_thumbColor = getThemeValue<Color>(theme, "scrollbar.thumb.color", { 120, 120, 120 });
				m_thumbBorderRadius = getThemeValue<f32>(theme, "scrollbar.thumb.borderRadius", 16);
				m_thumbBorderColor = getThemeValue<Color>(theme, "scrollbar.thumb.borderColor", { 150, 150, 150 });
				m_thumbShowBorder = getThemeValue<bool>(theme, "scrollbar.thumb.showBorder", true);
				m_trackColor = getThemeValue<Color>(theme, "scrollbar.track.color", { 70, 70, 70 });
				m_trackBorderRadii = getThemeValue<Rectangle>(theme, "scrollbar.track.borderRadii", { 0, 0, 10, 10 });
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

				if (is_mouse_in_thumb({ event.mouse->position_x, event.mouse->position_y }))
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

			void HorizontalScrollbar::update_thumb(void)
			{
				x = 0;
				y = getParent()->getSize().y - h;
				w = getParent()->getGlobalBounds().w;

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
				std::cout << Rectangle::toString() << "\n";
			}

			void HorizontalScrollbar::set_thumb_x(f32 thumbx)
			{
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

			bool HorizontalScrollbar::is_mouse_in_thumb(const Vec2& mouse_pos)
			{
				return m_thumbGlobalBounds.contains(mouse_pos, true);
			}
		}
	}
}
