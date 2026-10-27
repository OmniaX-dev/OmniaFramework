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

#include <ogfx/gui/widgets/Scrollbar.hpp>
#include <ostd/io/IOHandlers.hpp>
#include <ostd/string/TextStyleParser.hpp>
#include <deque>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// RichTextBuffer — lines of text where every character has its own foreground
		// and background color: the screen buffer of a terminal, without a fixed size.
		//
		// Text is written at a cursor (overwriting what is there, like a terminal),
		// '\n' starts a new line and '\t' moves to the next tab stop. A foreground with
		// alpha 0 means "the view's text color", a background with alpha 0 means "none".
		// Every line can carry a user value (e.g. the address of a disassembly line).
		//
		// setMaxLines() turns the buffer into a ring (the oldest lines are dropped),
		// for logs. getVersion() changes on every edit, so views know when to update.
		// Columns are codepoints (UTF-8 text is decoded).
		// ============================================================================
		class RichTextBuffer
		{
			// Colors are packed RGBA (see packColor()): ostd::Color is too big to have two per
			// character.
			public: struct Cell
			{
				u32 codepoint { ' ' };
				u32 foreground { 0 };
				u32 background { 0 };
			};
			public: struct Line
			{
				stdvec<Cell> cells;
				i64 userData { -1 };
			};

			public:
				void clear(void);
				// Writes at the cursor and moves it (see above).
				void write(const String& text, const Color& foreground = { 0, 0, 0, 0 }, const Color& background = { 0, 0, 0, 0 });
				// Same, with per-byte colors (TextStyleParser::tStyledString): the color of a
				// multi-byte character is the one of its first byte.
				void write(const ostd::TextStyleParser::tStyledString& styled);
				void newLine(void);
				void setCursor(i32 column, i32 line);
				inline i32 getCursorColumn(void) const { return m_cursorColumn; }
				inline i32 getCursorLine(void) const { return m_cursorLine; }
				void setTabWidth(u8 width);
				inline u8 getTabWidth(void) const { return m_tabWidth; }
				// 0 = unlimited.
				void setMaxLines(i32 maxLines);
				inline i32 getMaxLines(void) const { return m_maxLines; }

				inline i32 getLineCount(void) const { return cast<i32>(m_lines.size()); }
				inline bool isEmpty(void) const { return m_lines.empty(); }
				// nullptr if out of range.
				const Line* getLine(i32 index) const;
				bool setLineUserData(i32 index, i64 userData);
				i64 getLineUserData(i32 index) const;
				// First line (from `from`, wrapping around) whose user value is `userData`; -1 if none.
				i32 findLine(i64 userData, i32 from = 0) const;
				// Length of the longest line, in columns.
				i32 getLongestLine(void) const;
				// The text of lines [first, last], '\n' separated.
				String getText(i32 first = 0, i32 last = -1) const;
				inline u64 getVersion(void) const { return m_version; }
				// Lines dropped from the top so far (maxLines): views use it to keep their scroll.
				inline u64 getDroppedLineCount(void) const { return m_droppedLines; }

				static constexpr u32 packColor(u8 r, u8 g, u8 b, u8 a) { return (cast<u32>(r) << 24) | (cast<u32>(g) << 16) | (cast<u32>(b) << 8) | cast<u32>(a); }
				static u32 packColor(const Color& color);
				static Color unpackColor(u32 color);
				static constexpr u8 packedAlpha(u32 color) { return cast<u8>(color & 0xFF); }

			private:
				Line& line_at_cursor(void);
				void put(u32 codepoint, u32 foreground, u32 background);
				void trim_to_max(void);

			private:
				std::deque<Line> m_lines;
				i32 m_cursorColumn { 0 };
				i32 m_cursorLine { 0 };
				u8 m_tabWidth { 4 };
				i32 m_maxLines { 0 };
				u64 m_version { 0 };
				u64 m_droppedLines { 0 };
				mutable i32 m_longestLine { 0 };
				mutable u64 m_longestLineVersion { (u64)-1 };
		};

		// ============================================================================
		// RichTextView — a scrollable, read-only view of a RichTextBuffer, drawn with a
		// monospace font (the renderer's default mono font).
		//
		// The view owns a buffer (getBuffer()), but can show any other one (setBuffer(),
		// e.g. to switch between documents without rebuilding them). The easiest way to
		// fill it is a RichTextOutputHandler: the usual fg()/p()/nl()/pStyled() calls,
		// including OmniaFramework's styled strings ("[@@ style foreground:red]...[@@/]")
		// and RegexRichString.
		//
		// Mouse: the wheel scrolls, hovering highlights the line, clicking calls the
		// line callback (with the button and whether it was a double click). Dragging
		// with the left button selects lines; Ctrl+C copies them (or everything, with
		// nothing selected). Keyboard (when focused): arrows / PageUp / PageDown / Home /
		// End scroll.
		//
		// The line decorator is called for every visible line, before its text is
		// drawn: it can draw backgrounds and markers (current line, breakpoints, ...)
		// without touching the text.
		//
		// followTail: while the view is scrolled to the bottom, new lines keep it there
		// (for logs).
		//
		// Theme category: "richTextView" (fontSize, textColor, backgroundColor,
		// hoverColor, selectionColor, linePadding, ...).
		// ============================================================================
		class RichTextView : public ScrollableWidget
		{
			public: using LineCallback = std::function<void(RichTextView& view, i32 line, i32 column, MouseEventData::eButton button, bool doubleClick)>;
			public: using LineDecorator = std::function<void(RichTextView& view, BasicRenderer2D& gfx, i32 line, const Rectangle& lineBounds)>;

			public:
				RichTextView(Window& window);
				inline RichTextBuffer& getBuffer(void) { return *m_buffer; }
				inline const RichTextBuffer& getBuffer(void) const { return *m_buffer; }
				// nullptr = the view's own buffer. The buffer must outlive its use by the view.
				void setBuffer(RichTextBuffer* buffer);
				inline bool isShowingOwnBuffer(void) const { return m_buffer == &m_ownBuffer; }

				// ================================== VIEW ==================================
				// Scrolls so the line is visible (centered = in the middle of the view).
				void scrollToLine(i32 line, bool centered = false);
				void ensureLineVisible(i32 line);
				void scrollToTop(void);
				void scrollToBottom(void);
				// First and last line (partially) visible.
				i32 getFirstVisibleLine(void) const;
				i32 getLastVisibleLine(void) const;
				// Line / column under a window position; -1 if none.
				i32 getLineAt(const Vec2& position) const;
				i32 getColumnAt(const Vec2& position) const;
				// Global bounds of a line (as wide as the view).
				Rectangle getLineBounds(i32 line) const;
				// Size of one character cell.
				Vec2 getCharSize(void) const;
				// Whole columns / rows that fit in the view.
				i32 getVisibleColumns(void) const;
				i32 getVisibleRows(void) const;
				inline i32 getHoveredLine(void) const { return m_hoveredLine; }

				// =============================== SELECTION ===============================
				void selectLines(i32 first, i32 last);
				inline void clearSelection(void) { m_selectionStart = m_selectionEnd = -1; }
				inline bool hasSelection(void) const { return m_selectionStart >= 0; }
				inline i32 getSelectionFirst(void) const { return std::min(m_selectionStart, m_selectionEnd); }
				inline i32 getSelectionLast(void) const { return std::max(m_selectionStart, m_selectionEnd); }
				// The selected lines (or everything) to the clipboard.
				void copyToClipboard(void);

				// ================================ SETTINGS ================================
				inline void setLineCallback(LineCallback callback) { m_lineCallback = std::move(callback); }
				inline void setLineDecorator(LineDecorator decorator) { m_lineDecorator = std::move(decorator); }
				OSTD_BOOL_PARAM_GETSET_E(FollowTail, m_followTail);
				OSTD_BOOL_PARAM_GETSET_E(HoverHighlight, m_hoverHighlight);
				OSTD_BOOL_PARAM_GETSET_E(Selection, m_selectionEnabled);
				OSTD_PARAM_GETSET(Color, HoverColor, m_hoverColor);
				OSTD_PARAM_GETSET(Color, SelectionColor, m_selectionColor);
				// Extra vertical space around every line, in pixels.
				OSTD_PARAM_GETSET(f32, LinePadding, m_linePadding);

				// ================================ OVERRIDES ================================
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void afterDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;
				void onMousePressed(const Event& event) override;
				void onMouseReleased(const Event& event) override;
				void onMouseMoved(const Event& event) override;
				void onMouseDragged(const Event& event) override;
				void onMouseExited(const Event& event) override;
				void onKeyPressed(const Event& event) override;
				Rectangle getContentExtents(void) const override;

			private:
				void update_metrics(BasicRenderer2D& gfx);
				f32 line_height(void) const;
				void draw_line(BasicRenderer2D& gfx, const RichTextBuffer::Line& line, const Vec2& position);
				bool is_at_bottom(void) const;

			private:
				RichTextBuffer m_ownBuffer;
				RichTextBuffer* m_buffer { &m_ownBuffer };
				u64 m_seenVersion { (u64)-1 };
				u64 m_seenDroppedLines { 0 };
				i32 m_seenLineCount { 0 };
				const RichTextBuffer* m_seenBuffer { nullptr };

				Vec2 m_charSize { 9, 18 };
				i32 m_metricsFontSize { 0 };
				f32 m_linePadding { 2.0f };

				LineCallback m_lineCallback { nullptr };
				LineDecorator m_lineDecorator { nullptr };
				bool m_followTail { false };
				bool m_wasAtBottom { true };
				bool m_hoverHighlight { true };
				bool m_selectionEnabled { true };
				Color m_hoverColor { 255, 255, 255, 14 };
				Color m_selectionColor { 0, 117, 199, 110 };

				i32 m_hoveredLine { -1 };
				i32 m_selectionStart { -1 };
				i32 m_selectionEnd { -1 };
				bool m_selecting { false };
				u64 m_lastClickMs { 0 };
				i32 m_lastClickLine { -1 };
				// Line to scroll to once the scrollbars know the content size.
				i32 m_pendingScrollLine { -1 };
				bool m_pendingScrollCentered { false };
		};

		// ============================================================================
		// RichTextOutputHandler — an ostd::OutputHandlerBase that writes into a
		// RichTextBuffer, so code written for the console (out.fg(...).p(...).nl(),
		// pStyled(), RegexRichString, ...) can fill a RichTextView unchanged.
		//
		// Console colors (fg("red"), ConsoleColors::Blue, "[@@ style foreground:cyan]")
		// go through a palette, so they can look good on any background: the default
		// one is a terminal-like scheme for dark backgrounds; setConsoleColor() changes
		// an entry. Plain Colors are used as they are.
		//
		// The default foreground is "the view's text color" (and no background), so the
		// text follows the theme.
		// ============================================================================
		class RichTextOutputHandler : public ostd::OutputHandlerBase
		{
			public:
				inline RichTextOutputHandler(void) { loadDefaultPalette(); }
				inline RichTextOutputHandler(RichTextBuffer& buffer) : m_buffer(&buffer) { loadDefaultPalette(); }
				inline RichTextOutputHandler(RichTextView& view) : m_buffer(&view.getBuffer()) { loadDefaultPalette(); }
				inline void attach(RichTextBuffer& buffer) { m_buffer = &buffer; }
				inline void attach(RichTextView& view) { m_buffer = &view.getBuffer(); }
				inline RichTextBuffer* getBuffer(void) const { return m_buffer; }

				// Console color name (lowercase, as in ostd::ConsoleColors) -> color.
				void setConsoleColor(const String& name, const Color& color);
				Color getConsoleColor(const String& name) const;
				void loadDefaultPalette(void);
				// Used for "white", "brightgray", ... too, unless changed; alpha 0 = the view's text color.
				inline void setDefaultForeground(const Color& color) { m_defaultForeground = color; }
				inline void setDefaultBackground(const Color& color) { m_defaultBackground = color; }

				RichTextOutputHandler& bg(const Color& color) override;
				RichTextOutputHandler& bg(const ostd::ConsoleColors::tConsoleColor& color) override;
				RichTextOutputHandler& bg(const String& color) override;
				RichTextOutputHandler& fg(const Color& color) override;
				RichTextOutputHandler& fg(const ostd::ConsoleColors::tConsoleColor& color) override;
				RichTextOutputHandler& fg(const String& color) override;

				RichTextOutputHandler& pChar(char c) override;
				RichTextOutputHandler& pStyled(const String& styled) override;
				RichTextOutputHandler& pStyled(const ostd::TextStyleParser::tStyledString& styled) override;
				RichTextOutputHandler& pStyled(ostd::TextStyleBuilder::IRichStringBase& styled) override;
				RichTextOutputHandler& pObject(const ostd::Object& bo) override;

				RichTextOutputHandler& p(const String& se) override;
				RichTextOutputHandler& p(u8 i) override;
				RichTextOutputHandler& p(i8 i) override;
				RichTextOutputHandler& p(u16 i) override;
				RichTextOutputHandler& p(i16 i) override;
				RichTextOutputHandler& p(u32 i) override;
				RichTextOutputHandler& p(i32 i) override;
				RichTextOutputHandler& p(u64 i) override;
				RichTextOutputHandler& p(i64 i) override;
				RichTextOutputHandler& p(f32 f, u8 precision = 0) override;
				RichTextOutputHandler& p(f64 f, u8 precision = 0) override;

				RichTextOutputHandler& nl(void) override;
				RichTextOutputHandler& tab(void) override;
				RichTextOutputHandler& flush(void) override;
				// Empties the buffer.
				RichTextOutputHandler& clear(void) override;
				// Back to the default colors.
				RichTextOutputHandler& reset(void) override;

				RichTextOutputHandler& xy(IPoint position) override;
				RichTextOutputHandler& xy(i32 x, i32 y) override;
				RichTextOutputHandler& x(i32 x) override;
				RichTextOutputHandler& y(i32 y) override;

				IPoint getCursorPosition(void) override;
				void getCursorPosition(i32& outX, i32& outY) override;
				i32 getCursorX(void) override;
				i32 getCursorY(void) override;

				// The visible size of the attached view (setConsoleSize()), or 0 x 0.
				void getConsoleSize(i32& outColumns, i32& outRows) override;
				IPoint getConsoleSize(void) override;
				inline void setConsoleSize(const IPoint& size) { m_consoleSize = size; }

			private:
				Color resolve(const ostd::TextStyleParser::tColor& color, bool background) const;
				Color resolve_name(const String& name, const Color& fallback, bool background) const;
				void write(const String& text);

			private:
				RichTextBuffer* m_buffer { nullptr };
				stdumap<String, Color> m_palette;
				Color m_defaultForeground { 0, 0, 0, 0 };
				Color m_defaultBackground { 0, 0, 0, 0 };
				Color m_foreground { 0, 0, 0, 0 };
				Color m_background { 0, 0, 0, 0 };
				IPoint m_consoleSize { 0, 0 };
		};
	}
}
