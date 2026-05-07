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

#include <functional>
#include <ostd/data/BaseObject.hpp>
#include <ostd/string/String.hpp>
#include <ostd/math/Geometry.hpp>

namespace ostd
{
	/**
	 * TextBuffer — owns a UTF-8 text document and manages cursor/selection
	 * state along with all higher-level editing operations.
	 *
	 * Design invariants (these hold AT ALL TIMES outside private methods):
	 *   1. m_buffer contains valid UTF-8.
	 *   2. m_cursor and m_anchor are byte offsets in [0, m_buffer.size()].
	 *   3. m_cursor and m_anchor always sit on a codepoint boundary.
	 *   4. When m_multiline is false, m_buffer contains no '\n' bytes.
	 *
	 * Coordinate spaces:
	 *   - The canonical cursor representation is a single byte offset.
	 *   - IPoint line/col is a derived view, computed on demand from a
	 *     cached line-starts table that's lazily rebuilt after edits.
	 *   - "col" in IPoint is measured in CODEPOINTS, not bytes — this is
	 *     what callers (renderers, click hit-testing) actually want.
	 *
	 * Notifications:
	 *   - onChanged fires whenever buffer content is mutated.
	 *   - onCursorMoved fires whenever the cursor or selection changes
	 *     without a content change.
	 *   - Both fire on operations that do both (the cursor-move callback
	 *     fires after the change callback, so observers can scroll the
	 *     view to follow the cursor *after* layout has been updated).
	 */
	class TextBuffer : public BaseObject
	{
		public: using ChangeCallback = std::function<void(const TextBuffer&)>;

		public:
			TextBuffer(void);
			explicit TextBuffer(const String& initial, bool multiline = false);

			// ===================== Content access =====================
			inline const String& text(void) const { return m_buffer; }
			inline u32 byteSize(void) const { return cast<u32>(m_buffer.len()); }
			inline bool empty(void) const { return m_buffer.empty(); }
			u32 codepointCount(void) const;
			u32 lineCount(void) const;
			String getLine(u32 lineIdx) const;
			u32 lineByteStart(u32 lineIdx) const;
			u32 lineByteLength(u32 lineIdx) const;  // Excludes the trailing '\n'.

			// ====================== Mode toggle =======================
			// When switching to single-line, any existing newlines are stripped.
			void setMultiline(bool ml);
			inline bool isMultiline(void) const { return m_multiline; }

			// ===================== Replace contents ===================
			// Replaces the entire buffer atomically. Cursor is clamped/reset.
			void setText(const String& text);
			void clear(void);

			// ====================== Cursor state ======================
			inline u32 cursorByteOffset(void) const { return m_cursor; }
			IPoint cursorLineCol(void) const;
			void setCursorByteOffset(u32 byte_offset);
			void setCursorLineCol(IPoint pos);

			// ====================== Selection =========================
			inline bool hasSelection(void) const { return m_cursor != m_anchor; }
			inline u32 selectionStart(void) const { return m_cursor < m_anchor ? m_cursor : m_anchor; }
			inline u32 selectionEnd(void) const   { return m_cursor < m_anchor ? m_anchor : m_cursor; }
			inline u32 anchorByteOffset(void) const { return m_anchor; }
			String selectedText(void) const;
			void clearSelection(void);
			void selectAll(void);
			void selectWordAt(u32 byte_offset);
			void selectRange(u32 start_byte, u32 end_byte);

			// ===================== Cursor movement ====================
			// Each of these collapses the selection unless extend == true.
			void moveLeft(bool extend = false);
			void moveRight(bool extend = false);
			void moveUp(bool extend = false);
			void moveDown(bool extend = false);
			void moveWordLeft(bool extend = false);
			void moveWordRight(bool extend = false);
			void moveLineStart(bool extend = false);
			void moveLineEnd(bool extend = false);
			void moveDocumentStart(bool extend = false);
			void moveDocumentEnd(bool extend = false);
			void setCursorByteOffset(u32 byte_offset, bool extend);

			// ======================== Mutation ========================
			// All mutators replace the current selection (if any) before
			// performing their action.
			void insertText(const String& text);
			void insertCodepoint(u32 codepoint);
			void backspace(void);
			void deleteForward(void);
			void backspaceWord(void);
			void deleteWord(void);
			void deleteCurrentLine(void);
			void deleteSelection(void);

			// ===================== Notifications ======================
			inline void setOnChanged(ChangeCallback cb)     { m_onChanged = std::move(cb); }
			inline void setOnCursorMoved(ChangeCallback cb) { m_onCursorMoved = std::move(cb); }

			// ==================== BaseObject hooks ====================
			String toString(void) const override;

		private:
			// The two chokepoints — every public method funnels through these.
			// replace_range is the only place buffer content changes.
			// move_cursor_to is the only place m_cursor/m_anchor change.
			void replace_range(u32 start_byte, u32 end_byte, const String& with);
			void move_cursor_to(u32 byte_offset, bool extend);

			// Helpers
			void rebuild_line_starts(void) const;
			u32 line_index_for_byte(u32 byte_offset) const;
			u32 prev_word_boundary(u32 byte_offset) const;
			u32 next_word_boundary(u32 byte_offset) const;
			u32 codepoint_column_in_line(u32 byte_offset) const;
			u32 byte_offset_for_line_col(u32 line, u32 codepoint_col) const;
			u32 clamp_to_codepoint_boundary(u32 byte_offset) const;
			void strip_newlines_from_buffer(void);

			void fire_changed(void);
			void fire_cursor_moved(void);

		private:
			String m_buffer;
			u32 m_cursor { 0 };
			u32 m_anchor { 0 };

			// Sticky desired column for vertical movement: when you move up
			// from col 30 onto a 5-char line, the cursor visually goes to
			// col 5 but remembers 30, so moving up again lands at col 30 of
			// that line. Reset to -1 by any horizontal movement or edit.
			i32 m_desiredColumn { -1 };

			bool m_multiline { false };

			// Line-starts cache: byte offsets where each line begins.
			// m_lineStarts[0] is always 0. Rebuilt lazily on access.
			mutable stdvec<u32> m_lineStarts;
			mutable bool m_lineStartsDirty { true };

			ChangeCallback m_onChanged { nullptr };
			ChangeCallback m_onCursorMoved { nullptr };
	};
}
