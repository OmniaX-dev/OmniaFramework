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

#include "Text.hpp"
#include "../../utils/Keycodes.hpp"
#include "../../render/BasicRenderer.hpp"

namespace ogfx
{
	namespace gui
	{
		TextEdit& TextEdit::create(void)
		{
			setPadding({ 0, 0, 0, 0 });
			setTypeName("ogfx::gui::TextEdit");
			disableChildren();
			setStylesheetCategoryName("textEdit");
			enableFocus();
			enableBackground();
			enableBorder();
			enableTextEnteredEvent(true);
			setSize({ 200, 30 });
			m_cursorBlinkTimer.start();
			m_buffer.setOnChanged([this](const ostd::TextBuffer&) {
				m_layout.dirty = true;
			});
			m_buffer.setOnCursorMoved([this](const ostd::TextBuffer&) {
				if (m_cursorBlink)
					m_cursorBlinkTimer.start();
				m_cursorState = true;
			});
			validate();
			return *this;
		}

		void TextEdit::applyTheme(const ostd::Stylesheet& theme)
		{

		}

		void TextEdit::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			const auto bounds = getGlobalBounds();

			// -------------------------------------------------------------
			// 1. Rebuild the layout cache if the buffer (or font/size) has
			//    changed since last frame. We mark dirty in the buffer's
			//    onChanged callback (set up in create()).
			// -------------------------------------------------------------
			if (m_layout.dirty)
				rebuild_layout(gfx);

			// -------------------------------------------------------------
			// 2. Make sure the cursor is in view. This adjusts m_scrollX as
			//    needed so the cursor sits inside the widget's content area.
			// -------------------------------------------------------------
			ensure_cursor_visible(bounds);

			// -------------------------------------------------------------
			// 3. Compute the text content area inside the widget — the
			//    rectangle where text is allowed to render. Anything drawn
			//    outside this rect gets clipped away.
			// -------------------------------------------------------------
			const Rectangle contentArea {
				bounds.x + m_textPadding.left,
				bounds.y,
				bounds.w - m_textPadding.left - m_textPadding.right,
				bounds.h
			};

			// -------------------------------------------------------------
			// 5. Compute the y position for the text baseline area. Vertically
			//    centered using the cached line height. This is single-line
			//    behavior; for multiline you'd loop over lines and offset y
			//    by lineHeight per row.
			// -------------------------------------------------------------
			const f32 textY = bounds.y + (bounds.h - m_layout.lineHeight) * 0.5f;
			const f32 textX = contentArea.x - m_scrollX;

			// -------------------------------------------------------------
			// 6. Draw the selection highlight FIRST (under the text). This
			//    way the text stays the regular color and just sits on top
			//    of a colored band — like every text editor.
			// -------------------------------------------------------------
			if (m_buffer.hasSelection())
			{
				const u32 selStart = m_buffer.selectionStart();
				const u32 selEnd   = m_buffer.selectionEnd();
				const f32 x1 = textX + layout_x_for_byte(selStart);
				const f32 x2 = textX + layout_x_for_byte(selEnd);
				gfx.fillRect(
					{ x1, textY, x2 - x1, m_layout.lineHeight },
					m_selectionColor
				);
			}

			// -------------------------------------------------------------
			// 7. Draw the text itself. Single drawString call — the layout
			//    cache is only used for cursor / selection positioning;
			//    actual glyph rendering goes through the renderer's existing
			//    code path (which handles kerning, atlas, etc.).
			// -------------------------------------------------------------
			if (!m_buffer.empty())
				gfx.drawString(m_buffer.text(), { textX, textY }, getTextColor(), getFontSize());

			// -------------------------------------------------------------
			// 8. Draw the cursor. Only when focused and the blink is in the
			//    "on" phase. The cursor renders ON TOP of everything else so
			//    it's visible inside selections.
			// -------------------------------------------------------------
			if (isFocused() && (m_cursorState || !m_cursorBlink))
			{
				const f32 cx = textX + layout_x_for_byte(m_buffer.cursorByteOffset());
				gfx.fillRect(
					{ cx, textY, m_cursorWidth, m_layout.lineHeight },
					m_cursorColor
				);
			}
		}

		void TextEdit::onUpdate(void)
		{
			m_keyRepeatTimer.update();
			m_doubleClickTimer.update();
			if (m_cursorBlink)
			{
				m_cursorBlinkTimer.update();
				if (m_cursorBlinkTimer.isDone())
				{
					m_cursorState = !m_cursorState;
					m_cursorBlinkTimer.start();
				}
			}

			// Auto-scroll while drag-selecting past the widget edges.
			if (m_mouseSelecting)
			{
				const auto bounds = getGlobalBounds();
				const f32 contentLeft  = m_textPadding.left;
				const f32 contentRight = bounds.w - m_textPadding.right;

				// Only act when mouse is outside the content area horizontally.
				if (m_lastMouseLocalX < contentLeft || m_lastMouseLocalX > contentRight)
				{
					// Use the same hit-test as drag, but it'll naturally clamp to
					// start/end of buffer when the mouse is way outside. The
					// setCursorByteOffset call will trigger ensure_cursor_visible
					// on the next draw, which scrolls the view to follow.
					const u32 hit = byte_offset_for_pixel_x(m_lastMouseLocalX);
					m_buffer.setCursorByteOffset(hit, true);
				}
			}
		}

		void TextEdit::onFocusGained(const Event& event)
		{

		}

		void TextEdit::onFocusLost(const Event& event)
		{

		}

		void TextEdit::onTextEntered(const Event& event)
		{
			auto& data = *event.keyboard;
			if (!m_charFilter || m_charFilter->isValidChar(data.text))
			{
				if (m_lastChar == data.text && m_keyRepeatTimer.isCounting())
					return;
				m_buffer.insertText(data.text);
				m_lastChar = data.text;
				m_keyRepeatTimer.start();
				if (m_cursorBlink)
					m_cursorBlinkTimer.start();
				m_cursorState = true;
			}
		}

		void TextEdit::onKeyPressed(const Event& event)
		{
			auto& data = *event.keyboard;
			if (m_lastKeyCode == data.keyCode && m_keyRepeatTimer.isCounting())
				return;
			if (data.keyCode == KeyCode::Backspace)
			{
				m_keyRepeatTimer.start();
				m_lastKeyCode = data.keyCode;
				m_buffer.backspace();
			}
			else if (data.keyCode == KeyCode::Left)
			{
				m_keyRepeatTimer.start();
				m_lastKeyCode = data.keyCode;
				m_buffer.moveLeft();
			}
			else if (data.keyCode == KeyCode::Right)
			{
				m_keyRepeatTimer.start();

				m_lastKeyCode = data.keyCode;
				m_buffer.moveRight();
			}
			else if (data.keyCode == KeyCode::Return)
			{
				m_keyRepeatTimer.start();
				m_lastKeyCode = data.keyCode;
				if (callback_onActionPerformed)
					callback_onActionPerformed(event);
			}
		}

		void TextEdit::onKeyReleased(const Event& event)
		{

		}

		void TextEdit::onMousePressed(const Event& event)
		{
			auto& data = *event.mouse;
			const f32 local_x = data.position_x - getGlobalBounds().x;
			const u32 hit = byte_offset_for_pixel_x(local_x);

			// Double-click detection: a second press within the timer window AND
			// close enough to the last press position counts as a double-click.
			// The position check matters — without it, two presses far apart in
			// a long document would falsely trigger word selection.
			constexpr f32 doubleClickRadius = 5.0f;
			const bool isDoubleClick =
				m_lastClickValid
				&& m_doubleClickTimer.isCounting()
				&& std::abs(local_x - m_lastClickLocalX) < doubleClickRadius;

			if (isDoubleClick)
			{
				// Word selection. selectWordAt sets both anchor and cursor;
				// we mark the drag flag so subsequent drag motion extends the
				// selection by word — though we'll keep the simple implementation
				// for now where dragging after a double-click extends by character.
				m_buffer.selectWordAt(hit);
				m_mouseSelecting = true;
				m_lastClickValid = false;  // Don't triple-click into something weird.
			}
			else
			{
				m_buffer.setCursorByteOffset(hit);
				m_mouseSelecting = true;
				m_lastClickLocalX = local_x;
				m_lastMouseLocalX = local_x;
				m_lastClickValid = true;
				m_doubleClickTimer.start();
			}

			if (m_cursorBlink) m_cursorBlinkTimer.start();
			m_cursorState = true;
		}

		void TextEdit::onMouseReleased(const Event& event)
		{
			m_mouseSelecting = false;
		}

		void TextEdit::onMouseDragged(const Event& event)
		{
			if (!m_mouseSelecting) return;
			auto& data = *event.mouse;
			const f32 local_x = data.position_x - getGlobalBounds().x;
			const u32 hit = byte_offset_for_pixel_x(local_x);

			// Extend selection to the dragged-to position. The anchor was set by
			// setCursorByteOffset(hit) in onMousePressed and stays put.
			m_buffer.setCursorByteOffset(hit, true);
			m_lastMouseLocalX = local_x;

			if (m_cursorBlink) m_cursorBlinkTimer.start();
			m_cursorState = true;
		}

		void TextEdit::rebuild_layout(BasicRenderer2D& gfx)
		{
			m_layout.byteOffsets.clear();
			m_layout.xPositions.clear();
			m_layout.totalWidth = 0.0f;

			// Empty buffer: a single entry for "x=0 at byte 0" so the cursor
			// still has a position to render at.
			if (m_buffer.empty())
			{
				m_layout.lineHeight = gfx.getStringDimensions(" ", getFontSize()).y;
				m_layout.byteOffsets.push_back(0);
				m_layout.xPositions.push_back(0.0f);
				m_layout.dirty = false;
				return;
			}

			// One call to the renderer. Returns one Vec2 per codepoint:
			//   .x = advance (incl. kerning) contributed by that glyph
			//   .y = glyph height (same for all glyphs at this font/size)
			const stdvec<Vec2> perChar =
				gfx.getStringDimensionsPerCharacter(m_buffer.text(), getFontSize());

			if (perChar.empty())
			{
				// Defensive: shouldn't happen with non-empty text, but if the
				// font failed to load we still want a valid layout.
				m_layout.lineHeight = 0.0f;
				m_layout.byteOffsets.push_back(0);
				m_layout.xPositions.push_back(0.0f);
				m_layout.dirty = false;
				return;
			}

			m_layout.lineHeight = perChar[0].y;

			// Walk both the per-character advances AND the buffer's UTF-8 bytes
			// in lockstep. Since processString emits exactly one glyph per
			// codepoint, the i-th advance corresponds to the codepoint that
			// starts at the i-th byte boundary.
			const ostd::cpp_string& s = m_buffer.text().cpp_str();
			f32 x = 0.0f;
			u32 byte_pos = 0;
			size_t glyph_idx = 0;

			// First entry: position before any glyph.
			m_layout.byteOffsets.push_back(0);
			m_layout.xPositions.push_back(0.0f);

			while (byte_pos < s.size() && glyph_idx < perChar.size())
			{
				x += perChar[glyph_idx].x;
				byte_pos = String::utf8::next_codepoint_start(s, byte_pos);
				m_layout.byteOffsets.push_back(byte_pos);
				m_layout.xPositions.push_back(x);
				glyph_idx++;
			}

			m_layout.totalWidth = x;
			m_layout.dirty = false;
		}

		f32 TextEdit::layout_x_for_byte(u32 byte_offset) const
		{
			// Binary search for the entry. Layout invariant: byteOffsets is
			// strictly increasing, so std::lower_bound gives us the right index.
			const auto& v = m_layout.byteOffsets;
			if (v.empty()) return 0.0f;
			if (byte_offset >= v.back()) return m_layout.xPositions.back();

			auto it = std::lower_bound(v.begin(), v.end(), byte_offset);
			const size_t idx = std::distance(v.begin(), it);
			return m_layout.xPositions[idx];
		}

		void TextEdit::ensure_cursor_visible(const Rectangle& bounds)
		{
			const f32 contentW = bounds.w - m_textPadding.left - m_textPadding.right;
			if (contentW <= 0.0f) return;

			const f32 cursorX = layout_x_for_byte(m_buffer.cursorByteOffset());
			const f32 totalW  = m_layout.totalWidth;

			// Margin: how far the cursor should stay from the visible edges.
			// ~2 character widths feels right; using line height as a proxy works
			// well across font sizes since char width scales similarly.
			// This single-sourced value also caps itself at 25% of contentW so it
			// stays sensible even in very narrow widgets.
			const f32 marginRaw = m_layout.lineHeight * 0.6f;
			const f32 margin = std::min(marginRaw, contentW * 0.25f);

			// Case 1: all the text fits. No scroll needed.
			if (totalW <= contentW)
			{
				m_scrollX = 0.0f;
				return;
			}

			// Case 2: text overflows. Apply margin-aware scrolling.
			//
			// The visible window in document space is [m_scrollX, m_scrollX + contentW).
			// We want cursorX to satisfy:
			//     m_scrollX + margin <= cursorX <= m_scrollX + contentW - margin
			// — except clamped at the document edges.

			// If cursor is too close to the left edge of the visible window, scroll
			// left so there's `margin` of text visible to its left.
			if (cursorX < m_scrollX + margin)
				m_scrollX = cursorX - margin;

			// If cursor is too close to the right edge, scroll right so there's
			// `margin` of room to its right.
			else if (cursorX > m_scrollX + contentW - margin)
				m_scrollX = cursorX - contentW + margin;

			// Clamp scrollX to valid range. The lower bound is 0 (don't show empty
			// space to the left of the text). The upper bound is totalW - contentW
			// (don't show empty space to the right of the text — i.e., the right
			// edge of the text aligns with the right edge of the visible area).
			//
			// This upper clamp is what produces the "right-anchored when at end"
			// behavior automatically: when cursor is at end, the margin formula
			// wants to scroll past the end of the text, but we clamp it back so
			// the text's right edge sits at the visible area's right edge.
			if (m_scrollX < 0.0f)
				m_scrollX = 0.0f;
			if (m_scrollX > totalW - contentW)
				m_scrollX = totalW - contentW;
		}

		u32 TextEdit::byte_offset_for_pixel_x(f32 widget_local_x) const
		{
			const f32 doc_x = widget_local_x - m_textPadding.left + m_scrollX;

			const auto& xs = m_layout.xPositions;
			const auto& bs = m_layout.byteOffsets;
			if (xs.empty()) return 0;
			if (doc_x <= xs.front()) return bs.front();
			if (doc_x >= xs.back()) return bs.back();

			auto it = std::upper_bound(xs.begin(), xs.end(), doc_x);
			const size_t hi = std::distance(xs.begin(), it);
			const size_t lo = hi - 1;
			const f32 distLo = doc_x - xs[lo];
			const f32 distHi = xs[hi] - doc_x;
			return cast<u32>((distLo <= distHi) ? bs[lo] : bs[hi]);
		}
	}
}
