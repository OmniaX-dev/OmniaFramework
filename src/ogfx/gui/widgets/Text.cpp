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
		String TextEdit::CharacterFilter::predictResult(const String& utf8, const ostd::TextBuffer& buffer)
		{
			const String& current = buffer.text();
			const u32 selStart = buffer.selectionStart();
			const u32 selEnd   = buffer.selectionEnd();
			String head = current.new_substr(0, cast<i32>(selStart));
			String tail = current.new_substr(selEnd, cast<i32>(current.len()));
			return head + utf8 + tail;
		}





		bool TextEdit::IntegerFilter::isValidChar(const String& utf8, const ostd::TextBuffer& buffer)
		{
			// Reject anything that isn't a single ASCII byte. Multi-codepoint
			// input (IME, emoji, accented chars) is never valid in an integer.
			if (utf8.len() != 1) return false;
			const char c = utf8[0];
			const bool isDigit = (c >= '0' && c <= '9');
			const bool isSign  = (c == '-');
			if (!isDigit && !isSign) return false;

			// Predict the post-insert string and validate it as a prefix.
			const String result = predictResult(utf8, buffer);
			return isValidIntegerPrefix(result);
		}

		bool TextEdit::IntegerFilter::isValidIntegerPrefix(const String& s) const
		{
			if (s.empty()) return true;
			const ostd::cpp_string& bytes = s.cpp_str();

			u32 i = 0;
			// Optional leading minus.
			if (bytes[0] == '-') {
				if (!m_allowNegative) return false;
				i = 1;
				if (bytes.size() == 1) return true;  // Just "-" by itself: valid prefix.
			}

			// Everything from i onward must be digits. Count them while we're at it.
			u32 digitCount = 0;
			while (i < bytes.size()) {
				if (bytes[i] < '0' || bytes[i] > '9') return false;
				digitCount++;
				i++;
			}

			if (m_maxDigits > 0 && digitCount > cast<u32>(m_maxDigits))
				return false;

			return true;
		}





		bool TextEdit::HexadecimalFilter::isValidChar(const String& utf8, const ostd::TextBuffer& buffer)
		{
			// Reject anything that isn't a single ASCII byte. Multi-codepoint
			// input (IME, emoji, accented chars) is never valid in an hex.
			if (utf8.len() != 1) return false;
			const char c = utf8[0];
			if (!isHexDigit(c)) return false;

			// Predict the post-insert string and validate it as a prefix.
			const String result = predictResult(utf8, buffer);
			return isValidHexPrefix(result);
		}

		bool TextEdit::HexadecimalFilter::isValidHexPrefix(const String& s) const
		{
			if (s.empty()) return true;
			const ostd::cpp_string& bytes = s.cpp_str();

			u32 i = 0;

			// Everything from i onward must be digits. Count them while we're at it.
			u32 digitCount = 0;
			while (i < bytes.size()) {
				if (!isHexDigit(bytes[i])) return false;
				digitCount++;
				i++;
			}

			if (m_maxDigits > 0 && digitCount > cast<u32>(m_maxDigits))
				return false;

			return true;
		}

		bool TextEdit::HexadecimalFilter::isHexDigit(char c) const
		{
			return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
		}





		bool TextEdit::DecimalFilter::isValidChar(const String& utf8, const ostd::TextBuffer& buffer)
		{
			if (utf8.len() != 1) return false;
			const char c = utf8[0];
			const bool isDigit = (c >= '0' && c <= '9');
			const bool isSign  = (c == '-');
			const bool isSep   = (c == m_separator);
			if (!isDigit && !isSign && !isSep) return false;

			const String result = predictResult(utf8, buffer);
			return isValidDecimalPrefix(result);
		}

		bool TextEdit::DecimalFilter::isValidDecimalPrefix(const String& s) const
		{
			if (s.empty()) return true;
			const ostd::cpp_string& bytes = s.cpp_str();

			u32 i = 0;
			if (bytes[0] == '-') {
				if (!m_allowNegative) return false;
				i = 1;
				if (bytes.size() == 1) return true;  // Just "-": valid prefix.
			}

			u32 intDigits = 0;
			u32 fracDigits = 0;
			bool seenSep = false;

			while (i < bytes.size()) {
				const char c = bytes[i];
				if (c == m_separator) {
					if (seenSep) return false;     // Two separators: invalid.
					if (intDigits == 0) return false;  // ".5" without a leading digit:
													   // we choose to require it for now.
					seenSep = true;
				}
				else if (c >= '0' && c <= '9') {
					if (seenSep) fracDigits++;
					else         intDigits++;
				}
				else {
					return false;
				}
				i++;
			}

			if (m_maxIntDigits  > 0 && intDigits  > cast<u32>(m_maxIntDigits))  return false;
			if (m_maxFracDigits > 0 && fracDigits > cast<u32>(m_maxFracDigits)) return false;

			return true;
		}





		bool TextEdit::DateFilter::isValidChar(const String& utf8, const ostd::TextBuffer& buffer)
		{
			if (utf8.len() != 1) return false;
			const String result = predictResult(utf8, buffer);
			return isValidDatePrefix(result);
		}

		void TextEdit::DateFilter::buildPositionMap(void)
		{
			m_slots.clear();
			// Local helper to push a run of digit slots.
			auto pushDigits = [&](eSlot s, u32 n) {
				for (u32 i = 0; i < n; i++) m_slots.push_back(s);
			};
			auto pushSep = [&]() { m_slots.push_back(eSlot::Sep); };

			switch (m_format) {
				case eFormat::DDMMYY:
					pushDigits(eSlot::Day, 2);   pushSep();
					pushDigits(eSlot::Month, 2); pushSep();
					pushDigits(eSlot::Year, 2);  break;
				case eFormat::DDMMYYYY:
					pushDigits(eSlot::Day, 2);   pushSep();
					pushDigits(eSlot::Month, 2); pushSep();
					pushDigits(eSlot::Year, 4);  break;
				case eFormat::MMDDYY:
					pushDigits(eSlot::Month, 2); pushSep();
					pushDigits(eSlot::Day, 2);   pushSep();
					pushDigits(eSlot::Year, 2);  break;
				case eFormat::MMDDYYYY:
					pushDigits(eSlot::Month, 2); pushSep();
					pushDigits(eSlot::Day, 2);   pushSep();
					pushDigits(eSlot::Year, 4);  break;
				case eFormat::YYYYMMDD:
					pushDigits(eSlot::Year, 4);  pushSep();
					pushDigits(eSlot::Month, 2); pushSep();
					pushDigits(eSlot::Day, 2);   break;
				case eFormat::YYMMDD:
					pushDigits(eSlot::Year, 2);  pushSep();
					pushDigits(eSlot::Month, 2); pushSep();
					pushDigits(eSlot::Day, 2);   break;
			}
		}

		bool TextEdit::DateFilter::isValidDatePrefix(const String& s) const
		{
			const ostd::cpp_string& bytes = s.cpp_str();
			if (bytes.size() > m_slots.size()) return false;

			// Track the running values so we can validate ranges as soon as
			// a field is fully entered.
			u32 dayVal = 0,   dayChars = 0;
			u32 monthVal = 0, monthChars = 0;
			u32 yearVal = 0,  yearChars = 0;

			for (u32 i = 0; i < bytes.size(); i++) {
				const char c = bytes[i];
				const eSlot expected = m_slots[i];

				if (expected == eSlot::Sep) {
					if (c != m_separator) return false;
					// Separator means a field just ended — validate it.
					// (Only the field that *just ended* needs validation; earlier
					// fields were already validated when their separators were typed.)
				}
				else {
					if (c < '0' || c > '9') return false;
					const u32 d = cast<u32>(c - '0');
					if (expected == eSlot::Day)   { dayVal   = dayVal   * 10 + d; dayChars++;   }
					if (expected == eSlot::Month) { monthVal = monthVal * 10 + d; monthChars++; }
					if (expected == eSlot::Year)  { yearVal  = yearVal  * 10 + d; yearChars++;  }
				}

				// Range validation as soon as a field is fully populated.
				if (expected == eSlot::Day && dayChars == 2) {
					if (dayVal < 1 || dayVal > 31) return false;
				}
				if (expected == eSlot::Month && monthChars == 2) {
					if (monthVal < 1 || monthVal > 12) return false;
				}
				// No range check for year — historically valid years span millennia.

				// Also reject impossible *partial* values: a 2-digit day field
				// can't start with '4'-'9' if the second digit would always make
				// it exceed 31. Day field: first digit must be 0-3.
				// Month field: first digit must be 0-1.
				if (expected == eSlot::Day && dayChars == 1) {
					if (dayVal > 3) return false;
				}
				if (expected == eSlot::Month && monthChars == 1) {
					if (monthVal > 1) return false;
				}
			}

			return true;
		}





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
			enableCursorBlink(getThemeValue<bool>(theme, "cursorBlink", isCursorBlinkEnabled()));
			setCursorWidth(getThemeValue<f32>(theme, "cursorWidth", getCursorWidth()));
			setCursorColor(getThemeValue<Color>(theme, "cursorColor", getCursorColor()));
			setSelectionColor(getThemeValue<Color>(theme, "selectionColor", getSelectionColor()));
			setMaxLength(getThemeValue<i32>(theme, "macLength", getMaxLength()));
			setTextPadding(getThemeValue<f32>(theme, "textPaddingLeft", getTextPadding().left),
							  getThemeValue<f32>(theme, "textPaddingRight", getTextPadding().right));
			String mask = getThemeValue<String>(theme, "characterMask", String(""));
			if (mask.len() != 1)
				clearCharacterMask();
			else
				setCharacterMask(mask[0]);
			String styleStr = getThemeValue<String>(theme, "cursorStyle", String("line"));
			if      (styleStr == "block")      setCursorStyle(CursorStyle::Block);
			else if (styleStr == "box")        setCursorStyle(CursorStyle::Box);
			else if (styleStr == "underscore") setCursorStyle(CursorStyle::Underscore);
			else                               setCursorStyle(CursorStyle::Line);
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
			// 8. Draw the cursor. Only when focused and the blink is in the
			//    "on" phase. The cursor renders ON TOP of everything else so
			//    it's visible inside selections.
			// -------------------------------------------------------------
			if (isFocused() && (m_cursorState || !m_cursorBlink))
			{
				const f32 cx = textX + layout_x_for_byte(m_buffer.cursorByteOffset());
				auto l_char_width_at_cursor = [this](void) -> f32 {
					const u32 byteOff = m_buffer.cursorByteOffset();
					const auto& bs = m_layout.byteOffsets;
					const auto& xs = m_layout.xPositions;
					auto it = std::lower_bound(bs.begin(), bs.end(), byteOff);
					if (it != bs.end() && (it + 1) != bs.end()) {
						const size_t idx = std::distance(bs.begin(), it);
						return xs[idx + 1] - xs[idx];
					}
					return m_layout.lineHeight * 0.6f; // fallback at end-of-text
				};
				switch (m_cursorStyle)
				{
					case CursorStyle::Line:
						gfx.fillRect({ cx, textY, m_cursorWidth, m_layout.lineHeight }, m_cursorColor);
						break;
					case CursorStyle::Block:
					{
						gfx.fillRect({ cx, textY, l_char_width_at_cursor(), m_layout.lineHeight }, m_cursorColor);
						break;
					}
					case CursorStyle::Box:
					{
						gfx.drawRect({ cx, textY, l_char_width_at_cursor(), m_layout.lineHeight }, m_cursorColor, m_cursorWidth);
						break;
					}
					case CursorStyle::Underscore:
					{
						const f32 underH = std::max(2.0f, m_layout.lineHeight * 0.1f);
						gfx.fillRect({ cx, textY + m_layout.lineHeight - underH, l_char_width_at_cursor(), m_cursorWidth }, m_cursorColor);
						break;
					}
				}
			}

			// -------------------------------------------------------------
			// 7. Draw the text itself. Single drawString call — the layout
			//    cache is only used for cursor / selection positioning;
			//    actual glyph rendering goes through the renderer's existing
			//    code path (which handles kerning, atlas, etc.).
			// -------------------------------------------------------------
			if (!m_buffer.empty())
			{
				if (hasCharacterMask())
					gfx.drawString(String::duplicateChar(m_charMask, m_buffer.text().len()), { textX, textY }, getTextColor(), getFontSize());
				else
					gfx.drawString(m_buffer.text(), { textX, textY }, getTextColor(), getFontSize());
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

		void TextEdit::onFocusLost(const Event& event)
		{
			if (m_buffer.hasSelection())
				m_buffer.clearSelection();
		}

		void TextEdit::onTextEntered(const Event& event)
		{
			auto& data = *event.keyboard;
			const String toInsert = clamp_input_to_max_length(data.text);
			if (toInsert.empty()) return;
			if (!m_filter || m_filter->isValidChar(data.text, m_buffer))
			{
				if (m_lastChar == toInsert && m_keyRepeatTimer.isCounting())
					return;
				m_buffer.insertText(toInsert);
				m_lastChar = toInsert;
				m_keyRepeatTimer.start();
			}
		}

		void TextEdit::onKeyPressed(const Event& event)
		{
			auto& data = *event.keyboard;
			if (m_lastKeyCode == data.keyCode && m_keyRepeatTimer.isCounting())
				return;

			const bool extend = data.modifiers.anyShift();
			const bool word   = data.modifiers.primary();
			const bool mask   = hasCharacterMask();

			// ----- Editing operations (chords first, since they're more specific) -----

			if (data.isCopy())
			{
				if (hasCharacterMask() && !isCopyOnMaskEnabled())
					return;
				const String sel = m_buffer.selectedText();
				if (!sel.empty())
					SDL_SetClipboardText(sel);
			}
			else if (data.isCut())
			{
				if (hasCharacterMask() && !isCopyOnMaskEnabled())
					return;
				const String sel = m_buffer.selectedText();
				if (!sel.empty())
				{
					SDL_SetClipboardText(sel);
					m_buffer.deleteSelection();
				}
			}
			else if (data.isPaste())
			{
				const String clipboardText = SDL_GetClipboardText();
				if (!clipboardText.empty())
				{
					const String toInsert = clamp_input_to_max_length(clipboardText);
					if (!toInsert.empty())
						m_buffer.insertText(toInsert);
				}
			}
			else if (data.isSelectAll())
			{
				m_buffer.selectAll();
			}
			else if (data.isUndo())
			{
				if (m_buffer.canUndo())
					m_buffer.undo();
			}
			else if (data.isRedo())
			{
				if (m_buffer.canRedo())
					m_buffer.redo();
			}

			// ----- Cursor movement -----

			else if (data.keyCode == KeyCode::Left)
			{
				if (mask && word) m_buffer.moveDocumentStart(extend);
				else if (!mask && word) m_buffer.moveWordLeft(extend);
				else      m_buffer.moveLeft(extend);
			}
			else if (data.keyCode == KeyCode::Right)
			{
				if (mask && word) m_buffer.moveDocumentEnd(extend);
				else if (!mask && word) m_buffer.moveWordRight(extend);
				else      m_buffer.moveRight(extend);
			}
			else if (data.keyCode == KeyCode::Home || data.keyCode == KeyCode::Pageup)
			{
				// PageUp on single-line == Home. When you go multiline, split these.
				if (word) m_buffer.moveDocumentStart(extend);
				else      m_buffer.moveLineStart(extend);
			}
			else if (data.keyCode == KeyCode::End || data.keyCode == KeyCode::Pagedown)
			{
				if (word) m_buffer.moveDocumentEnd(extend);
				else      m_buffer.moveLineEnd(extend);
			}

			// ----- Deletion -----

			else if (data.keyCode == KeyCode::Backspace)
			{
				if (m_buffer.hasSelection())
				{
					m_buffer.deleteSelection();
				}
				else if (mask && word)
				{
					if (m_buffer.cursorByteOffset() > 0) {
						m_buffer.selectRange(0, m_buffer.cursorByteOffset());
						m_buffer.deleteSelection();
					}
				}
				else if (!mask && word) m_buffer.backspaceWord();
				else                    m_buffer.backspace();
			}
			else if (data.keyCode == KeyCode::Delete)
			{
				if (m_buffer.hasSelection())
				{
					m_buffer.deleteSelection();
				}
				else if (mask && word)
				{
					if (m_buffer.cursorByteOffset() < m_buffer.byteSize()) {
						m_buffer.selectRange(m_buffer.cursorByteOffset(), m_buffer.byteSize());
						m_buffer.deleteSelection();
					}
				}
				else if (!mask && word) m_buffer.deleteWord();
				else                    m_buffer.deleteForward();
			}

			// ----- Other -----

			else if (data.keyCode == KeyCode::Escape)
			{
				// Clear selection if there is one; otherwise no-op (the widget could
				// give up focus here, but that's an opinionated choice — leave it
				// up to the application via callback_onEscape if you want).
				if (m_buffer.hasSelection())
					m_buffer.clearSelection();
			}
			else if (data.keyCode == KeyCode::Return)
			{
				if (callback_onActionPerformed)
					callback_onActionPerformed(event);
			}
			else
			{
				// Unrecognized key — don't engage the repeat throttle, and don't
				// record m_lastKeyCode. Returning here keeps the throttle accurate
				// (if the user presses an unhandled key followed by a real one,
				// the real one fires immediately rather than being suppressed).
				return;
			}

			// Single bookkeeping point for every accepted branch.
			m_keyRepeatTimer.start();
			m_lastKeyCode = data.keyCode;
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
				if (hasCharacterMask())
					m_buffer.selectAll();
				else
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

		void TextEdit::setText(const String& text)
		{
			if (m_maxLength <= 0) {
				m_buffer.setText(text);
				return;
			}

			const u32 inputCps = String::utf8::count_codepoints(text.cpp_str());
			if (inputCps <= cast<u32>(m_maxLength)) {
				m_buffer.setText(text);
				return;
			}

			m_buffer.setText(String::utf8::truncate(text, cast<u32>(m_maxLength)));
		}

		void TextEdit::setMaxLength(i32 codepoints)
		{
			m_maxLength = codepoints;
			if (m_maxLength <= 0) return;

			const u32 currentCps = m_buffer.codepointCount();
			if (currentCps <= cast<u32>(m_maxLength)) return;

			const String truncated = String::utf8::truncate(m_buffer.text(), cast<u32>(m_maxLength));
			m_buffer.setText(truncated);
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

			const String displayText = hasCharacterMask()
				? String::duplicateChar(m_charMask, m_buffer.text().len())
				: m_buffer.text();

			// One call to the renderer. Returns one Vec2 per codepoint:
			//   .x = advance (incl. kerning) contributed by that glyph
			//   .y = glyph height (same for all glyphs at this font/size)
			const stdvec<Vec2> perChar =
				gfx.getStringDimensionsPerCharacter(displayText, getFontSize());

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

		String TextEdit::clamp_input_to_max_length(const String& utf8) const
		{
			if (m_maxLength <= 0) return utf8;

			const u32 currentCps = m_buffer.codepointCount();

			u32 selCps = 0;
			if (m_buffer.hasSelection()) {
				const u32 a = m_buffer.selectionStart();
				const u32 b = m_buffer.selectionEnd();
				const String selected = m_buffer.text().new_substr(a, cast<i32>(b));
				selCps = String::utf8::count_codepoints(selected.cpp_str());
			}

			const u32 insertedCps = String::utf8::count_codepoints(utf8.cpp_str());
			const u32 resultCps   = currentCps - selCps + insertedCps;
			const u32 capCps      = cast<u32>(m_maxLength);

			if (resultCps <= capCps) return utf8;

			// Overflow. Truncate ONLY in the appending case: cursor at end, no selection.
			const bool atEnd = (m_buffer.cursorByteOffset() == m_buffer.byteSize());
			if (!atEnd || m_buffer.hasSelection()) {
				// Mid-buffer insert or replace-selection: reject the whole input.
				return String("");
			}

			// Appending case: truncate to fit.
			const u32 availableCps = capCps - currentCps;
			return String::utf8::truncate(utf8, availableCps);
		}
	}
}
