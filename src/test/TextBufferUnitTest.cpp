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

// Standalone tests for ostd::TextBuffer.
// Exercises invariants, edge cases, UTF-8 handling, multiline behavior,
// word boundaries, selection, and the desired-column sticky behavior.
//
// Run as a separate executable. No external test framework — just a
// minimal in-house assertion macro that records pass/fail counts.

#include <ostd/io/IOHandlers.hpp>
#include <ostd/string/String.hpp>
#include <ostd/string/TextBuffer.hpp>

namespace {

// ============================================================
//  Tiny test harness
// ============================================================

struct TestStats {
	u32 total { 0 };
	u32 passed { 0 };
	u32 failed { 0 };
	String currentTest;
	stdvec<String> failures;
};

static TestStats g_stats;
static ostd::ConsoleOutputHandler g_out;

static void __record_pass(void)
{
	g_stats.total++;
	g_stats.passed++;
}

static void __record_fail(const String& test, const String& detail, i32 line)
{
	g_stats.total++;
	g_stats.failed++;
	String msg;
	msg.add("  [").add(test).add(":").add(line).add("] ").add(detail);
	g_stats.failures.push_back(msg);
	g_out.fg(ostd::ConsoleColors::BrightRed).p("F").reset().flush();
}

#define CHECK(cond)                                                          \
	do {                                                                     \
		if (cond) { __record_pass();                                         \
			g_out.fg(ostd::ConsoleColors::BrightGreen).p(".").reset().flush(); \
		}                                                                    \
		else {                                                               \
			__record_fail(g_stats.currentTest, String("CHECK failed: ") + #cond, __LINE__); \
		}                                                                    \
	} while(0)

#define CHECK_EQ(actual, expected)                                                          \
	do {                                                                                    \
		auto __a = (actual);                                                                \
		auto __e = (expected);                                                              \
		if (__a == __e) { __record_pass();                                                  \
			g_out.fg(ostd::ConsoleColors::BrightGreen).p(".").reset().flush();              \
		}                                                                                   \
		else {                                                                              \
			String __msg;                                                                   \
			__msg.add("CHECK_EQ failed: ").add(#actual).add(" == ").add(#expected);         \
			__msg.add("  (got ").add(String("").add(__a)).add(", expected ").add(String("").add(__e)).add(")"); \
			__record_fail(g_stats.currentTest, __msg, __LINE__);                            \
		}                                                                                   \
	} while(0)

// String comparison overload — String::add doesn't accept String, so handle separately.
#define CHECK_STR_EQ(actual, expected)                                                      \
	do {                                                                                    \
		String __a = (actual);                                                              \
		String __e = (expected);                                                            \
		if (__a == __e.c_str()) { __record_pass();                                          \
			g_out.fg(ostd::ConsoleColors::BrightGreen).p(".").reset().flush();              \
		}                                                                                   \
		else {                                                                              \
			String __msg;                                                                   \
			__msg.add("CHECK_STR_EQ failed:  got \"").add(__a).add("\", expected \"").add(__e).add("\""); \
			__record_fail(g_stats.currentTest, __msg, __LINE__);                            \
		}                                                                                   \
	} while(0)

#define TEST(name)                                                                          \
	g_stats.currentTest = name;                                                             \
	g_out.nl().fg(ostd::ConsoleColors::Cyan).p("[").p(name).p("] ").reset().flush();


// ============================================================
//  String literals for the UTF-8 test corpus
// ============================================================
//
// "café"       = 'c' 'a' 'f' (0xC3 0xA9)         → 5 bytes, 4 codepoints
// "日本"       = (0xE6 0x97 0xA5) (0xE6 0x9C 0xAC) → 6 bytes, 2 codepoints
// "😀"         = (0xF0 0x9F 0x98 0x80)              → 4 bytes, 1 codepoint
// "héllo"      = 'h' (0xC3 0xA9) 'l' 'l' 'o'        → 6 bytes, 5 codepoints

// ============================================================
//  Tests
// ============================================================

void test_construction_and_basic(void)
{
	TEST("construction_and_basic");

	ostd::TextBuffer empty;
	CHECK(empty.empty());
	CHECK_EQ(empty.byteSize(), 0u);
	CHECK_EQ(empty.cursorByteOffset(), 0u);
	CHECK_EQ(empty.lineCount(), 1u);  // Always at least one (empty) line.
	CHECK(!empty.hasSelection());

	ostd::TextBuffer hello("hello");
	CHECK_EQ(hello.byteSize(), 5u);
	CHECK_EQ(hello.codepointCount(), 5u);
	CHECK_EQ(hello.cursorByteOffset(), 5u);  // Constructor parks cursor at end.
	CHECK_STR_EQ(hello.text(), "hello");
}

void test_setText_and_clear(void)
{
	TEST("setText_and_clear");

	ostd::TextBuffer buf;
	buf.setText("hello");
	CHECK_STR_EQ(buf.text(), "hello");
	CHECK_EQ(buf.cursorByteOffset(), 5u);

	buf.clear();
	CHECK(buf.empty());
	CHECK_EQ(buf.cursorByteOffset(), 0u);
	CHECK_EQ(buf.lineCount(), 1u);
}

void test_simple_insert_and_backspace(void)
{
	TEST("simple_insert_and_backspace");

	ostd::TextBuffer buf;
	buf.insertCodepoint('h');
	buf.insertCodepoint('i');
	CHECK_STR_EQ(buf.text(), "hi");
	CHECK_EQ(buf.cursorByteOffset(), 2u);

	buf.backspace();
	CHECK_STR_EQ(buf.text(), "h");
	CHECK_EQ(buf.cursorByteOffset(), 1u);

	buf.backspace();
	CHECK(buf.empty());

	// Backspace on empty must be a no-op.
	buf.backspace();
	CHECK(buf.empty());
	CHECK_EQ(buf.cursorByteOffset(), 0u);
}

void test_insert_text_at_position(void)
{
	TEST("insert_text_at_position");

	ostd::TextBuffer buf("hello world");
	buf.setCursorByteOffset(5);
	buf.insertText(",");
	CHECK_STR_EQ(buf.text(), "hello, world");
	CHECK_EQ(buf.cursorByteOffset(), 6u);

	buf.setCursorByteOffset(0);
	buf.insertText("Why ");
	CHECK_STR_EQ(buf.text(), "Why hello, world");
	CHECK_EQ(buf.cursorByteOffset(), 4u);
}

void test_utf8_backspace(void)
{
	TEST("utf8_backspace");

	// "café" — 5 bytes, 4 codepoints. Backspacing once removes 'é' (2 bytes).
	ostd::TextBuffer buf("café");
	CHECK_EQ(buf.byteSize(), 5u);
	CHECK_EQ(buf.codepointCount(), 4u);
	CHECK_EQ(buf.cursorByteOffset(), 5u);

	buf.backspace();
	CHECK_STR_EQ(buf.text(), "caf");
	CHECK_EQ(buf.cursorByteOffset(), 3u);
	CHECK_EQ(buf.codepointCount(), 3u);
}

void test_utf8_movement(void)
{
	TEST("utf8_movement");

	// "héllo" = 6 bytes: 'h'(1) é(2) 'l'(1) 'l'(1) 'o'(1)
	ostd::TextBuffer buf("héllo");
	CHECK_EQ(buf.byteSize(), 6u);
	CHECK_EQ(buf.cursorByteOffset(), 6u);

	buf.moveLeft();  // before 'o'
	CHECK_EQ(buf.cursorByteOffset(), 5u);
	buf.moveLeft();  // before 'l'
	CHECK_EQ(buf.cursorByteOffset(), 4u);
	buf.moveLeft();  // before 'l'
	CHECK_EQ(buf.cursorByteOffset(), 3u);
	buf.moveLeft();  // before 'é' — must skip both bytes.
	CHECK_EQ(buf.cursorByteOffset(), 1u);
	buf.moveLeft();  // before 'h'
	CHECK_EQ(buf.cursorByteOffset(), 0u);
	buf.moveLeft();  // no-op at start
	CHECK_EQ(buf.cursorByteOffset(), 0u);

	// And forward.
	buf.moveRight();
	CHECK_EQ(buf.cursorByteOffset(), 1u);
	buf.moveRight();
	CHECK_EQ(buf.cursorByteOffset(), 3u);  // skipped past 2-byte 'é'
}

void test_utf8_4byte_codepoint(void)
{
	TEST("utf8_4byte_codepoint");

	// "a😀b" — 'a' (1) + 😀 (4) + 'b' (1) = 6 bytes, 3 codepoints
	ostd::TextBuffer buf("a😀b");
	CHECK_EQ(buf.byteSize(), 6u);
	CHECK_EQ(buf.codepointCount(), 3u);

	buf.setCursorByteOffset(6);
	buf.moveLeft();           // before 'b'
	CHECK_EQ(buf.cursorByteOffset(), 5u);
	buf.moveLeft();           // before 😀 — must skip 4 bytes
	CHECK_EQ(buf.cursorByteOffset(), 1u);
	buf.moveLeft();           // before 'a'
	CHECK_EQ(buf.cursorByteOffset(), 0u);

	// Backspace from end deletes 'b' (1 byte), then 😀 (4 bytes), then 'a'.
	buf.setCursorByteOffset(6);
	buf.backspace();
	CHECK_STR_EQ(buf.text(), "a😀");
	CHECK_EQ(buf.cursorByteOffset(), 5u);
	buf.backspace();
	CHECK_STR_EQ(buf.text(), "a");
	CHECK_EQ(buf.cursorByteOffset(), 1u);
	buf.backspace();
	CHECK(buf.empty());
}

void test_clamp_to_codepoint_boundary(void)
{
	TEST("clamp_to_codepoint_boundary");

	// "café" — setting cursor to byte 4 (middle of 'é') should clamp to 3.
	ostd::TextBuffer buf("café");
	buf.setCursorByteOffset(4);  // middle of multibyte sequence
	CHECK_EQ(buf.cursorByteOffset(), 3u);
}

void test_selection_basic(void)
{
	TEST("selection_basic");

	ostd::TextBuffer buf("hello world");
	buf.selectAll();
	CHECK(buf.hasSelection());
	CHECK_EQ(buf.selectionStart(), 0u);
	CHECK_EQ(buf.selectionEnd(), 11u);
	CHECK_STR_EQ(buf.selectedText(), "hello world");

	buf.clearSelection();
	CHECK(!buf.hasSelection());

	buf.selectRange(6, 11);
	CHECK_STR_EQ(buf.selectedText(), "world");

	// Inverted range: anchor > cursor still selects same span.
	buf.selectRange(11, 6);
	CHECK(buf.hasSelection());
	CHECK_EQ(buf.selectionStart(), 6u);
	CHECK_EQ(buf.selectionEnd(), 11u);
	CHECK_STR_EQ(buf.selectedText(), "world");
}

void test_selection_replace_on_insert(void)
{
	TEST("selection_replace_on_insert");

	ostd::TextBuffer buf("hello world");
	buf.selectRange(6, 11);
	buf.insertText("there");
	CHECK_STR_EQ(buf.text(), "hello there");
	CHECK(!buf.hasSelection());
	CHECK_EQ(buf.cursorByteOffset(), 11u);  // end of inserted text
}

void test_selection_replace_on_backspace(void)
{
	TEST("selection_replace_on_backspace");

	ostd::TextBuffer buf("hello world");
	buf.selectRange(5, 11);  // ", world"
	buf.backspace();
	CHECK_STR_EQ(buf.text(), "hello");
	CHECK(!buf.hasSelection());
	CHECK_EQ(buf.cursorByteOffset(), 5u);
}

void test_extend_selection(void)
{
	TEST("extend_selection");

	ostd::TextBuffer buf("hello");
	buf.setCursorByteOffset(0);
	buf.moveRight(true);
	buf.moveRight(true);
	buf.moveRight(true);
	CHECK(buf.hasSelection());
	CHECK_EQ(buf.cursorByteOffset(), 3u);
	CHECK_EQ(buf.anchorByteOffset(), 0u);
	CHECK_STR_EQ(buf.selectedText(), "hel");

	// Move-without-extend collapses to the right edge of the selection
	// (matches macOS / modern Windows / Qt / GTK convention).
	buf.moveRight(false);
	CHECK(!buf.hasSelection());
	CHECK_EQ(buf.cursorByteOffset(), 3u);  // was 4u — collapsed, did not advance further

	// And from a selection, plain moveLeft collapses to the LEFT edge.
	buf.setCursorByteOffset(0);
	buf.moveRight(true);
	buf.moveRight(true);
	CHECK_EQ(buf.cursorByteOffset(), 2u);
	buf.moveLeft(false);
	CHECK(!buf.hasSelection());
	CHECK_EQ(buf.cursorByteOffset(), 0u);  // collapsed to selection start
}

void test_multiline_basic(void)
{
	TEST("multiline_basic");

	ostd::TextBuffer buf("line1\nline2\nline3", true);
	CHECK(buf.isMultiline());
	CHECK_EQ(buf.lineCount(), 3u);
	CHECK_STR_EQ(buf.getLine(0), "line1");
	CHECK_STR_EQ(buf.getLine(1), "line2");
	CHECK_STR_EQ(buf.getLine(2), "line3");
	CHECK_EQ(buf.lineByteStart(0), 0u);
	CHECK_EQ(buf.lineByteStart(1), 6u);   // after "line1\n"
	CHECK_EQ(buf.lineByteStart(2), 12u);
	CHECK_EQ(buf.lineByteLength(0), 5u);  // "line1" without the '\n'
}

void test_multiline_insertion(void)
{
	TEST("multiline_insertion");

	ostd::TextBuffer buf("hello", true);
	buf.setCursorByteOffset(5);
	buf.insertCodepoint('\n');
	buf.insertText("world");
	CHECK_STR_EQ(buf.text(), "hello\nworld");
	CHECK_EQ(buf.lineCount(), 2u);

	// Inserting CRLF should normalize to LF.
	buf.clear();
	buf.insertText("a\r\nb\r\nc");
	CHECK_STR_EQ(buf.text(), "a\nb\nc");
	CHECK_EQ(buf.lineCount(), 3u);
}

void test_singleline_strips_newlines_on_insert(void)
{
	TEST("singleline_strips_newlines_on_insert");

	ostd::TextBuffer buf;
	CHECK(!buf.isMultiline());
	buf.insertText("a\nb\r\nc\rd");
	CHECK_STR_EQ(buf.text(), "a b c d");

	// And insertCodepoint('\n') is rejected.
	buf.clear();
	buf.insertCodepoint('h');
	buf.insertCodepoint('\n');
	buf.insertCodepoint('i');
	CHECK_STR_EQ(buf.text(), "hi");
}

void test_setMultiline_strip(void)
{
	TEST("setMultiline_strip");

	// Constructed multiline, switching to single-line strips newlines.
	ostd::TextBuffer buf("a\nb\nc", true);
	CHECK_EQ(buf.lineCount(), 3u);
	buf.setMultiline(false);
	CHECK_STR_EQ(buf.text(), "a b c");
	CHECK_EQ(buf.lineCount(), 1u);
}

void test_vertical_movement(void)
{
	TEST("vertical_movement");

	// Three lines, each 5 chars: "abcde\nfghij\nklmno"
	ostd::TextBuffer buf("abcde\nfghij\nklmno", true);
	buf.setCursorByteOffset(0);
	CHECK_EQ(buf.cursorLineCol().y, 0);
	CHECK_EQ(buf.cursorLineCol().x, 0);

	buf.moveDown();
	CHECK_EQ(buf.cursorLineCol().y, 1);
	CHECK_EQ(buf.cursorLineCol().x, 0);
	CHECK_EQ(buf.cursorByteOffset(), 6u);  // 'f'

	buf.moveDown();
	CHECK_EQ(buf.cursorLineCol().y, 2);

	// At the last line, moveDown jumps to end of document.
	buf.moveDown();
	CHECK_EQ(buf.cursorByteOffset(), buf.byteSize());
}

void test_desired_column_sticky(void)
{
	TEST("desired_column_sticky");

	// Long, short, long: cursor on column 4 of line 0, move down to line 1
	// (only 2 chars long, lands at end), then back up — should remember col 4.
	ostd::TextBuffer buf("abcdef\nxy\nabcdef", true);
	buf.setCursorLineCol({ 4, 0 });
	CHECK_EQ(buf.cursorLineCol().x, 4);

	buf.moveDown();
	// Line 1 is "xy" (2 codepoints), so we land at col 2 visually but remember 4.
	CHECK_EQ(buf.cursorLineCol().y, 1);
	CHECK_EQ(buf.cursorLineCol().x, 2);

	buf.moveDown();
	// Back on a long line: should restore column 4.
	CHECK_EQ(buf.cursorLineCol().y, 2);
	CHECK_EQ(buf.cursorLineCol().x, 4);

	// Horizontal movement resets the desired column.
	buf.moveLeft();
	CHECK_EQ(buf.cursorLineCol().x, 3);
	buf.moveUp();
	// Now from col 3 on a 2-char line, we land at col 2. But on next moveUp,
	// we should NOT remember col 4 anymore — should remember col 3.
	CHECK_EQ(buf.cursorLineCol().y, 1);
	buf.moveUp();
	CHECK_EQ(buf.cursorLineCol().y, 0);
	CHECK_EQ(buf.cursorLineCol().x, 3);
}

void test_line_start_end(void)
{
	TEST("line_start_end");

	ostd::TextBuffer buf("hello\nworld", true);
	buf.setCursorByteOffset(8);  // middle of "world"
	buf.moveLineStart();
	CHECK_EQ(buf.cursorByteOffset(), 6u);
	buf.moveLineEnd();
	CHECK_EQ(buf.cursorByteOffset(), 11u);

	buf.setCursorByteOffset(2);  // middle of "hello"
	buf.moveLineEnd();
	CHECK_EQ(buf.cursorByteOffset(), 5u);  // before the '\n'
	buf.moveLineStart();
	CHECK_EQ(buf.cursorByteOffset(), 0u);
}

void test_word_movement(void)
{
	TEST("word_movement");

	ostd::TextBuffer buf("hello, world  foo");
	//  Indices: h=0 ... o=4, ','=5, ' '=6, w=7 ... d=11, ' '=12, ' '=13, f=14 ... o=16
	buf.setCursorByteOffset(0);
	buf.moveWordRight();
	// Skip "hello", then skip ", " whitespace+punct run... actually let's
	// check what the implementation does: it skips the run of "Word" then
	// any following whitespace. ',' is Punctuation, not Word, so the first
	// run is "hello" (5 chars), then we skip Whitespace (the comma is NOT
	// whitespace, so we stop).  Result: cursor at 5 (on the comma).
	CHECK_EQ(buf.cursorByteOffset(), 5u);

	buf.moveWordRight();
	// From the comma: classify(',') == Punctuation. Skip Punct run = just the
	// comma. Then skip Whitespace run = the space. Land on 'w'.
	CHECK_EQ(buf.cursorByteOffset(), 7u);

	buf.moveWordRight();
	// Skip "world", then skip the two spaces. Land on 'f'.
	CHECK_EQ(buf.cursorByteOffset(), 14u);

	// Reverse direction.
	buf.setCursorByteOffset(17);  // end of buffer
	buf.moveWordLeft();
	// Skip whitespace-to-the-left first (none — we're after 'o'), then skip
	// the Word run "foo". Land on 'f' (byte 14).
	CHECK_EQ(buf.cursorByteOffset(), 14u);

	buf.moveWordLeft();
	// Skip the two spaces (whitespace run), then skip the Word run "world".
	CHECK_EQ(buf.cursorByteOffset(), 7u);
}

void test_word_backspace_and_delete(void)
{
	TEST("word_backspace_and_delete");

	ostd::TextBuffer buf("hello world foo");
	buf.setCursorByteOffset(15);
	buf.backspaceWord();
	CHECK_STR_EQ(buf.text(), "hello world ");
	CHECK_EQ(buf.cursorByteOffset(), 12u);

	buf.backspaceWord();
	CHECK_STR_EQ(buf.text(), "hello ");
	CHECK_EQ(buf.cursorByteOffset(), 6u);

	buf.setCursorByteOffset(0);
	buf.deleteWord();
	CHECK_STR_EQ(buf.text(), "");
}

void test_delete_current_line(void)
{
	TEST("delete_current_line");

	ostd::TextBuffer buf("a\nb\nc\nd", true);
	buf.setCursorLineCol({ 0, 1 });
	buf.deleteCurrentLine();
	CHECK_STR_EQ(buf.text(), "a\nc\nd");
	CHECK_EQ(buf.lineCount(), 3u);

	// Last line: deletes through end of buffer (no trailing newline to consume).
	buf.setCursorLineCol({ 0, 2 });
	buf.deleteCurrentLine();
	CHECK_STR_EQ(buf.text(), "a\nc\n");
}

void test_callbacks_fire(void)
{
	TEST("callbacks_fire");

	ostd::TextBuffer buf;
	u32 changed = 0;
	u32 cursor_moved = 0;
	buf.setOnChanged([&](const ostd::TextBuffer&) { changed++; });
	buf.setOnCursorMoved([&](const ostd::TextBuffer&) { cursor_moved++; });

	buf.insertText("hello");
	CHECK_EQ(changed, 1u);
	CHECK(cursor_moved >= 1u);  // exact count is impl-detail; just verify it fired

	const u32 cm_before = cursor_moved;
	buf.setCursorByteOffset(0);
	CHECK_EQ(changed, 1u);  // movement does NOT count as change
	CHECK(cursor_moved > cm_before);

	// No-op moves should not fire.
	const u32 cm_pre = cursor_moved;
	buf.setCursorByteOffset(0);
	CHECK_EQ(cursor_moved, cm_pre);
}

void test_cursor_clamp_after_setText(void)
{
	TEST("cursor_clamp_after_setText");

	ostd::TextBuffer buf("hello world");
	buf.setCursorByteOffset(11);

	buf.setText("hi");
	CHECK_EQ(buf.cursorByteOffset(), 2u);  // setText parks cursor at end
	CHECK(!buf.hasSelection());
}

void test_replace_range_via_selection_across_lines(void)
{
	TEST("replace_range_via_selection_across_lines");

	ostd::TextBuffer buf("aaa\nbbb\nccc", true);
	// Select from middle of line 0 to middle of line 2.
	buf.selectRange(2, 9);
	buf.insertText("X");
	CHECK_STR_EQ(buf.text(), "aaXcc");
	CHECK_EQ(buf.lineCount(), 1u);
	CHECK_EQ(buf.cursorByteOffset(), 3u);
}

void test_no_op_edits_dont_fire_changed(void)
{
	TEST("no_op_edits_dont_fire_changed");

	ostd::TextBuffer buf("hello");
	u32 changed = 0;
	buf.setOnChanged([&](const ostd::TextBuffer&) { changed++; });

	// Backspace at start of buffer is a no-op.
	buf.setCursorByteOffset(0);
	buf.backspace();
	CHECK_EQ(changed, 0u);

	// Delete-forward at end is a no-op.
	buf.setCursorByteOffset(buf.byteSize());
	buf.deleteForward();
	CHECK_EQ(changed, 0u);
}

void test_utf8_word_movement(void)
{
	TEST("utf8_word_movement");

	// "héllo wörld" — non-ASCII letters classified as Word, so the buffer
	// should treat "héllo" and "wörld" as single word runs.
	// Bytes: h(1) é(2) l(1) l(1) o(1) ' '(1) w(1) ö(2) r(1) l(1) d(1) = 13 bytes
	ostd::TextBuffer buf("héllo wörld");
	CHECK_EQ(buf.byteSize(), 13u);

	buf.setCursorByteOffset(0);
	buf.moveWordRight();
	CHECK_EQ(buf.cursorByteOffset(), 7u);  // end of "héllo", on the space
	buf.moveWordRight();
	// Skip space, skip "wörld" → end of buffer
	CHECK_EQ(buf.cursorByteOffset(), 13u);
}

void test_select_all_on_empty(void)
{
	TEST("select_all_on_empty");

	ostd::TextBuffer buf;
	buf.selectAll();
	CHECK(!buf.hasSelection());  // 0..0 is no selection
}

void test_line_count_with_trailing_newline(void)
{
	TEST("line_count_with_trailing_newline");

	// "a\n" — is this 1 line or 2? Convention varies. Our impl counts the
	// position after the trailing '\n' as a new (empty) line, so this is 2.
	ostd::TextBuffer buf("a\n", true);
	CHECK_EQ(buf.lineCount(), 2u);
	CHECK_STR_EQ(buf.getLine(0), "a");
	CHECK_STR_EQ(buf.getLine(1), "");
}

}  // anonymous namespace


// ============================================================
//  Driver
// ============================================================

int main(void)
{
	g_out.fg(ostd::ConsoleColors::BrightCyan)
		 .p("Running TextBuffer tests...").nl().reset();

	test_construction_and_basic();
	test_setText_and_clear();
	test_simple_insert_and_backspace();
	test_insert_text_at_position();
	test_utf8_backspace();
	test_utf8_movement();
	test_utf8_4byte_codepoint();
	test_clamp_to_codepoint_boundary();
	test_selection_basic();
	test_selection_replace_on_insert();
	test_selection_replace_on_backspace();
	test_extend_selection();
	test_multiline_basic();
	test_multiline_insertion();
	test_singleline_strips_newlines_on_insert();
	test_setMultiline_strip();
	test_vertical_movement();
	test_desired_column_sticky();
	test_line_start_end();
	test_word_movement();
	test_word_backspace_and_delete();
	test_delete_current_line();
	test_callbacks_fire();
	test_cursor_clamp_after_setText();
	test_replace_range_via_selection_across_lines();
	test_no_op_edits_dont_fire_changed();
	test_utf8_word_movement();
	test_select_all_on_empty();
	test_line_count_with_trailing_newline();

	g_out.nl().nl();

	if (g_stats.failed == 0) {
		g_out.fg(ostd::ConsoleColors::BrightGreen)
			 .p("ALL ").p(g_stats.passed).p(" CHECKS PASSED").reset().nl();
		return 0;
	}
	else {
		g_out.fg(ostd::ConsoleColors::BrightRed)
			 .p(g_stats.failed).p(" FAILURES out of ").p(g_stats.total).p(" checks:")
			 .reset().nl();
		for (const auto& f : g_stats.failures)
			g_out.p(f).nl();
		return 1;
	}
}
