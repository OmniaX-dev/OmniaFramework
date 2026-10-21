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

#include "Widget.hpp"
#include "gui/Events.hpp"
#include "io/Memory.hpp"
#include "../..//render/BasicRenderer.hpp"
#include "../Window.hpp"
#include <algorithm>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		ostd::Object* Widget::s_dragAndDropData { nullptr };
		bool Widget::s_hasDragAndDropData { false };

		Widget::Widget(const Rectangle& bounds, Window& window) : Rectangle(bounds), m_widgets(window, *this)
		{
			m_window = &window;
		}

		Widget::~Widget(void)
		{
			if (m_window && m_tabIndex >= 0)
				m_window->getFocusManager().unregisterTabIndex(*this);
			auto& fm = m_window->getFocusManager();
			if (fm.getFocused() == this)
				fm.clearFocus();  // you'd add this method
		}

		bool Widget::addWidget(Widget& child, const Vec2& position, bool __skip_callback)
		{
			if (!m_allowChildren)
				return false;
			child.reloadTheme();
			bool result = m_widgets.addWidget(child);
			if (!result) return false;
			// The position is applied after parenting so that the child's offsets
			// are computed against our content area.
			if (position.x != 0 || position.y != 0)
				child.setPosition(position);
			else
				child.apply_anchors(false);
			if (!__skip_callback)
				onWidgetAdded(child);
			if (managesChildLayout())
			{
				updateMinimumSize();
				layoutChildren();
			}
			return result;
		}

		bool Widget::removeWidget(Widget& child)
		{
			bool result = m_widgets.removeWidget(child);
			if (result && managesChildLayout())
			{
				updateMinimumSize();
				layoutChildren();
			}
			return result;
		}

		Vec2 Widget::getGlobalPosition(void) const
		{
			Vec2 glob = getPosition();
			if (!m_rootChild && m_parent != nullptr)
			{
				glob += m_parent->getGlobalPosition();
				if (!isIgnoreScrollEnabled())
				{
					glob += m_parent->getPadding().getPosition();
					glob += m_parent->getContentOffset();
					glob += m_parent->getScrollOffset();
				}
			}
			glob += m_margin.getPosition();
			return glob;
		}

		Vec2 Widget::getGlobalContentPosition(void) const
		{
			return getGlobalPosition() + getContentBounds().getPosition();
		}

		Rectangle Widget::getGlobalBounds(void) const
		{
			return { getGlobalPosition(), getSize() + (m_margin.getSize() * 2) };
		}

		Rectangle Widget::getContentBounds(void) const
		{
			// Padding is stored as { left, top, right, bottom }.
			const auto pad = getPadding();
			const Vec2 padTotal { pad.left() + pad.right(), pad.top() + pad.bottom() };
			return { m_contentOffset + pad.getPosition(), getSize() - m_contentOffset - padTotal };
		}

		Rectangle Widget::getPureContentBounds(void) const
		{
			return { { m_contentOffset.x, m_contentOffset.y }, (getSize() - m_contentOffset) };
		}

		Rectangle Widget::getGlobalContentBounds(void) const
		{
			return { getGlobalContentPosition(), getContentBounds().getSize() };
		}

		Rectangle Widget::getGlobalPureContentBounds(void) const
		{
			return { getGlobalPosition() + getContentOffset(), getPureContentBounds().getSize() };
		}

		Rectangle Widget::getContentExtents(void) const
		{
			// Bounding box of the children in our content frame (same space as child
			// positions). It always includes the origin, and extends to negative
			// coordinates when a child sits left of / above it.
			f32 minX = 0, minY = 0, maxX = 0, maxY = 0;
			for (auto* child : m_widgets.getWidgets())
			{
				if (!child || child->isInvalid()) continue;
				if (child->isIgnoreScrollEnabled()) continue;
				if (!child->isVisible()) continue;
				const Vec2 localPos = child->getPosition() + child->getMargin().getPosition();
				minX = std::min(minX, localPos.x);
				minY = std::min(minY, localPos.y);
				maxX = std::max(maxX, localPos.x + child->getw() + child->getMargin().w);
				maxY = std::max(maxY, localPos.y + child->geth() + child->getMargin().h);
			}
			return { minX, minY, maxX - minX, maxY - minY };
		}

		bool Widget::contains(Vec2 p, bool includeBounds) const
		{
			return Rectangle(getGlobalPosition(), getSize()).contains(p, includeBounds);
		}

		void Widget::enable(bool enable)
		{
			m_enabled = enable;
			setThemeQualifier("disabled", !enable);
		}

		void Widget::addThemeOverride(const String& fullKey, ostd::Stylesheet::TypeVariant value)
		{
			m_themeOverrides.push_back({ fullKey, value });
		}

		void Widget::reloadTheme(bool propagate)
		{
			if (getWindow().theme() == nullptr)
				return;
			if (!isThemingEnabled())
				return;

			auto& theme = *getWindow().theme();
			applyTheme(theme);
			ostd::Stylesheet& const_cast_theme = const_cast<ostd::Stylesheet&>(theme);

			struct tBackup {
				const ostd::Stylesheet::TypeVariant* ptr;
				ostd::Stylesheet::TypeVariant val;
				String key;
			};

			stdvec<tBackup> backup;
			for (auto& rule : m_themeOverrides)
			{
				auto currentValuePtr = theme.getFull(rule.fullKey);
				ostd::Stylesheet::TypeVariant currentValue;
				if (currentValuePtr)
					currentValue = *currentValuePtr;
				backup.push_back({ currentValuePtr, currentValue, rule.fullKey });
				const_cast_theme.setFull(rule.fullKey, rule.value);
			}
			__applyTheme(const_cast_theme, propagate);
			for (auto&[ptr, val, key] : backup)
			{
				if (ptr == nullptr)
					const_cast_theme.removeRule(key);
				else
					const_cast_theme.setFull(key, val);
			}
		}

		void Widget::setThemeQualifier(const String& qualifier, bool value)
		{
			for (auto& [name, state] : m_qualifierList)
			{
				if (name == qualifier)
				{
					if (state == value) return;
					state = value;
					reloadTheme();
					return;
				}
			}
		}

		bool Widget::getThemeQualifier(const String& qualifier) const
		{
			for (auto& [name, state] : m_qualifierList)
			{
				if (name == qualifier)
					return state;
			}
			return false;
		}

		bool Widget::addThemeID(const String& id)
		{
			if (STDVEC_CONTAINS(m_themeIDList, id))
				return false;
			m_themeIDList.push_back(id);
			return true;
		}

		bool Widget::removeThemeID(const String& id)
		{
			if (!STDVEC_CONTAINS(m_themeIDList, id))
				return false;
			STDVEC_REMOVE(m_themeIDList, id);
			return true;
		}

		void Widget::setVisible(bool v)
		{
			if (m_visible == v)
				return;
			m_visible = v;
			parent_child_list_changed();
		}

		bool Widget::setTabIndex(i32 index)
		{
			if (getWindow().getFocusManager().registerTabIndex(*this, index))
			{
				m_tabIndex = index;
				return true;
			}
			return false;
		}

		// ============================================================================
		// LAYOUT
		// ============================================================================

		void Widget::setx(f32 xx)
		{
			Rectangle::setx(xx);
			sync_offsets_h();
		}

		void Widget::sety(f32 yy)
		{
			Rectangle::sety(yy);
			sync_offsets_v();
		}

		void Widget::setw(f32 ww)
		{
			const f32 old = w;
			Rectangle::setw(ww);
			sync_offsets_h();
			if (old != w)
				notify_size_changed();
		}

		void Widget::seth(f32 hh)
		{
			const f32 old = h;
			Rectangle::seth(hh);
			sync_offsets_v();
			if (old != h)
				notify_size_changed();
		}

		void Widget::setSize(const Vec2& size)
		{
			setSize(size.x, size.y);
		}

		void Widget::setSize(f32 ww, f32 hh)
		{
			begin_size_batch();
			setw(ww);
			seth(hh);
			end_size_batch();
		}

		void Widget::setBounds(f32 xx, f32 yy, f32 ww, f32 hh)
		{
			begin_size_batch();
			setx(xx);
			sety(yy);
			setw(ww);
			seth(hh);
			end_size_batch();
		}

		void Widget::setLayoutRect(const Rectangle& rect)
		{
			m_applyingLayout = true;
			begin_size_batch();
			setx(rect.x);
			sety(rect.y);
			setw(rect.w);
			seth(rect.h);
			m_applyingLayout = false;
			end_size_batch();
		}

		void Widget::setAnchors(const Anchors& anchors, bool keepRect)
		{
			m_layoutData.anchors = anchors;
			if (keepRect)
			{
				sync_offsets_h();
				sync_offsets_v();
			}
			else
				apply_anchors(true);
		}

		void Widget::setAnchor(eSide side, f32 value, bool keepRect)
		{
			Anchors a = m_layoutData.anchors;
			switch (side)
			{
				case eSide::Left:   a.left   = value; break;
				case eSide::Top:    a.top    = value; break;
				case eSide::Right:  a.right  = value; break;
				case eSide::Bottom: a.bottom = value; break;
			}
			setAnchors(a, keepRect);
		}

		void Widget::setOffsets(const Offsets& offsets)
		{
			m_layoutData.offsets = offsets;
			apply_anchors(true);
		}

		void Widget::setOffset(eSide side, f32 value)
		{
			Offsets o = m_layoutData.offsets;
			switch (side)
			{
				case eSide::Left:   o.left   = value; break;
				case eSide::Top:    o.top    = value; break;
				case eSide::Right:  o.right  = value; break;
				case eSide::Bottom: o.bottom = value; break;
			}
			setOffsets(o);
		}

		void Widget::setAnchorPreset(eAnchorPreset preset, ePresetResizeMode resizeMode, f32 margin)
		{
			// Each preset is a combination of one horizontal and one vertical mode.
			enum class eAxis : u8 { Begin, End, Center, Wide };
			eAxis hor = eAxis::Begin, ver = eAxis::Begin;
			switch (preset)
			{
				case eAnchorPreset::TopLeft:      hor = eAxis::Begin;  ver = eAxis::Begin;  break;
				case eAnchorPreset::TopRight:     hor = eAxis::End;    ver = eAxis::Begin;  break;
				case eAnchorPreset::BottomLeft:   hor = eAxis::Begin;  ver = eAxis::End;    break;
				case eAnchorPreset::BottomRight:  hor = eAxis::End;    ver = eAxis::End;    break;
				case eAnchorPreset::CenterLeft:   hor = eAxis::Begin;  ver = eAxis::Center; break;
				case eAnchorPreset::CenterTop:    hor = eAxis::Center; ver = eAxis::Begin;  break;
				case eAnchorPreset::CenterRight:  hor = eAxis::End;    ver = eAxis::Center; break;
				case eAnchorPreset::CenterBottom: hor = eAxis::Center; ver = eAxis::End;    break;
				case eAnchorPreset::Center:       hor = eAxis::Center; ver = eAxis::Center; break;
				case eAnchorPreset::LeftWide:     hor = eAxis::Begin;  ver = eAxis::Wide;   break;
				case eAnchorPreset::TopWide:      hor = eAxis::Wide;   ver = eAxis::Begin;  break;
				case eAnchorPreset::RightWide:    hor = eAxis::End;    ver = eAxis::Wide;   break;
				case eAnchorPreset::BottomWide:   hor = eAxis::Wide;   ver = eAxis::End;    break;
				case eAnchorPreset::VCenterWide:  hor = eAxis::Center; ver = eAxis::Wide;   break;
				case eAnchorPreset::HCenterWide:  hor = eAxis::Wide;   ver = eAxis::Center; break;
				case eAnchorPreset::FullRect:     hor = eAxis::Wide;   ver = eAxis::Wide;   break;
			}

			Vec2 size = getSize();
			const Vec2 minSize = getCombinedMinSize();
			switch (resizeMode)
			{
				case ePresetResizeMode::KeepSize:   break;
				case ePresetResizeMode::MinSize:    size = minSize; break;
				case ePresetResizeMode::KeepWidth:  size.y = minSize.y; break;
				case ePresetResizeMode::KeepHeight: size.x = minSize.x; break;
			}

			// Computes (anchorBegin, anchorEnd, offsetBegin, offsetEnd, grow) for one axis.
			auto l_axis = [margin](eAxis mode, f32 length, f32& aBegin, f32& aEnd, f32& oBegin, f32& oEnd, eGrowDirection& grow) {
				switch (mode)
				{
					case eAxis::Begin:
						aBegin = 0.0f; aEnd = 0.0f;
						oBegin = margin; oEnd = margin + length;
						grow = eGrowDirection::End;
						break;
					case eAxis::End:
						aBegin = 1.0f; aEnd = 1.0f;
						oBegin = -margin - length; oEnd = -margin;
						grow = eGrowDirection::Begin;
						break;
					case eAxis::Center:
						aBegin = 0.5f; aEnd = 0.5f;
						oBegin = -length * 0.5f; oEnd = oBegin + length;
						grow = eGrowDirection::Both;
						break;
					case eAxis::Wide:
						aBegin = 0.0f; aEnd = 1.0f;
						oBegin = margin; oEnd = -margin;
						grow = eGrowDirection::End;
						break;
				}
			};

			auto& ld = m_layoutData;
			l_axis(hor, size.x, ld.anchors.left, ld.anchors.right, ld.offsets.left, ld.offsets.right, ld.growH);
			l_axis(ver, size.y, ld.anchors.top, ld.anchors.bottom, ld.offsets.top, ld.offsets.bottom, ld.growV);
			apply_anchors(true);
		}

		void Widget::setRelativeBounds(const Rectangle& fractions, const Offsets& insets)
		{
			m_layoutData.anchors = { fractions.x, fractions.y, fractions.x + fractions.w, fractions.y + fractions.h };
			m_layoutData.offsets = { insets.left, insets.top, -insets.right, -insets.bottom };
			apply_anchors(true);
		}

		void Widget::setGrowDirection(eGrowDirection horizontal, eGrowDirection vertical)
		{
			m_layoutData.growH = horizontal;
			m_layoutData.growV = vertical;
			apply_anchors(false);
		}

		void Widget::setMinSize(const Vec2& size)
		{
			m_layoutData.minSize = size;
			updateMinimumSize();
		}

		Vec2 Widget::getCombinedMinSize(void) const
		{
			const Vec2 content = getContentMinSize();
			return { std::max(m_layoutData.minSize.x, content.x), std::max(m_layoutData.minSize.y, content.y) };
		}

		void Widget::setSizeFlags(eSizeFlags horizontal, eSizeFlags vertical)
		{
			m_layoutData.sizeFlagsH = horizontal;
			m_layoutData.sizeFlagsV = vertical;
			if (m_parent && !m_rootChild && m_parent->managesChildLayout())
				m_parent->layoutChildren();
		}

		void Widget::setStretchRatio(f32 ratio)
		{
			m_layoutData.stretchRatio = std::max(0.0f, ratio);
			if (m_parent && !m_rootChild && m_parent->managesChildLayout())
				m_parent->layoutChildren();
		}

		bool Widget::isLayoutManaged(void) const
		{
			if (m_parent && !m_rootChild && m_parent->managesChildLayout())
				return true;
			const auto& a = m_layoutData.anchors;
			return a.left != a.right || a.top != a.bottom;
		}

		void Widget::layoutChildren(void)
		{
			for (auto* c : m_widgets.getWidgets())
			{
				if (!c || c->isInvalid()) continue;
				c->apply_anchors(false);
			}
		}

		void Widget::updateLayout(void)
		{
			apply_anchors(true);
		}

		void Widget::updateMinimumSize(void)
		{
			// Nobody depends on our minimum size until we have a parent (this also
			// keeps content measurement away from widgets constructed before the
			// window's renderer is initialized).
			if (!m_parent || m_rootChild)
				return;
			const Vec2 minSize = getCombinedMinSize();
			if (minSize == m_lastMinSize)
				return;
			m_lastMinSize = minSize;
			if (m_parent->managesChildLayout())
			{
				m_parent->updateMinimumSize();
				m_parent->layoutChildren();
			}
			else
				apply_anchors(false);
		}

		void Widget::applyAutoSize(const Vec2& newSize)
		{
			const f32 dw = newSize.x - getw();
			const f32 dh = newSize.y - geth();
			f32 nx = getx(), ny = gety();
			if (m_layoutData.growH == eGrowDirection::Begin) nx -= dw;
			else if (m_layoutData.growH == eGrowDirection::Both) nx -= dw * 0.5f;
			if (m_layoutData.growV == eGrowDirection::Begin) ny -= dh;
			else if (m_layoutData.growV == eGrowDirection::Both) ny -= dh * 0.5f;
			setBounds(nx, ny, newSize.x, newSize.y);
		}

		Vec2 Widget::contentToWidgetSize(const Vec2& contentSize) const
		{
			// Inverse of getContentBounds().
			return contentSize + m_contentOffset + Vec2 { m_padding.left() + m_padding.right(), m_padding.top() + m_padding.bottom() };
		}

		void Widget::setPadding(const Rectangle& padding)
		{
			if (m_padding == padding)
				return;
			m_padding = padding;
			layoutChildren();
			updateMinimumSize();
		}

		void Widget::setContentOffset(const Vec2& offset)
		{
			if (m_contentOffset == offset)
				return;
			m_contentOffset = offset;
			layoutChildren();
			updateMinimumSize();
		}

		Vec2 Widget::parent_content_size(void) const
		{
			if (!m_parent || m_rootChild)
				return { 0, 0 };
			return m_parent->getContentBounds().getSize();
		}

		void Widget::sync_offsets_h(void)
		{
			if (m_applyingLayout)
				return;
			const f32 pw = parent_content_size().x;
			m_layoutData.offsets.left = x - (m_layoutData.anchors.left * pw);
			m_layoutData.offsets.right = (x + w) - (m_layoutData.anchors.right * pw);
		}

		void Widget::sync_offsets_v(void)
		{
			if (m_applyingLayout)
				return;
			const f32 ph = parent_content_size().y;
			m_layoutData.offsets.top = y - (m_layoutData.anchors.top * ph);
			m_layoutData.offsets.bottom = (y + h) - (m_layoutData.anchors.bottom * ph);
		}

		void Widget::apply_anchors(bool force)
		{
			if (m_rootChild)
				return;
			if (m_parent && m_parent->managesChildLayout())
				return;  // the Container decides
			const auto& a = m_layoutData.anchors;
			const auto& o = m_layoutData.offsets;
			// Widgets anchored to the top-left corner don't depend on the parent's
			// size at all; leave them alone unless explicitly asked to.
			if (!force && a.isZero())
				return;

			const Vec2 ps = parent_content_size();
			f32 left   = (ps.x * a.left)   + o.left;
			f32 top    = (ps.y * a.top)    + o.top;
			f32 right  = (ps.x * a.right)  + o.right;
			f32 bottom = (ps.y * a.bottom) + o.bottom;

			// Grow according to the grow direction if the rect is below minimum size.
			auto l_grow = [](f32& begin, f32& end, f32 minLen, eGrowDirection dir) {
				if (end - begin >= minLen)
					return;
				switch (dir)
				{
					case eGrowDirection::Begin: begin = end - minLen; break;
					case eGrowDirection::End:   end = begin + minLen; break;
					case eGrowDirection::Both:
					{
						const f32 center = (begin + end) * 0.5f;
						begin = center - (minLen * 0.5f);
						end = begin + minLen;
						break;
					}
				}
			};
			const Vec2 minSize = getCombinedMinSize();
			l_grow(left, right, std::max(0.0f, minSize.x), m_layoutData.growH);
			l_grow(top, bottom, std::max(0.0f, minSize.y), m_layoutData.growV);

			// Snap to whole pixels so text and borders stay crisp.
			left = std::round(left); top = std::round(top);
			right = std::round(right); bottom = std::round(bottom);
			setLayoutRect({ left, top, right - left, bottom - top });
		}

		void Widget::notify_size_changed(void)
		{
			if (m_sizeBatchDepth > 0)
			{
				m_sizeChangedPending = true;
				return;
			}
			layoutChildren();
			onBoundsChanged(getSize());
		}

		void Widget::begin_size_batch(void)
		{
			m_sizeBatchDepth++;
		}

		void Widget::end_size_batch(void)
		{
			if (--m_sizeBatchDepth > 0)
				return;
			m_sizeBatchDepth = 0;
			if (m_sizeChangedPending)
			{
				m_sizeChangedPending = false;
				notify_size_changed();
			}
		}

		void Widget::parent_child_list_changed(void)
		{
			if (!m_parent || m_rootChild || !m_parent->managesChildLayout())
				return;
			m_parent->updateMinimumSize();
			m_parent->layoutChildren();
		}

		void Widget::setCallback(eCallback type, EventCallback callback)
		{
			switch (type)
			{
				case eCallback::MousePressed:    callback_onMousePressed    = std::move(callback); break;
				case eCallback::MouseReleased:   callback_onMouseReleased   = std::move(callback); break;
				case eCallback::MouseMoved:      callback_onMouseMoved      = std::move(callback); break;
				case eCallback::MouseScrolled:   callback_onMouseScrolled   = std::move(callback); break;
				case eCallback::DragAndDrop:     callback_onDragAndDrop     = std::move(callback); break;
				case eCallback::MouseEntered:    callback_onMouseEntered    = std::move(callback); break;
				case eCallback::MouseExited:     callback_onMouseExited     = std::move(callback); break;
				case eCallback::MouseDragged:    callback_onMouseDragged    = std::move(callback); break;
				case eCallback::KeyPressed:      callback_onKeyPressed      = std::move(callback); break;
				case eCallback::KeyReleased:     callback_onKeyReleased     = std::move(callback); break;
				case eCallback::TextEntered:     callback_onTextEntered     = std::move(callback); break;
				case eCallback::FocusGained:     callback_onFocusGained     = std::move(callback); break;
				case eCallback::FocusLost:       callback_onFocusLost       = std::move(callback); break;
				case eCallback::WindowClosed:    callback_onWindowClosed    = std::move(callback); break;
				case eCallback::WindowResized:   callback_onWindowResized   = std::move(callback); break;
				case eCallback::WindowFocused:   callback_onWindowFocused   = std::move(callback); break;
				case eCallback::WindowFocusLost: callback_onWindowFocusLost = std::move(callback); break;
				case eCallback::ActionPerformed: callback_onActionPerformed = std::move(callback); break;
				default: break;
			}
		}

		void Widget::__draw(ogfx::BasicRenderer2D& gfx)
		{
			if (!isVisible())
				return;
			beforeDraw(gfx);
			if (m_useBackgroundGradient)
				gfx.fillGradientRect({ getGlobalPosition(), getSize() }, m_backgroundGradient);
			else if (m_showBackground)
				gfx.fillRoundRect({ getGlobalPosition(), getSize() }, m_backgroundColor, m_borderRadius);
			onDraw(gfx);

			// gfx.fillRect(getGlobalPureContentBounds(), { 0, 255, 0, 120 });
			const bool needsContentClip = !m_rootChild;
			if (needsContentClip)
				gfx.pushClippingRect(getGlobalPureContentBounds(), true);
			if (hasChildren())
				m_widgets.draw(gfx);
			if (needsContentClip)
				gfx.popClippingRect();
			if (m_showBorder)
				gfx.drawRoundRect({ getGlobalPosition(), getSize() }, m_borderColor, m_borderRadius, m_borderWidth);
			afterDraw(gfx);
			if (isFocused() && isFocusBorderEnabled())
				gfx.drawRoundRect({ getGlobalPosition(), getSize() }, m_focusBorderColor, m_borderRadius, m_focusBorderWidth);
		}

		void Widget::__update(void)
		{
			onUpdate();
			if (hasChildren())
				m_widgets.update();
		}

		void Widget::__onMousePressed(const Event& event)
		{
			if (event.isHandled() || !isMousePressedEventEnabled()) return;
			setThemeQualifier("pressed");
			if (hasChildren())
				m_widgets.onMousePressed(event);
			if (!event.isHandled())
			{
				if (callback_onMousePressed)
					callback_onMousePressed(event);
				onMousePressed(event);
				m_pressedButton = event.mouse->button;
			}
		}

		void Widget::__onMouseReleased(const Event& event)
		{
			if (event.isHandled() || !isMouseReleasedEventEnabled()) return;
			setThemeQualifier("pressed", false);
			m_pressedButton = ogfx::MouseEventData::eButton::None;
			if (hasChildren())
				m_widgets.onMouseReleased(event);
			if (!event.isHandled())
			{
				if (callback_onMouseReleased)
					callback_onMouseReleased(event);
				onMouseReleased(event);
			}
		}

		void Widget::__onDragAndDrop(const Event& event)
		{
			if (callback_onDragAndDrop)
				callback_onDragAndDrop(event);
			onDragAndDrop(event);
		}

		void Widget::__onMouseMoved(const Event& event)
		{
			m_mouseInside = contains(event.mouse->position_x, event.mouse->position_y);
			if (event.isHandled() || !isMouseMovedEventEnabled()) return;
			if (hasChildren())
				m_widgets.onMouseMoved(event);
			if (!event.isHandled())
			{
				if (callback_onMouseMoved)
					callback_onMouseMoved(event);
				onMouseMoved(event);
			}
			if (isTooltipEnabled() && isMouseInside())
				getWindow().restartTooltipTimer();
		}

		void Widget::__onMouseScrolled(const Event& event)
		{
			if (event.isHandled() || !isMouseScrolledEventEnabled()) return;
			if (hasChildren())
				m_widgets.onMouseScrolled(event);
			if (!event.isHandled())
			{
				if (callback_onMouseScrolled)
					callback_onMouseScrolled(event);
				onMouseScrolled(event);
			}
		}

		void Widget::__onMouseEntered(const Event& event)
		{
			if (event.isHandled() || !isMouseEnteredEventEnabled()) return;
			setThemeQualifier("hover");
			if (callback_onMouseEntered)
				callback_onMouseEntered(event);
			onMouseEntered(event);
			if (isTooltipEnabled())
				getWindow().startTooltipTimer(getTooltipText());
		}

		void Widget::__onMouseExited(const Event& event)
		{
			// Mouse-exit is only detected by a widget's direct parent, so when the
			// mouse leaves us, children still flagged as hovered (e.g. a button
			// touching our edge, inside a container) must be exited here too.
			for (auto* child : m_widgets.getWidgets())
			{
				if (!child || child->isInvalid() || !child->m_mouseInside) continue;
				child->m_mouseInside = false;
				child->__onMouseExited(event);
			}
			if (event.isHandled() || !isMouseExitedEventEnabled()) return;
			setThemeQualifier("hover", false);
			if (callback_onMouseExited)
				callback_onMouseExited(event);
			onMouseExited(event);
			if (isTooltipEnabled())
				getWindow().stopTooltipTimer();
		}

		void Widget::__onMouseDragged(const Event& event)
		{
			m_mouseInside = contains(event.mouse->position_x, event.mouse->position_y);
			if (event.isHandled() || !isMouseDraggedEventEnabled()) return;
			if (hasChildren())
				m_widgets.onMouseDragged(event);
			if (!event.isHandled())
			{
				if (callback_onMouseDragged)
					callback_onMouseDragged(event);
				onMouseDragged(event);
			}
			if (isTooltipEnabled() && isMouseInside())
				getWindow().restartTooltipTimer();
		}

		void Widget::__onKeyPressed(const Event& event)
		{
			if (event.isHandled() || !isKeyPressedEventEnabled()) return;
			if (callback_onKeyPressed)
				callback_onKeyPressed(event);
			onKeyPressed(event);
		}

		void Widget::__onKeyReleased(const Event& event)
		{
			if (event.isHandled() || !isKeyReleasedEventEnabled()) return;
			m_pressedButton = ogfx::MouseEventData::eButton::None;
			if (callback_onKeyReleased)
				callback_onKeyReleased(event);
			onKeyReleased(event);
		}

		void Widget::__onTextEntered(const Event& event)
		{
			if (event.isHandled() || !isTextEnteredEventEnabled()) return;
			if (callback_onTextEntered)
				callback_onTextEntered(event);
			onTextEntered(event);
		}

		void Widget::__onWindowClosed(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowClosed(event);
			if (!event.isHandled())
			{
				if (callback_onWindowClosed)
					callback_onWindowClosed(event);
				onWindowClosed(event);
			}
		}

		void Widget::__onWindowResized(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowResized(event);
			if (!event.isHandled())
			{
				if (callback_onWindowResized)
					callback_onWindowResized(event);
				onWindowResized(event);
			}
		}

		void Widget::__onWindowFocused(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowFocused(event);
			if (!event.isHandled())
			{
				if (callback_onWindowFocused)
					callback_onWindowFocused(event);
				onWindowFocused(event);
			}
		}

		void Widget::__onWindowFocusLost(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowFocusLost(event);
			if (!event.isHandled())
			{
				if (callback_onWindowFocusLost)
					callback_onWindowFocusLost(event);
				onWindowFocusLost(event);
			}
		}

		void Widget::__applyTheme(const ostd::Stylesheet& theme, bool propagate)
		{
			if (propagate && hasChildren())
				m_widgets.onThemeApplied(theme);
			apply_common_theme_values(theme);
			applyTheme(theme);
		}

		void Widget::apply_common_theme_values(const ostd::Stylesheet& theme)
		{
			setTextColor(getThemeValue<Color>(theme, "textColor", m_textColor));
			setBackgroundColor(getThemeValue<Color>(theme, "backgroundColor", m_backgroundColor));
			setFontSize(getThemeValue<i32>(theme, "fontSize", m_fontSize));
			setBorderRadius(getThemeValue<i32>(theme, "borderRadius", m_borderRadius));
			setBorderWidth(getThemeValue<i32>(theme, "borderWidth", m_borderWidth));
			enableBorder(getThemeValue<bool>(theme, "showBorder", m_showBorder));
			setBorderColor(getThemeValue<Color>(theme, "borderColor", m_borderColor));
			enableFocusBorder(getThemeValue<bool>(theme, "showFocusBorder", m_showFocusBorder));
			setFocusBorderColor(getThemeValue<Color>(theme, "focusBorderColor", m_focusBorderColor));
			setFocusBorderWidth(getThemeValue<i32>(theme, "focusBorderWidth", m_focusBorderWidth));
			enableBackground(getThemeValue<bool>(theme, "showBackground", m_showBackground));
			setPadding(getThemeValue<Rectangle>(theme, "padding", m_padding));
			setMargin(getThemeValue<Rectangle>(theme, "margin", m_margin));
			enableBackgroundGradient(getThemeValue<bool>(theme, "useBackgroundGradient", m_useBackgroundGradient));
			setBackgroundGradient(getThemeValue<ColorGradient>(theme, "backgroundGradient", m_backgroundGradient));
			if (isBackgroundGradientEnabled() && isBackgroundEnabled())
				enableBackground(false);
		}
	}
}
