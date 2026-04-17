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
			VerticalScrollBar& VerticalScrollBar::create(const String& text)
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
				if (!getParent()->needsScroll())
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
		}
	}
}
