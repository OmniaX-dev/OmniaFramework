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

#include "TextBuffer.hpp"
#include <ostd/utils/Time.hpp>

namespace ostd
{
	// ============================================================
	//  Construction
	// ============================================================

	TextBuffer::TextBuffer(void)
	{
		setTypeName("ostd::TextBuffer");
		validate();
		m_lineStarts.push_back(0);
		m_lineStartsDirty = false;
	}

	TextBuffer::TextBuffer(const String& initial, bool multiline)
	{
		setTypeName("ostd::TextBuffer");
		validate();
		m_multiline = multiline;
		m_buffer = initial;
		if (!m_multiline) strip_newlines_from_buffer();
		m_cursor = m_anchor = cast<u32>(m_buffer.len());
		m_lineStartsDirty = true;
	}


	// ============================================================
	//  Content access
	// ============================================================

	u32 TextBuffer::codepointCount(void) const
	{
		return String::utf8::count_codepoints(m_buffer.cpp_str());
	}

	u32 TextBuffer::lineCount(void) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		return cast<u32>(m_lineStarts.size());
	}

	String TextBuffer::getLine(u32 lineIdx) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		if (lineIdx >= m_lineStarts.size()) return String("");
		const u32 start = m_lineStarts[lineIdx];
		const u32 end = (lineIdx + 1 < m_lineStarts.size())
			? m_lineStarts[lineIdx + 1] - 1   // -1 to exclude the '\n'
			: cast<u32>(m_buffer.len());
		return m_buffer.new_substr(start, cast<i32>(end));
	}

	u32 TextBuffer::lineByteStart(u32 lineIdx) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		if (lineIdx >= m_lineStarts.size()) return cast<u32>(m_buffer.len());
		return m_lineStarts[lineIdx];
	}

	u32 TextBuffer::lineByteLength(u32 lineIdx) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		if (lineIdx >= m_lineStarts.size()) return 0;
		const u32 start = m_lineStarts[lineIdx];
		const u32 end = (lineIdx + 1 < m_lineStarts.size())
			? m_lineStarts[lineIdx + 1] - 1
			: cast<u32>(m_buffer.len());
		return end - start;
	}


	// ============================================================
	//  Mode
	// ============================================================

	void TextBuffer::setMultiline(bool ml)
	{
		if (m_multiline == ml) return;
		m_multiline = ml;
		if (!m_multiline)
		{
			// Strip newlines via replace_range so the change is undoable as
			// a single step. We compute the stripped form on a copy and
			// hand it to the chokepoint.
			String stripped = m_buffer;
			stripped.replaceAll("\r\n", " ");
			stripped.replaceAll("\n", " ");
			stripped.replaceAll("\r", " ");
			if (stripped.cpp_str() != m_buffer.cpp_str())
				replace_range(0, cast<u32>(m_buffer.len()), stripped, EditKind::Generic);
			// If nothing changed (no newlines to strip), no edit is recorded.
		}
	}


	// ============================================================
	//  Replace contents
	// ============================================================

	void TextBuffer::setText(const String& text)
	{
		// Strip newlines first if we're in single-line mode, since
		// replace_range writes the input as-is. We do this on a copy so the
		// caller's String isn't mutated.
		String to_set = text;
		if (!m_multiline)
		{
			to_set.replaceAll("\r\n", " ");
			to_set.replaceAll("\n", " ");
			to_set.replaceAll("\r", " ");
		}
		// Route through replace_range so this counts as an undoable operation.
		// replace_range will leave the cursor at the end of the inserted text,
		// matching the original setText() contract.
		replace_range(0, cast<u32>(m_buffer.len()), to_set, EditKind::Generic);
		m_desiredColumn = -1;
	}

	void TextBuffer::clear(void)
	{
		// replace_range already short-circuits no-op edits, so we don't need
		// the explicit empty-and-cursor-at-zero guard the original had.
		replace_range(0, cast<u32>(m_buffer.len()), String(""), EditKind::Generic);
		m_desiredColumn = -1;
	}


	// ============================================================
	//  Cursor / selection
	// ============================================================

	IPoint TextBuffer::cursorLineCol(void) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		const u32 line = line_index_for_byte(m_cursor);
		const u32 col  = codepoint_column_in_line(m_cursor);
		return IPoint(cast<i32>(col), cast<i32>(line));
	}

	void TextBuffer::setCursorByteOffset(u32 byte_offset)
	{
		const u32 clamped = clamp_to_codepoint_boundary(byte_offset);
		move_cursor_to(clamped, false);
		m_desiredColumn = -1;
	}

	void TextBuffer::setCursorLineCol(IPoint pos)
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		const i32 line_count = cast<i32>(m_lineStarts.size());
		i32 line = pos.y;
		if (line < 0) line = 0;
		if (line >= line_count) line = line_count - 1;
		const u32 col = pos.x < 0 ? 0 : cast<u32>(pos.x);
		const u32 byte = byte_offset_for_line_col(cast<u32>(line), col);
		move_cursor_to(byte, false);
		m_desiredColumn = -1;
	}

	String TextBuffer::selectedText(void) const
	{
		if (!hasSelection()) return String("");
		return m_buffer.new_substr(selectionStart(), cast<i32>(selectionEnd()));
	}

	void TextBuffer::clearSelection(void)
	{
		if (!hasSelection()) return;
		m_anchor = m_cursor;
		m_lastEditKind = EditKind::None;
		fire_cursor_moved();
	}

	void TextBuffer::selectAll(void)
	{
		m_anchor = 0;
		m_cursor = cast<u32>(m_buffer.len());
		m_desiredColumn = -1;
		m_lastEditKind = EditKind::None;
		fire_cursor_moved();
	}

	void TextBuffer::selectRange(u32 start_byte, u32 end_byte)
	{
		const u32 a = clamp_to_codepoint_boundary(start_byte);
		const u32 b = clamp_to_codepoint_boundary(end_byte);
		m_anchor = a;
		m_cursor = b;
		m_desiredColumn = -1;
		m_lastEditKind = EditKind::None;
		fire_cursor_moved();
	}


	// ============================================================
	//  Cursor movement
	// ============================================================

	void TextBuffer::moveLeft(bool extend)
	{
		m_desiredColumn = -1;
		if (!extend && hasSelection())
		{
			// Collapse selection to its left edge instead of moving past it —
			// matches the behavior of every text editor.
			move_cursor_to(selectionStart(), false);
			return;
		}
		if (m_cursor == 0)
		{
			if (!extend) clearSelection();
			return;
		}
		const u32 prev = String::utf8::prev_codepoint_start(m_buffer.cpp_str(), m_cursor);
		move_cursor_to(prev, extend);
	}

	void TextBuffer::moveRight(bool extend)
	{
		m_desiredColumn = -1;
		if (!extend && hasSelection())
		{
			move_cursor_to(selectionEnd(), false);
			return;
		}
		if (m_cursor >= m_buffer.len())
		{
			if (!extend) clearSelection();
			return;
		}
		const u32 next = String::utf8::next_codepoint_start(m_buffer.cpp_str(), m_cursor);
		move_cursor_to(next, extend);
	}

	void TextBuffer::moveUp(bool extend)
	{
		if (!m_multiline) { moveLineStart(extend); return; }
		if (m_lineStartsDirty) rebuild_line_starts();
		const u32 line = line_index_for_byte(m_cursor);
		if (line == 0)
		{
			move_cursor_to(0, extend);
			m_desiredColumn = -1;
			return;
		}
		const u32 col = (m_desiredColumn >= 0)
			? cast<u32>(m_desiredColumn)
			: codepoint_column_in_line(m_cursor);
		if (m_desiredColumn < 0) m_desiredColumn = cast<i32>(col);
		const u32 target = byte_offset_for_line_col(line - 1, col);
		move_cursor_to(target, extend);
	}

	void TextBuffer::moveDown(bool extend)
	{
		if (!m_multiline) { moveLineEnd(extend); return; }
		if (m_lineStartsDirty) rebuild_line_starts();
		const u32 line = line_index_for_byte(m_cursor);
		if (line + 1 >= m_lineStarts.size())
		{
			move_cursor_to(cast<u32>(m_buffer.len()), extend);
			m_desiredColumn = -1;
			return;
		}
		const u32 col = (m_desiredColumn >= 0)
			? cast<u32>(m_desiredColumn)
			: codepoint_column_in_line(m_cursor);
		if (m_desiredColumn < 0) m_desiredColumn = cast<i32>(col);
		const u32 target = byte_offset_for_line_col(line + 1, col);
		move_cursor_to(target, extend);
	}

	void TextBuffer::moveWordLeft(bool extend)
	{
		m_desiredColumn = -1;
		const u32 target = prev_word_boundary(m_cursor);
		move_cursor_to(target, extend);
	}

	void TextBuffer::moveWordRight(bool extend)
	{
		m_desiredColumn = -1;
		const u32 target = next_word_boundary(m_cursor);
		move_cursor_to(target, extend);
	}

	void TextBuffer::moveLineStart(bool extend)
	{
		m_desiredColumn = -1;
		if (m_lineStartsDirty) rebuild_line_starts();
		const u32 line = line_index_for_byte(m_cursor);
		move_cursor_to(m_lineStarts[line], extend);
	}

	void TextBuffer::moveLineEnd(bool extend)
	{
		m_desiredColumn = -1;
		if (m_lineStartsDirty) rebuild_line_starts();
		const u32 line = line_index_for_byte(m_cursor);
		const u32 end = (line + 1 < m_lineStarts.size())
			? m_lineStarts[line + 1] - 1
			: cast<u32>(m_buffer.len());
		move_cursor_to(end, extend);
	}

	void TextBuffer::moveDocumentStart(bool extend)
	{
		m_desiredColumn = -1;
		move_cursor_to(0, extend);
	}

	void TextBuffer::moveDocumentEnd(bool extend)
	{
		m_desiredColumn = -1;
		move_cursor_to(cast<u32>(m_buffer.len()), extend);
	}

	void TextBuffer::setCursorByteOffset(u32 byte_offset, bool extend)
	{
		const u32 clamped = clamp_to_codepoint_boundary(byte_offset);
		move_cursor_to(clamped, extend);
		m_desiredColumn = -1;
	}


	// ============================================================
	//  Mutation
	// ============================================================

	void TextBuffer::insertText(const String& text)
	{
		String to_insert = text;
		if (!m_multiline)
		{
			// Strip newlines from the input rather than rejecting wholesale.
			// This matches what users expect when pasting multi-line text
			// into a single-line field.
			to_insert.replaceAll("\r\n", " ");
			to_insert.replaceAll("\n", " ");
			to_insert.replaceAll("\r", " ");
		}
		else
		{
			// Normalize line endings to '\n' on the way in. This is the only
			// place we need to think about CRLF vs LF — once inside the buffer,
			// everything is LF.
			to_insert.replaceAll("\r\n", "\n");
			to_insert.replaceAll("\r", "\n");
		}
		// Decide the edit kind for undo coalescing. The caller might be
		// pasting a whole document (definitely Generic — one undo step for
		// the entire paste) or it might be the keyboard handler shovelling
		// one typed character at a time through this method (TextEdit does
		// exactly that — every keystroke arrives as a single-codepoint
		// String). For the typing case we want the same coalescing as
		// insertCodepoint, so consecutive typed characters collapse into
		// one undo step.
		//
		// Note: we deliberately *don't* exclude the selection-replace case
		// here. The selection-replace becomes one InsertCodepoint entry,
		// and subsequent typing merges into it — so "select, type 'hello'"
		// is undone in one step. The kind-mismatch check in
		// try_coalesce_with_top stops this entry from merging backward
		// into whatever Generic edit preceded the selection, which is the
		// part that actually matters.
		EditKind kind = EditKind::Generic;
		if (!to_insert.empty()
			&& String::utf8::count_codepoints(to_insert.cpp_str()) == 1
			&& to_insert.cpp_str() != "\n")
		{
			kind = EditKind::InsertCodepoint;
		}
		replace_range(selectionStart(), selectionEnd(), to_insert, kind);
		m_desiredColumn = -1;
	}

	void TextBuffer::insertCodepoint(u32 codepoint)
	{
		if (!m_multiline && codepoint == '\n') return;
		const String encoded = String::utf8::encode(codepoint);
		if (encoded.empty()) return;  // Invalid codepoint — silently drop.
		// Tag as InsertCodepoint unconditionally (except for newlines, where
		// each line break is a natural undo boundary). When this replaces a
		// selection, the kind-mismatch check in try_coalesce_with_top stops
		// it from merging into the previous entry — but it can serve as the
		// *start* of a fresh coalesce chain that subsequent typing extends,
		// which means "select, type 'hello'" undoes in one step.
		const EditKind kind = (codepoint == '\n')
			? EditKind::Generic
			: EditKind::InsertCodepoint;
		replace_range(selectionStart(), selectionEnd(), encoded, kind);
		m_desiredColumn = -1;
	}

	void TextBuffer::backspace(void)
	{
		m_desiredColumn = -1;
		if (hasSelection())
		{
			replace_range(selectionStart(), selectionEnd(), String(""));
			return;
		}
		if (m_cursor == 0) return;
		const u32 prev = String::utf8::prev_codepoint_start(m_buffer.cpp_str(), m_cursor);
		replace_range(prev, m_cursor, String(""), EditKind::Backspace);
	}

	void TextBuffer::deleteForward(void)
	{
		m_desiredColumn = -1;
		if (hasSelection())
		{
			replace_range(selectionStart(), selectionEnd(), String(""));
			return;
		}
		if (m_cursor >= m_buffer.len()) return;
		const u32 next = String::utf8::next_codepoint_start(m_buffer.cpp_str(), m_cursor);
		replace_range(m_cursor, next, String(""), EditKind::DeleteForward);
	}

	void TextBuffer::backspaceWord(void)
	{
		m_desiredColumn = -1;
		if (hasSelection())
		{
			replace_range(selectionStart(), selectionEnd(), String(""));
			return;
		}
		if (m_cursor == 0) return;
		const u32 target = prev_word_boundary(m_cursor);
		replace_range(target, m_cursor, String(""));
	}

	void TextBuffer::deleteWord(void)
	{
		m_desiredColumn = -1;
		if (hasSelection())
		{
			replace_range(selectionStart(), selectionEnd(), String(""));
			return;
		}
		if (m_cursor >= m_buffer.len()) return;
		const u32 target = next_word_boundary(m_cursor);
		replace_range(m_cursor, target, String(""));
	}

	void TextBuffer::deleteCurrentLine(void)
	{
		m_desiredColumn = -1;
		if (m_lineStartsDirty) rebuild_line_starts();
		const u32 line = line_index_for_byte(m_cursor);
		const u32 start = m_lineStarts[line];
		// Delete through the trailing newline (if there is one) so the
		// next line moves up. On the last line, just delete to end.
		const u32 end = (line + 1 < m_lineStarts.size())
			? m_lineStarts[line + 1]
			: cast<u32>(m_buffer.len());
		replace_range(start, end, String(""));
	}

	void TextBuffer::deleteSelection(void)
	{
		if (hasSelection()) {
			replace_range(selectionStart(), selectionEnd(), String(""));
			return;
		}
	}


	// ============================================================
	//  toString
	// ============================================================

	String TextBuffer::toString(void) const
	{
		String s = getObjectHeaderString();
		s.add(" { bytes=");
		s.add(cast<u32>(m_buffer.len()));
		s.add(", lines=");
		s.add(lineCount());
		s.add(", cursor@");
		s.add(m_cursor);
		s.add(m_multiline ? ", multiline }" : ", single-line }");
		return s;
	}


	// ============================================================
	//  Private — chokepoints
	// ============================================================

	void TextBuffer::replace_range(u32 start_byte, u32 end_byte, const String& with,
	                               EditKind kind)
	{
		// Defensive clamping. In normal usage the public methods already
		// produce well-formed boundaries, but we want the chokepoint itself
		// to be unconditionally safe.
		if (start_byte > m_buffer.len()) start_byte = cast<u32>(m_buffer.len());
		if (end_byte   > m_buffer.len()) end_byte   = cast<u32>(m_buffer.len());
		if (start_byte > end_byte) std::swap(start_byte, end_byte);

		// Skip no-op edits to avoid spurious notifications.
		if (start_byte == end_byte && with.empty()) return;

		// Snapshot pre-edit state for undo recording. We must do this BEFORE
		// mutating m_buffer or m_cursor/m_anchor.
		const u32    cursor_before = m_cursor;
		const u32    anchor_before = m_anchor;
		const String removed       = m_buffer.new_substr(start_byte, cast<i32>(end_byte));

		// Build the new buffer.
		String head = m_buffer.new_substr(0, cast<i32>(start_byte));
		String tail = m_buffer.new_substr(end_byte, cast<i32>(m_buffer.len()));
		m_buffer = head + with + tail;

		// Cursor lands at the end of the inserted text. Selection collapses.
		const u32 new_cursor = start_byte + cast<u32>(with.len());
		m_cursor = m_anchor = new_cursor;

		m_lineStartsDirty = true;

		// Record this edit on the undo stack — but only if it didn't itself
		// originate from undo()/redo() applying a stored entry.
		if (!m_applyingHistory)
		{
			UndoEntry e;
			e.start         = start_byte;
			e.removed       = removed;
			e.inserted      = with;
			e.cursor_before = cursor_before;
			e.anchor_before = anchor_before;
			e.kind          = kind;
			e.timestamp_ms  = Time::getRunningTime_ms();
			push_undo_entry(std::move(e));
		}

		fire_changed();
		// During history application the apply_*_entry functions will set
		// the final cursor/anchor and fire onCursorMoved themselves — firing
		// here would emit an intermediate position that observers might act
		// on (e.g. scroll the view). Skip it in that case.
		if (!m_applyingHistory) fire_cursor_moved();
	}

	void TextBuffer::move_cursor_to(u32 byte_offset, bool extend)
	{
		if (byte_offset > m_buffer.len()) byte_offset = cast<u32>(m_buffer.len());
		// We trust the caller produced a codepoint-aligned offset; the public
		// movement primitives all do. This is the "internal" entrypoint.
		const u32 old_cursor = m_cursor;
		const u32 old_anchor = m_anchor;
		m_cursor = byte_offset;
		if (!extend) m_anchor = byte_offset;
		if (m_cursor != old_cursor || m_anchor != old_anchor)
		{
			// Any deliberate cursor movement breaks the undo coalesce chain.
			// Without this, typing "ab", moving the cursor, then typing "cd"
			// would produce a single undo step "abcd" — surprising to the user.
			m_lastEditKind = EditKind::None;
			fire_cursor_moved();
		}
	}


	// ============================================================
	//  Private — line tracking
	// ============================================================

	void TextBuffer::rebuild_line_starts(void) const
	{
		m_lineStarts.clear();
		m_lineStarts.push_back(0);
		const cpp_string& s = m_buffer.cpp_str();  // Note: returns by value;
		// since cpp_str() returns a copy, this scan is over a temporary, which
		// is fine for a one-off rebuild but worth being aware of. If you ever
		// add cpp_str_ref() const, prefer that here.
		for (u32 i = 0; i < s.size(); i++)
		{
			if (s[i] == '\n')
				m_lineStarts.push_back(i + 1);
		}
		m_lineStartsDirty = false;
	}

	u32 TextBuffer::line_index_for_byte(u32 byte_offset) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		// Binary search: find largest line whose start <= byte_offset.
		u32 lo = 0, hi = cast<u32>(m_lineStarts.size());
		while (lo + 1 < hi)
		{
			const u32 mid = (lo + hi) / 2;
			if (m_lineStarts[mid] <= byte_offset) lo = mid;
			else hi = mid;
		}
		return lo;
	}


	// ============================================================
	//  Private — column / line+col conversions
	// ============================================================

	u32 TextBuffer::codepoint_column_in_line(u32 byte_offset) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		const u32 line = line_index_for_byte(byte_offset);
		const u32 line_start = m_lineStarts[line];
		// Count codepoints between line_start and byte_offset.
		const cpp_string& s = m_buffer.cpp_str();
		u32 col = 0;
		u32 i = line_start;
		while (i < byte_offset && i < s.size())
		{
			if ((cast<u8>(s[i]) & 0xC0) != 0x80) col++;
			i++;
		}
		return col;
	}

	u32 TextBuffer::byte_offset_for_line_col(u32 line, u32 codepoint_col) const
	{
		if (m_lineStartsDirty) rebuild_line_starts();
		if (m_lineStarts.empty()) return 0;
		if (line >= m_lineStarts.size()) line = cast<u32>(m_lineStarts.size()) - 1;
		const u32 line_start = m_lineStarts[line];
		const u32 line_end = (line + 1 < m_lineStarts.size())
			? m_lineStarts[line + 1] - 1
			: cast<u32>(m_buffer.len());

		// Walk forward codepoint by codepoint from line_start, stopping when
		// we hit the requested column or the end of the line.
		u32 i = line_start;
		u32 col = 0;
		const cpp_string s = m_buffer.cpp_str();
		while (i < line_end && col < codepoint_col)
		{
			i = String::utf8::next_codepoint_start(s, i);
			col++;
		}
		return i;
	}


	// ============================================================
	//  Private — word boundaries
	// ============================================================

	// Word-class categorization. We use a simple tri-state:
	//   - Word:        ASCII letters/digits/underscore, plus any non-ASCII
	//                  byte (treats all non-ASCII as "word" — good enough
	//                  default for European/CJK text without a Unicode DB).
	//   - Whitespace:  ASCII space/tab/newline/CR.
	//   - Punctuation: everything else printable ASCII.
	//
	// More sophisticated word segmentation (UAX #29) is out of scope; this
	// rule is what most editors implement and it matches user intuition for
	// the languages our font is likely to support.
	namespace {
		enum class WordClass { Word, Whitespace, Punctuation };

		WordClass classify_byte(u8 b)
		{
			if (b == ' ' || b == '\t' || b == '\n' || b == '\r')
				return WordClass::Whitespace;
			if ((b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z') ||
				(b >= '0' && b <= '9') || b == '_')
				return WordClass::Word;
			if (b >= 0x80)  // Treat all non-ASCII as word.
				return WordClass::Word;
			return WordClass::Punctuation;
		}
	}

	void TextBuffer::selectWordAt(u32 byte_offset)
	{
		m_desiredColumn = -1;
		if (m_buffer.empty()) return;

		// Clamp to a valid codepoint boundary first. If we're past end, back up
		// to the last codepoint so we have something to classify.
		u32 pos = clamp_to_codepoint_boundary(byte_offset);
		if (pos >= m_buffer.len()) {
			// At end-of-buffer — step back one codepoint to get something to
			// classify, then expand from there.
			if (m_buffer.len() == 0) return;
			pos = String::utf8::prev_codepoint_start(m_buffer.cpp_str(), m_buffer.len());
		}

		// Expand left and right from `pos`. The trick: prev_word_boundary and
		// next_word_boundary are *almost* what we want, but they're designed
		// for "skip the run AND following whitespace" semantics. For word
		// *selection*, we want just the run.
		//
		// Easiest: classify the byte at pos, then walk outward in both
		// directions while the class matches.

		const cpp_string& s = m_buffer.cpp_str();
		const WordClass cls = classify_byte(cast<u8>(s[pos]));

		// Walk left: advance start backward while previous codepoint is same class.
		u32 start = pos;
		while (start > 0) {
			const u32 prev = String::utf8::prev_codepoint_start(s, start);
			if (classify_byte(cast<u8>(s[prev])) != cls) break;
			start = prev;
		}

		// Walk right: advance end forward while current codepoint is same class.
		u32 end = pos;
		while (end < s.size()) {
			if (classify_byte(cast<u8>(s[end])) != cls) break;
			end = String::utf8::next_codepoint_start(s, end);
		}

		selectRange(start, end);
	}

	u32 TextBuffer::prev_word_boundary(u32 byte_offset) const
	{
		const cpp_string& s = m_buffer.cpp_str();
		if (byte_offset == 0) return 0;
		u32 i = byte_offset;
		// Step 1: skip any whitespace immediately to the left.
		while (i > 0)
		{
			const u32 prev = String::utf8::prev_codepoint_start(s, i);
			if (classify_byte(cast<u8>(s[prev])) != WordClass::Whitespace) break;
			i = prev;
		}
		// Step 2: skip the run of same-class characters to the left.
		if (i == 0) return 0;
		const u32 first_prev = String::utf8::prev_codepoint_start(s, i);
		const WordClass cls = classify_byte(cast<u8>(s[first_prev]));
		while (i > 0)
		{
			const u32 prev = String::utf8::prev_codepoint_start(s, i);
			if (classify_byte(cast<u8>(s[prev])) != cls) break;
			i = prev;
		}
		return i;
	}

	u32 TextBuffer::next_word_boundary(u32 byte_offset) const
	{
		const cpp_string& s = m_buffer.cpp_str();
		const u32 size = cast<u32>(s.size());
		if (byte_offset >= size) return size;
		u32 i = byte_offset;
		// Step 1: skip the run of same-class characters at the cursor.
		const WordClass cls = classify_byte(cast<u8>(s[i]));
		while (i < size && classify_byte(cast<u8>(s[i])) == cls)
			i = String::utf8::next_codepoint_start(s, i);
		// Step 2: skip any whitespace following.
		while (i < size && classify_byte(cast<u8>(s[i])) == WordClass::Whitespace)
			i = String::utf8::next_codepoint_start(s, i);
		return i;
	}


	// ============================================================
	//  Private — utility
	// ============================================================

	u32 TextBuffer::clamp_to_codepoint_boundary(u32 byte_offset) const
	{
		if (byte_offset >= m_buffer.len()) return cast<u32>(m_buffer.len());
		// If we're on a continuation byte, walk back to the lead byte.
		return String::utf8::align_to_codepoint(m_buffer.cpp_str(), byte_offset);
	}

	void TextBuffer::strip_newlines_from_buffer(void)
	{
		m_buffer.replaceAll("\r\n", " ");
		m_buffer.replaceAll("\n", " ");
		m_buffer.replaceAll("\r", " ");
	}

	void TextBuffer::fire_changed(void)
	{
		if (m_onChanged) m_onChanged(*this);
	}

	void TextBuffer::fire_cursor_moved(void)
	{
		if (m_onCursorMoved) m_onCursorMoved(*this);
	}


	// ============================================================
	//  Undo / Redo
	// ============================================================

	bool TextBuffer::undo(void)
	{
		if (m_undoStack.empty()) return false;

		// Move the entry from undo to redo before applying, so that the
		// reentrancy guard sees a clean stack-top — we don't want
		// apply_entry_inverse() to accidentally try to coalesce into the
		// entry we're undoing.
		UndoEntry e = std::move(m_undoStack.back());
		m_undoStack.pop_back();

		apply_entry_inverse(e);

		m_redoStack.push_back(std::move(e));

		// Any new edit after an undo starts a fresh chain — never coalesce
		// across an undo boundary.
		m_lastEditKind = EditKind::None;
		m_desiredColumn = -1;
		return true;
	}

	bool TextBuffer::redo(void)
	{
		if (m_redoStack.empty()) return false;

		UndoEntry e = std::move(m_redoStack.back());
		m_redoStack.pop_back();

		apply_entry_forward(e);

		m_undoStack.push_back(std::move(e));

		m_lastEditKind = EditKind::None;
		m_desiredColumn = -1;
		return true;
	}

	void TextBuffer::clearHistory(void)
	{
		m_undoStack.clear();
		m_redoStack.clear();
		m_lastEditKind = EditKind::None;
	}

	void TextBuffer::setMaxUndoSteps(u32 max)
	{
		m_maxUndoSteps = max;
		if (max == 0) return;  // Unlimited — nothing to trim.
		// Drop oldest entries from the front until we fit. Both stacks share
		// the same limit; redo entries existed in undo before they were
		// undone, so trimming undo while leaving redo unbounded would be
		// inconsistent.
		while (m_undoStack.size() > m_maxUndoSteps) m_undoStack.pop_front();
		while (m_redoStack.size() > m_maxUndoSteps) m_redoStack.pop_front();
	}

	void TextBuffer::push_undo_entry(UndoEntry entry)
	{
		// A new edit invalidates any pending redos — the linear undo model.
		// (Branching/persistent undo is out of scope; the linear model is
		// what every text editor does.)
		m_redoStack.clear();

		// Try to coalesce this edit with the one on top of the stack.
		if (!try_coalesce_with_top(entry))
		{
			m_undoStack.push_back(std::move(entry));
			// Enforce the size limit by dropping the oldest entry. We do this
			// inside the non-coalesce branch because coalescing doesn't grow
			// the stack.
			if (m_maxUndoSteps != 0 && m_undoStack.size() > m_maxUndoSteps)
				m_undoStack.pop_front();
		}

		// Record the kind of the most-recent edit for the *next* coalesce
		// decision. We do this after coalescing because the coalesced entry
		// retains the same kind as the previous one.
		m_lastEditKind = m_undoStack.empty() ? EditKind::None : m_undoStack.back().kind;
	}

	bool TextBuffer::try_coalesce_with_top(const UndoEntry& candidate)
	{
		// Coalescing requires:
		//  - A previous entry exists.
		//  - The previous edit's kind is the same coalescible kind.
		//  - The candidate is itself a coalescible kind (Generic never merges).
		//  - The candidate occurred within the timeout window of the previous.
		//  - The kinds aren't EditKind::None or EditKind::Generic.
		if (m_undoStack.empty()) return false;
		if (m_coalesceTimeout_ms == 0) return false;
		if (candidate.kind != m_lastEditKind) return false;
		if (candidate.kind == EditKind::None || candidate.kind == EditKind::Generic)
			return false;

		UndoEntry& top = m_undoStack.back();

		// Hard timeout: we don't merge edits that are more than
		// m_coalesceTimeout_ms apart, even if everything else lines up. This
		// matches the behavior of pretty much every text editor — a pause in
		// typing is the user's natural "step boundary".
		if (candidate.timestamp_ms < top.timestamp_ms) return false;  // clock weirdness
		if (candidate.timestamp_ms - top.timestamp_ms > m_coalesceTimeout_ms)
			return false;

		switch (candidate.kind)
		{
			case EditKind::InsertCodepoint:
			{
				// New insert must start exactly where the previous one ended.
				const u32 prev_end = top.start + cast<u32>(top.inserted.len());
				if (candidate.start != prev_end) return false;
				// Merge: append the new inserted text. cursor_before / anchor_before
				// stay as the *original* values so undo rewinds to the start of
				// the typing run.
				top.inserted = top.inserted + candidate.inserted;
				top.timestamp_ms = candidate.timestamp_ms;
				return true;
			}
			case EditKind::Backspace:
			{
				// Each successive backspace removes a codepoint immediately to
				// the LEFT of the previous removal. So candidate's range ends
				// where top's range begins.
				if (candidate.start + cast<u32>(candidate.removed.len()) != top.start)
					return false;
				// Merge: prepend the newly-removed codepoint, slide start left.
				top.removed = candidate.removed + top.removed;
				top.start = candidate.start;
				top.timestamp_ms = candidate.timestamp_ms;
				return true;
			}
			case EditKind::DeleteForward:
			{
				// Each successive delete-forward removes a codepoint at the
				// SAME position (cursor doesn't move on forward delete). So
				// candidate.start should equal top.start.
				if (candidate.start != top.start) return false;
				// Merge: append the newly-removed codepoint to the run.
				top.removed = top.removed + candidate.removed;
				top.timestamp_ms = candidate.timestamp_ms;
				return true;
			}
			default:
				return false;
		}
	}

	void TextBuffer::apply_entry_inverse(const UndoEntry& e)
	{
		// Undo replaces what was inserted with what was removed, then
		// restores the cursor and anchor to their pre-edit positions. We use
		// the chokepoint with the reentrancy guard so notifications still
		// fire but no new history is recorded. replace_range suppresses its
		// own cursor-moved fire while m_applyingHistory is true; we fire it
		// here exactly once with the final restored cursor/anchor.
		const u32 inserted_end = e.start + cast<u32>(e.inserted.len());
		m_applyingHistory = true;
		replace_range(e.start, inserted_end, e.removed);
		m_applyingHistory = false;
		m_cursor = e.cursor_before;
		m_anchor = e.anchor_before;
		fire_cursor_moved();
	}

	void TextBuffer::apply_entry_forward(const UndoEntry& e)
	{
		// Redo re-applies the original edit. replace_range's natural
		// post-edit cursor (at start + inserted.len(), anchor == cursor)
		// matches what an in-progress edit would leave behind, so we don't
		// need to override it. Same pattern: suppress replace_range's
		// cursor-moved fire and emit one ourselves.
		const u32 removed_end = e.start + cast<u32>(e.removed.len());
		m_applyingHistory = true;
		replace_range(e.start, removed_end, e.inserted);
		m_applyingHistory = false;
		fire_cursor_moved();
	}
}
