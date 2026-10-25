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

#include "SubWindow.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../Window.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		using eCursor = WindowCore::eCursor;

		static u64 now_ms(void)
		{
			using namespace std::chrono;
			return cast<u64>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
		}

		// ===================================================================
		// SubWindow
		// ===================================================================
		SubWindow& SubWindow::create(const String& title)
		{
			m_title = title;
			setTypeName("ogfx::gui::SubWindow");
			setStylesheetCategoryName("subwindow");
			disableFocus();
			enableStopEvents();
			// Defaults for windows without a "subwindow" theme section.
			enableBackground();
			setBackgroundColor({ 35, 35, 40 });
			enableBorder();
			setBorderColor({ 15, 15, 15 });
			setBorderWidth(1);
			setPadding({ 8, 8, 8, 8 });
			setContentOffset({ 0, m_titlebarHeight });
			validate();
			return *this;
		}

		SubWindow::~SubWindow(void)
		{
			getWindow().__forgetSubWindow(*this);
		}

		void SubWindow::open(void)
		{
			getWindow().addSubWindow(*this);
			show();
			bringToFront();
		}

		void SubWindow::close(void)
		{
			if (!isVisible())
				return;
			if (m_closeCallback && !m_closeCallback(*this))
				return;
			m_dragging = false;
			m_resizing = { };
			m_pressedButtonId = eButtonId::None;
			m_hoveredButton = eButtonId::None;
			set_cursor(cast<u8>(eCursor::Default));
			hide();
			getWindow().__onSubWindowClosed(*this);
		}

		void SubWindow::maximize(void)
		{
			if (isMaximized())
				return;
			if (m_state == eState::Normal)
				save_layout();
			set_state(eState::Maximized);
			setAnchorPreset(eAnchorPreset::FullRect);
			layoutChildren();
			bringToFront();
		}

		void SubWindow::minimize(void)
		{
			if (isMinimized())
				return;
			if (m_state == eState::Normal)
				save_layout();
			m_dragging = false;
			m_resizing = { };
			// The window assigns the slot along the bottom edge.
			set_state(eState::Minimized);
		}

		void SubWindow::restore(void)
		{
			if (m_state == eState::Normal)
				return;
			set_state(eState::Normal);
			restore_layout();
			layoutChildren();
			__clampToParent();
			bringToFront();
		}

		void SubWindow::bringToFront(void)
		{
			if (getParent() != nullptr)
				getParent()->getChildren().bringToFront(*this);
		}

		void SubWindow::setAlwaysOnTop(bool alwaysOnTop)
		{
			if (m_alwaysOnTop == alwaysOnTop)
				return;
			m_alwaysOnTop = alwaysOnTop;
			layer_changed();
		}

		void SubWindow::setModal(bool modal)
		{
			if (m_modal == modal)
				return;
			m_modal = modal;
			layer_changed();
		}

		void SubWindow::centerInParent(void)
		{
			const Vec2 area = parent_area_size();
			setPosition(std::round((area.x - getw()) * 0.5f), std::max(0.0f, std::round((area.y - geth()) * 0.5f)));
		}

		i32 SubWindow::getZLayer(void) const
		{
			if (m_modal)
				return cast<i32>(eZLayer::Modal);
			if (m_alwaysOnTop)
				return cast<i32>(eZLayer::AlwaysOnTop);
			return cast<i32>(eZLayer::Normal);
		}

		void SubWindow::layoutChildren(void)
		{
			// Minimized windows keep their children's layout for when they're restored.
			if (isMinimized())
				return;
			Widget::layoutChildren();
		}

		void SubWindow::applyTheme(const ostd::Stylesheet& theme)
		{
			m_titlebarHeight = getThemeValue<f32>(theme, "titlebarHeight", m_titlebarHeight);
			m_titlebarColor = getThemeValue<Color>(theme, "titlebarColor", m_titlebarColor);
			m_titlebarBorderColor = getThemeValue<Color>(theme, "titlebarBorderColor", m_titlebarBorderColor);
			m_titlebarBorderWidth = getThemeValue<i32>(theme, "titlebarBorderWidth", m_titlebarBorderWidth);
			m_titleColor = getThemeValue<Color>(theme, "titleColor", m_titleColor);
			m_titlebarFontSize = getThemeValue<i32>(theme, "titlebarFontSize", m_titlebarFontSize);
			m_titleTextAlign = getThemeValue<i32>(theme, "titlebarTextAlign", m_titleTextAlign);

			m_buttonSize = getThemeValue<f32>(theme, "button.size", m_buttonSize);
			m_buttonSpacing = getThemeValue<f32>(theme, "button.spacing", m_buttonSpacing);
			m_buttonBorderRadius = getThemeValue<f32>(theme, "button.borderRadius", m_buttonBorderRadius);
			m_buttonGlyphThickness = getThemeValue<f32>(theme, "button.glyphThickness", m_buttonGlyphThickness);
			m_buttonColor = getThemeValue<Color>(theme, "button.color", m_buttonColor);
			m_buttonHoverColor = getThemeValue<Color>(theme, "button.hoverColor", m_buttonHoverColor);
			m_buttonPressedColor = getThemeValue<Color>(theme, "button.pressedColor", m_buttonPressedColor);
			m_buttonActiveColor = getThemeValue<Color>(theme, "button.activeColor", m_buttonActiveColor);
			m_buttonGlyphColor = getThemeValue<Color>(theme, "button.glyphColor", m_buttonGlyphColor);
			m_buttonGlyphHoverColor = getThemeValue<Color>(theme, "button.glyphHoverColor", m_buttonGlyphHoverColor);
			m_closeHoverColor = getThemeValue<Color>(theme, "closeButton.hoverColor", m_closeHoverColor);
			m_closePressedColor = getThemeValue<Color>(theme, "closeButton.pressedColor", m_closePressedColor);

			m_resizeGripSize = getThemeValue<f32>(theme, "resizeGripSize", m_resizeGripSize);
			m_minimumSize.x = getThemeValue<f32>(theme, "minWidth", m_minimumSize.x);
			m_minimumSize.y = getThemeValue<f32>(theme, "minHeight", m_minimumSize.y);
			m_minimizedWidth = getThemeValue<f32>(theme, "minimizedWidth", m_minimizedWidth);

			if (!m_blurEnabledSet)
				m_blurEnabled = getThemeValue<bool>(theme, "blur.enabled", m_blurEnabled);
			if (!m_blurRadiusSet)
				m_blurRadius = getThemeValue<f32>(theme, "blur.radius", m_blurRadius);
			m_blurTint = getThemeValue<Color>(theme, "blur.tint", m_blurTint);

			if (!m_shadowEnabledSet)
				m_shadowEnabled = getThemeValue<bool>(theme, "shadow.enabled", m_shadowEnabled);
			m_shadowSize = getThemeValue<i32>(theme, "shadow.size", m_shadowSize);
			m_shadowColor = getThemeValue<Color>(theme, "shadow.color", m_shadowColor);
			m_shadowOffset.x = getThemeValue<f32>(theme, "shadow.offsetX", m_shadowOffset.x);
			m_shadowOffset.y = getThemeValue<f32>(theme, "shadow.offsetY", m_shadowOffset.y);
			m_modalOverlayColor = getThemeValue<Color>(theme, "modalOverlayColor", m_modalOverlayColor);

			setContentOffset({ 0, m_titlebarHeight });
		}

		void SubWindow::beforeDraw(ogfx::BasicRenderer2D& gfx)
		{
			if (m_blurReady && m_blurEnabled)
				gfx.drawRenderTargetBlurred(m_blurSnippet, m_blurDest.getPosition(), m_blurRadius, m_blurDest.getSize(), m_blurTint, m_blurSrc);
			m_blurReady = false;
		}

		void SubWindow::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			draw_titlebar(gfx);
		}

		void SubWindow::onMousePressed(const Event& event)
		{
			if (event.mouse->button != MouseEventData::eButton::Left)
				return;
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			m_pressMouse = pos;
			m_pressBounds = getBounds();

			const ResizeEdges edges = resize_edges_at(pos);
			if (edges.any())
			{
				m_resizing = edges;
				return;
			}
			const eButtonId button = button_at(pos);
			if (button != eButtonId::None)
			{
				m_pressedButtonId = button;
				m_hoveredButton = button;
				return;
			}
			if (!titlebar_rect().contains(pos, true))
				return;
			const u64 now = now_ms();
			if (now - m_lastTitleClickMs <= DoubleClickMs)
			{
				m_lastTitleClickMs = 0;
				if (isMinimized())
					restore();
				else if (hasButton(Buttons::Maximize) && !m_modal)
					toggleMaximized();
				return;
			}
			m_lastTitleClickMs = now;
			if (m_draggable && m_state == eState::Normal)
				m_dragging = true;
		}

		void SubWindow::onMouseReleased(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			if (m_pressedButtonId != eButtonId::None)
			{
				const eButtonId pressed = m_pressedButtonId;
				m_pressedButtonId = eButtonId::None;
				if (button_at(pos) == pressed)
					activate_button(pressed);
			}
			m_dragging = false;
			m_resizing = { };
			if (isVisible() && contains(pos, true))
				update_cursor(pos);
			else
				set_cursor(cast<u8>(eCursor::Default));
		}

		void SubWindow::onMouseMoved(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			m_hoveredButton = button_at(pos);
			update_cursor(pos);
		}

		void SubWindow::onMouseDragged(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			const Vec2 delta = pos - m_pressMouse;
			if (m_dragging)
			{
				const Vec2 area = parent_area_size();
				const f32 keepVisible = 60.0f;
				f32 nx = m_pressBounds.x + delta.x;
				f32 ny = m_pressBounds.y + delta.y;
				nx = std::clamp(nx, keepVisible - getw(), std::max(keepVisible - getw(), area.x - keepVisible));
				ny = std::clamp(ny, 0.0f, std::max(0.0f, area.y - m_titlebarHeight));
				setPosition(std::round(nx), std::round(ny));
			}
			else if (m_resizing.any())
			{
				const Vec2 minSize = minimum_window_size();
				Rectangle r = m_pressBounds;
				if (m_resizing.left)
				{
					const f32 nx = std::min(r.x + delta.x, r.x + r.w - minSize.x);
					r.w = r.x + r.w - nx;
					r.x = nx;
				}
				if (m_resizing.right)
					r.w = std::max(minSize.x, r.w + delta.x);
				if (m_resizing.top)
				{
					const f32 ny = std::max(0.0f, std::min(r.y + delta.y, r.y + r.h - minSize.y));
					r.h = r.y + r.h - ny;
					r.y = ny;
				}
				if (m_resizing.bottom)
					r.h = std::max(minSize.y, r.h + delta.y);
				setBounds(std::round(r.x), std::round(r.y), std::round(r.w), std::round(r.h));
			}
			else if (m_pressedButtonId != eButtonId::None)
				m_hoveredButton = button_at(pos);
		}

		void SubWindow::onMouseExited(const Event& event)
		{
			m_hoveredButton = eButtonId::None;
			if (!m_dragging && !m_resizing.any())
				set_cursor(cast<u8>(eCursor::Default));
		}

		bool SubWindow::__needsBackdrop(void) const
		{
			return m_blurEnabled && m_blurRadius > 0.0f && isVisible() && !isInvalid();
		}

		void SubWindow::__captureBackdrop(ogfx::BasicRenderer2D& gfx, const RenderTarget& backdrop)
		{
			m_blurReady = false;
			const Rectangle screen { 0, 0, backdrop.getSize().x, backdrop.getSize().y };
			const Rectangle bounds { getGlobalPosition(), getSize() };
			if (!screen.intersects(bounds))
				return;
			// Only the visible part of the window is blurred; the copied region adds the
			// blur reach around it so the edges blur with what's really next to them.
			const Rectangle visible = screen.getIntersection(bounds);
			const f32 reach = std::ceil(m_blurRadius);
			Rectangle padded { visible.x - reach, visible.y - reach, visible.w + reach * 2, visible.h + reach * 2 };
			padded = screen.getIntersection(padded);
			if (padded.w < 1 || padded.h < 1)
				return;

			// Rounded up so a window being resized doesn't re-create the target every frame.
			const f32 step = 32.0f;
			const Vec2 snippetSize { std::ceil(padded.w / step) * step, std::ceil(padded.h / step) * step };
			if (!m_blurSnippet.isCreated())
				m_blurSnippet.create(gfx, snippetSize);
			else
				m_blurSnippet.resize(snippetSize);
			if (!gfx.beginRenderTarget(m_blurSnippet, true, Colors::Transparent))
				return;
			gfx.drawRenderTarget(backdrop, { 0, 0 }, padded.getSize(), Colors::White, padded);
			gfx.endRenderTarget();

			m_blurDest = visible;
			m_blurSrc = { visible.x - padded.x, visible.y - padded.y, visible.w, visible.h };
			m_blurReady = true;
		}

		void SubWindow::__drawShadow(ogfx::BasicRenderer2D& gfx)
		{
			if (!m_shadowEnabled || m_shadowSize <= 0 || m_shadowColor.a == 0)
				return;
			// Nested translucent rects: the alpha adds up towards the window, so the
			// shadow fades out linearly over shadowSize pixels.
			Color layer = m_shadowColor;
			layer.a = cast<u8>(std::max(1, cast<i32>(m_shadowColor.a) / m_shadowSize));
			const Rectangle bounds { getGlobalPosition() + m_shadowOffset, getSize() };
			const f32 radius = cast<f32>(getBorderRadius());
			for (i32 i = m_shadowSize; i >= 1; i--)
			{
				const f32 e = cast<f32>(i);
				gfx.fillRoundRect({ bounds.x - e, bounds.y - e, bounds.w + e * 2, bounds.h + e * 2 }, layer, radius + e);
			}
		}

		void SubWindow::__setMinimizedSlot(i32 slot)
		{
			const f32 ww = m_minimizedWidth;
			const f32 hh = m_titlebarHeight;
			const f32 xx = MinimizedSpacing + cast<f32>(slot) * (ww + MinimizedSpacing);
			setAnchors(Anchors { 0, 1, 0, 1 }, true);
			setOffsets(Offsets { xx, -hh - MinimizedSpacing, xx + ww, -MinimizedSpacing });
		}

		void SubWindow::__clampToParent(void)
		{
			if (m_state != eState::Normal || getParent() == nullptr)
				return;
			const Vec2 area = parent_area_size();
			const f32 keepVisible = 60.0f;
			const f32 nx = std::clamp(getx(), keepVisible - getw(), std::max(keepVisible - getw(), area.x - keepVisible));
			const f32 ny = std::clamp(gety(), 0.0f, std::max(0.0f, area.y - m_titlebarHeight));
			if (nx != getx() || ny != gety())
				setPosition(nx, ny);
		}

		Rectangle SubWindow::titlebar_rect(void) const
		{
			return { getGlobalPosition(), { getw(), std::min(m_titlebarHeight, geth()) } };
		}

		Rectangle SubWindow::button_rect(eButtonId id) const
		{
			static constexpr u8 flags[] = { Buttons::Close, Buttons::Maximize, Buttons::Minimize, Buttons::AlwaysOnTop };
			const Rectangle tb = titlebar_rect();
			const f32 size = std::max(0.0f, std::min(m_buttonSize, tb.h - 4.0f));
			const f32 inset = std::floor((tb.h - size) * 0.5f);
			f32 right = tb.x + tb.w - inset;
			for (i32 i = 0; i < cast<i32>(eButtonId::Count); i++)
			{
				// Modal windows only offer the close button.
				if (!hasButton(flags[i]) || (m_modal && i != cast<i32>(eButtonId::Close)))
					continue;
				right -= size;
				if (i == cast<i32>(id))
					return { right, tb.y + inset, size, size };
				right -= m_buttonSpacing;
			}
			return { 0, 0, 0, 0 };
		}

		SubWindow::eButtonId SubWindow::button_at(const Vec2& pos) const
		{
			for (i32 i = 0; i < cast<i32>(eButtonId::Count); i++)
			{
				const Rectangle r = button_rect(cast<eButtonId>(i));
				if (r.w > 0 && r.contains(pos, true))
					return cast<eButtonId>(i);
			}
			return eButtonId::None;
		}

		SubWindow::ResizeEdges SubWindow::resize_edges_at(const Vec2& pos) const
		{
			ResizeEdges edges;
			if (!m_resizable || m_state != eState::Normal || m_resizeGripSize <= 0)
				return edges;
			const Rectangle b { getGlobalPosition(), getSize() };
			if (!b.contains(pos, true))
				return edges;
			// Corners get a larger grip than the edges, they're the common target.
			const f32 g = m_resizeGripSize;
			const f32 c = g * 2.0f;
			const bool nearLeft = pos.x < b.x + c, nearRight = pos.x > b.x + b.w - c;
			const bool nearTop = pos.y < b.y + c, nearBottom = pos.y > b.y + b.h - c;
			if ((nearLeft || nearRight) && (nearTop || nearBottom))
			{
				edges.left = nearLeft;
				edges.right = !nearLeft && nearRight;
				edges.top = nearTop;
				edges.bottom = !nearTop && nearBottom;
				return edges;
			}
			edges.left = pos.x < b.x + g;
			edges.right = pos.x > b.x + b.w - g;
			edges.top = pos.y < b.y + g;
			edges.bottom = pos.y > b.y + b.h - g;
			return edges;
		}

		void SubWindow::update_cursor(const Vec2& pos)
		{
			if (m_dragging || m_resizing.any())
				return;
			const ResizeEdges e = resize_edges_at(pos);
			eCursor cursor = eCursor::Default;
			if ((e.left && e.top) || (e.right && e.bottom))
				cursor = eCursor::NWSE_Resize;
			else if ((e.right && e.top) || (e.left && e.bottom))
				cursor = eCursor::NESW_Resize;
			else if (e.left || e.right)
				cursor = eCursor::EW_Resize;
			else if (e.top || e.bottom)
				cursor = eCursor::NS_Resize;
			set_cursor(cast<u8>(cursor));
		}

		void SubWindow::set_cursor(u8 cursor)
		{
			const bool isDefault = (cursor == cast<u8>(eCursor::Default));
			// Never reset a cursor some other widget set.
			if (isDefault && !m_cursorSet)
				return;
			if (m_cursorSet && m_currentCursor == cursor)
				return;
			getWindow().setCursor(cast<eCursor>(cursor));
			m_currentCursor = cursor;
			m_cursorSet = !isDefault;
		}

		void SubWindow::draw_titlebar(ogfx::BasicRenderer2D& gfx)
		{
			const Rectangle tb = titlebar_rect();
			const f32 radius = cast<f32>(getBorderRadius());
			const bool minimized = isMinimized();
			gfx.fillRoundRect(tb, m_titlebarColor, minimized ? Rectangle { radius, radius, radius, radius } : Rectangle { radius, radius, 0, 0 });
			if (!minimized && m_titlebarBorderWidth > 0)
				gfx.drawLine({ { tb.x, tb.y + tb.h }, { tb.x + tb.w, tb.y + tb.h } }, m_titlebarBorderColor, m_titlebarBorderWidth, false);

			// The title gets whatever space the buttons leave.
			f32 textRight = tb.x + tb.w;
			for (i32 i = 0; i < cast<i32>(eButtonId::Count); i++)
			{
				const Rectangle r = button_rect(cast<eButtonId>(i));
				if (r.w > 0)
					textRight = std::min(textRight, r.x);
			}
			const Rectangle textArea { tb.x + 10, tb.y, textRight - tb.x - 14, tb.h };
			if (textArea.w > 0)
			{
				gfx.pushClippingRect(textArea, true);
				if (m_titleTextAlign == cast<i32>(Window::eTextAlign::Center))
					gfx.drawCenteredString(m_title, textArea, m_titleColor, m_titlebarFontSize);
				else
					gfx.drawVCenteredString(m_title, textArea, m_titleColor, m_titlebarFontSize);
				gfx.popClippingRect();
			}

			for (i32 i = 0; i < cast<i32>(eButtonId::Count); i++)
				draw_button(gfx, cast<eButtonId>(i));
		}

		void SubWindow::draw_button(ogfx::BasicRenderer2D& gfx, eButtonId id)
		{
			const Rectangle b = button_rect(id);
			if (b.w <= 0)
				return;
			const bool hovered = (m_hoveredButton == id);
			const bool pressed = hovered && (m_pressedButtonId == id);
			const bool isClose = (id == eButtonId::Close);
			Color bg = m_buttonColor;
			if (id == eButtonId::AlwaysOnTop && m_alwaysOnTop)
				bg = m_buttonActiveColor;
			if (pressed)
				bg = isClose ? m_closePressedColor : m_buttonPressedColor;
			else if (hovered)
				bg = isClose ? m_closeHoverColor : m_buttonHoverColor;
			if (bg.a > 0)
				gfx.fillRoundRect(b, bg, m_buttonBorderRadius);

			const Color glyph = hovered ? m_buttonGlyphHoverColor : m_buttonGlyphColor;
			const i32 t = std::max(1, cast<i32>(m_buttonGlyphThickness));
			const f32 inset = std::round(b.w * 0.3f);
			const Rectangle in { b.x + inset, b.y + inset, b.w - inset * 2, b.h - inset * 2 };
			const f32 cx = in.x + in.w * 0.5f;
			switch (id)
			{
				case eButtonId::Close:
					gfx.drawLine({ { in.x, in.y }, { in.x + in.w, in.y + in.h } }, glyph, t);
					gfx.drawLine({ { in.x + in.w, in.y }, { in.x, in.y + in.h } }, glyph, t);
					break;
				case eButtonId::Maximize:
					if (isMaximized())
					{
						// "Restore": two overlapping frames.
						const f32 o = std::max(2.0f, std::round(in.w * 0.25f));
						gfx.drawRect({ in.x + o, in.y, in.w - o, in.h - o }, glyph, t, true, true, false, false);
						gfx.drawRect({ in.x, in.y + o, in.w - o, in.h - o }, glyph, t);
					}
					else
						gfx.drawRect(in, glyph, t);
					break;
				case eButtonId::Minimize:
					if (isMinimized())
					{
						// "Restore": a caret pointing up.
						gfx.drawLine({ { in.x, in.y + in.h * 0.7f }, { cx, in.y + in.h * 0.3f } }, glyph, t);
						gfx.drawLine({ { cx, in.y + in.h * 0.3f }, { in.x + in.w, in.y + in.h * 0.7f } }, glyph, t);
					}
					else
						gfx.drawLine({ { in.x, in.y + in.h - t * 0.5f }, { in.x + in.w, in.y + in.h - t * 0.5f } }, glyph, t);
					break;
				case eButtonId::AlwaysOnTop:
				{
					// A pin: head and needle.
					const f32 r = in.w * 0.3f;
					const Vec2 head { cx, in.y + r };
					if (m_alwaysOnTop)
						gfx.fillCircle(head, r, glyph);
					else
						gfx.drawCircle(head, r, glyph, t);
					gfx.drawLine({ { cx, head.y + r }, { cx, in.y + in.h } }, glyph, t);
					break;
				}
				default: break;
			}
		}

		void SubWindow::activate_button(eButtonId id)
		{
			switch (id)
			{
				case eButtonId::Close: close(); break;
				case eButtonId::Maximize: if (isMinimized()) maximize(); else toggleMaximized(); break;
				case eButtonId::Minimize: if (isMinimized()) restore(); else minimize(); break;
				case eButtonId::AlwaysOnTop: setAlwaysOnTop(!m_alwaysOnTop); break;
				default: break;
			}
		}

		void SubWindow::save_layout(void)
		{
			m_savedAnchors = getAnchors();
			m_savedOffsets = getOffsets();
			m_hasSavedLayout = true;
		}

		void SubWindow::restore_layout(void)
		{
			if (!m_hasSavedLayout)
				return;
			setAnchors(m_savedAnchors, true);
			setOffsets(m_savedOffsets);
		}

		void SubWindow::set_state(eState state)
		{
			if (m_state == state)
				return;
			m_state = state;
			suspendChildren(state == eState::Minimized);
			getWindow().__onSubWindowStateChanged(*this);
			if (m_stateChangedCallback)
				m_stateChangedCallback(*this, state);
		}

		void SubWindow::layer_changed(void)
		{
			if (getParent() == nullptr)
				return;
			getParent()->getChildren().sortByLayer();
			bringToFront();
		}

		Vec2 SubWindow::parent_area_size(void) const
		{
			if (getParent() == nullptr)
				return { 0, 0 };
			return getParent()->getContentBounds().getSize();
		}

		Vec2 SubWindow::minimum_window_size(void) const
		{
			// Room for the buttons plus a bit of title, and for the grips.
			f32 buttonsWidth = 0;
			const Rectangle tb = titlebar_rect();
			for (i32 i = 0; i < cast<i32>(eButtonId::Count); i++)
			{
				const Rectangle r = button_rect(cast<eButtonId>(i));
				if (r.w > 0)
					buttonsWidth = std::max(buttonsWidth, tb.x + tb.w - r.x);
			}
			const Vec2 combined = getCombinedMinSize();
			return {
				std::max({ m_minimumSize.x, combined.x, buttonsWidth + 60.0f }),
				std::max({ m_minimumSize.y, combined.y, m_titlebarHeight + m_resizeGripSize * 2.0f })
			};
		}



		// ===================================================================
		// MessageDialog
		// ===================================================================
		MessageDialog::MessageDialog(Window& window, const String& title, const String& text) : SubWindow(window, title), m_okButton(window, "OK")
		{
			m_text = text;
			build();
		}

		MessageDialog::MessageDialog(Window& window, const String& title, const String& text, const Icon& icon) : SubWindow(window, title), m_okButton(window, "OK")
		{
			m_text = text;
			m_icon = icon.asAnim();
			m_hasIcon = true;
			build();
		}

		void MessageDialog::build(void)
		{
			setTypeName("ogfx::gui::MessageDialog");
			addThemeID("messagebox");
			setButtons(Buttons::Close);
			disableResizable();
			setModal();
			setText(m_text);
			m_okButton.setSizeMode(eSizeMode::Manual);
			m_okButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { close(); });
			addWidget(m_okButton);
		}

		void MessageDialog::setText(const String& text)
		{
			m_text = text;
			m_lines.clear();
			const ostd::cpp_string& data = m_text.cpp_str();
			size_t start = 0;
			while (true)
			{
				const size_t nl = data.find('\n', start);
				m_lines.push_back(data.substr(start, nl == ostd::cpp_string::npos ? ostd::cpp_string::npos : nl - start));
				if (nl == ostd::cpp_string::npos)
					break;
				start = nl + 1;
			}
			if (getParent() != nullptr)
				fitToContent();
		}

		void MessageDialog::applyTheme(const ostd::Stylesheet& theme)
		{
			SubWindow::applyTheme(theme);
			m_iconSize = getThemeValue<f32>(theme, "messageBox.iconSize", m_iconSize);
			m_spacing = getThemeValue<f32>(theme, "messageBox.spacing", m_spacing);
			m_lineSpacing = getThemeValue<f32>(theme, "messageBox.lineSpacing", m_lineSpacing);
			m_minWidth = getThemeValue<f32>(theme, "messageBox.minWidth", m_minWidth);
			if (getParent() != nullptr)
				fitToContent();
		}

		void MessageDialog::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			SubWindow::onDraw(gfx);
			if (isMinimized())
				return;
			const Rectangle content = getGlobalContentBounds();
			const Vec2 textSize = text_size();
			f32 textX = content.x;
			if (m_hasIcon)
			{
				gfx.drawAnimation(m_icon, { content.x, content.y + std::max(0.0f, std::round((textSize.y - m_iconSize) * 0.5f)) }, { m_iconSize, m_iconSize });
				textX += m_iconSize + m_spacing;
			}
			f32 y = content.y + (m_hasIcon ? std::max(0.0f, std::round((m_iconSize - textSize.y) * 0.5f)) : 0.0f);
			for (const auto& line : m_lines)
			{
				gfx.drawString(line, { textX, y }, getTextColor(), getFontSize());
				y += gfx.getStringDimensions(line.new_trim() == "" ? String(" ") : line, getFontSize()).y + m_lineSpacing;
			}
		}

		void MessageDialog::onUpdate(void)
		{
			if (m_hasIcon)
				m_icon.update();
		}

		void MessageDialog::fitToContent(void)
		{
			const Vec2 textSize = text_size();
			Vec2 buttonSize = m_okButton.getContentMinSize();
			buttonSize.x = std::max(buttonSize.x, 90.0f);
			const f32 iconWidth = m_hasIcon ? m_iconSize + m_spacing : 0.0f;
			const f32 contentW = std::max(iconWidth + textSize.x, buttonSize.x);
			const f32 contentH = std::max(m_hasIcon ? m_iconSize : 0.0f, textSize.y) + m_spacing + buttonSize.y;
			Vec2 size = contentToWidgetSize({ contentW, contentH });
			size.x = std::max(size.x, m_minWidth);
			setSize(std::ceil(size.x), std::ceil(size.y));
			m_okButton.setSize(buttonSize);
			m_okButton.setAnchorPreset(eAnchorPreset::BottomRight);
		}

		Vec2 MessageDialog::text_size(void) const
		{
			auto& gfx = getWindow().getGFX();
			Vec2 size { 0, 0 };
			for (const auto& line : m_lines)
			{
				const Vec2 dims = gfx.getStringDimensions(line.new_trim() == "" ? String(" ") : line, getFontSize());
				size.x = std::max(size.x, dims.x);
				size.y += dims.y;
			}
			if (m_lines.size() > 1)
				size.y += m_lineSpacing * cast<f32>(m_lines.size() - 1);
			return size;
		}
	}
}
