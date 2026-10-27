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

#include "RichTextView.hpp"
#include "../Window.hpp"
#include "../../utils/Keycodes.hpp"
#include <chrono>

namespace ogfx
{
	namespace gui
	{
		static u64 now_ms(void)
		{
			using namespace std::chrono;
			return cast<u64>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
		}

		static constexpr u64 DoubleClickMs { 400 };

		static bool same_color(const Color& a, const Color& b)
		{
			return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
		}



		// ===================================================== RichTextBuffer =====================================================
		void RichTextBuffer::clear(void)
		{
			m_lines.clear();
			m_cursorColumn = 0;
			m_cursorLine = 0;
			m_version++;
		}

		u32 RichTextBuffer::packColor(const Color& color)
		{
			return packColor(color.r, color.g, color.b, color.a);
		}

		Color RichTextBuffer::unpackColor(u32 color)
		{
			return Color(cast<u8>(color >> 24), cast<u8>((color >> 16) & 0xFF), cast<u8>((color >> 8) & 0xFF), cast<u8>(color & 0xFF));
		}

		void RichTextBuffer::write(const String& text, const Color& foreground, const Color& background)
		{
			const u32 fg = packColor(foreground);
			const u32 bg = packColor(background);
			const auto& str = text.cpp_str();
			u32 pos = 0;
			while (pos < str.size())
			{
				const u8 len = String::utf8::codepoint_length(str, pos);
				const auto cps = String::utf8::decode(str.substr(pos, len));
				pos += std::max<u8>(len, 1);
				if (!cps.empty())
					put(cps[0], fg, bg);
			}
		}

		void RichTextBuffer::write(const ostd::TextStyleParser::tStyledString& styled)
		{
			if (!styled.validate())
				return;
			const auto& str = styled.text.cpp_str();
			u32 pos = 0;
			while (pos < str.size())
			{
				const u8 len = String::utf8::codepoint_length(str, pos);
				const auto cps = String::utf8::decode(str.substr(pos, len));
				if (!cps.empty())
					put(cps[0], packColor(styled.foregroundColors[pos].fullColor), packColor(styled.backgroundColors[pos].fullColor));
				pos += std::max<u8>(len, 1);
			}
		}

		void RichTextBuffer::newLine(void)
		{
			m_cursorLine++;
			m_cursorColumn = 0;
			m_version++;
			trim_to_max();
		}

		void RichTextBuffer::setCursor(i32 column, i32 line)
		{
			m_cursorColumn = std::max(0, column);
			m_cursorLine = std::max(0, line);
		}

		void RichTextBuffer::setTabWidth(u8 width)
		{
			m_tabWidth = std::max<u8>(width, 1);
		}

		void RichTextBuffer::setMaxLines(i32 maxLines)
		{
			m_maxLines = std::max(0, maxLines);
			trim_to_max();
		}

		const RichTextBuffer::Line* RichTextBuffer::getLine(i32 index) const
		{
			if (index < 0 || index >= getLineCount())
				return nullptr;
			return &m_lines[index];
		}

		bool RichTextBuffer::setLineUserData(i32 index, i64 userData)
		{
			if (index < 0)
				return false;
			// Like writing: the line is created if needed.
			while (getLineCount() <= index)
				m_lines.emplace_back();
			m_lines[index].userData = userData;
			return true;
		}

		i64 RichTextBuffer::getLineUserData(i32 index) const
		{
			const Line* line = getLine(index);
			return line != nullptr ? line->userData : -1;
		}

		i32 RichTextBuffer::findLine(i64 userData, i32 from) const
		{
			const i32 count = getLineCount();
			if (count == 0)
				return -1;
			from = std::clamp(from, 0, count - 1);
			for (i32 i = 0; i < count; i++)
			{
				const i32 index = (from + i) % count;
				if (m_lines[index].userData == userData)
					return index;
			}
			return -1;
		}

		i32 RichTextBuffer::getLongestLine(void) const
		{
			if (m_longestLineVersion == m_version)
				return m_longestLine;
			m_longestLine = 0;
			for (const auto& line : m_lines)
				m_longestLine = std::max(m_longestLine, cast<i32>(line.cells.size()));
			m_longestLineVersion = m_version;
			return m_longestLine;
		}

		String RichTextBuffer::getText(i32 first, i32 last) const
		{
			const i32 count = getLineCount();
			if (last < 0 || last >= count)
				last = count - 1;
			first = std::max(0, first);
			stdvec<u32> codepoints;
			for (i32 i = first; i <= last; i++)
			{
				for (const auto& cell : m_lines[i].cells)
					codepoints.push_back(cell.codepoint);
				if (i < last)
					codepoints.push_back('\n');
			}
			return String::utf8::encode(codepoints);
		}

		RichTextBuffer::Line& RichTextBuffer::line_at_cursor(void)
		{
			while (getLineCount() <= m_cursorLine)
				m_lines.emplace_back();
			return m_lines[m_cursorLine];
		}

		void RichTextBuffer::put(u32 codepoint, u32 foreground, u32 background)
		{
			if (codepoint == '\n')
			{
				// A line that only ends exists too (an empty line, in a log).
				line_at_cursor();
				newLine();
				return;
			}
			if (codepoint == '\r')
			{
				m_cursorColumn = 0;
				return;
			}
			if (codepoint == '\t')
			{
				const i32 spaces = m_tabWidth - (m_cursorColumn % m_tabWidth);
				for (i32 i = 0; i < spaces; i++)
					put(' ', foreground, background);
				return;
			}
			if (codepoint < 0x20 || codepoint == 0x7F)
				return;
			Line& line = line_at_cursor();
			if (cast<i32>(line.cells.size()) < m_cursorColumn)
				line.cells.resize(m_cursorColumn);
			if (cast<i32>(line.cells.size()) == m_cursorColumn)
				line.cells.push_back({ codepoint, foreground, background });
			else
				line.cells[m_cursorColumn] = { codepoint, foreground, background };
			m_cursorColumn++;
			m_version++;
		}

		void RichTextBuffer::trim_to_max(void)
		{
			if (m_maxLines <= 0)
				return;
			while (getLineCount() > m_maxLines)
			{
				m_lines.pop_front();
				m_cursorLine = std::max(0, m_cursorLine - 1);
				m_droppedLines++;
				m_version++;
			}
		}
		// ===================================================== RichTextBuffer =====================================================





		// ===================================================== RichTextView =====================================================
		RichTextView::RichTextView(Window& window) : ScrollableWidget(window)
		{
			setTypeName("ogfx::gui::RichTextView");
			setStylesheetCategoryName("richTextView");
			enableBackground();
			enableBorder();
			setBackgroundColor({ 18, 20, 18 });
			setBorderColor({ 50, 50, 50 });
			setBorderWidth(1);
			setTextColor({ 210, 215, 210 });
			setFontSize(16);
			setPadding({ 6, 4, 6, 4 });
			setScrollSpeed(1.2f);
			reloadTheme();
			validate();
		}

		void RichTextView::setBuffer(RichTextBuffer* buffer)
		{
			m_buffer = (buffer != nullptr ? buffer : &m_ownBuffer);
			clearSelection();
			m_hoveredLine = -1;
		}

		void RichTextView::scrollToLine(i32 line, bool centered)
		{
			m_pendingScrollLine = std::max(0, line);
			m_pendingScrollCentered = centered;
			updateScrollbarsSize();
			const f32 lh = line_height();
			f32 y = cast<f32>(m_pendingScrollLine) * lh;
			if (centered)
				y -= (getContentBounds().h - lh) * 0.5f;
			setScrollOffset({ getScrollOffset().x, -y });
		}

		void RichTextView::ensureLineVisible(i32 line)
		{
			if (line < 0)
				return;
			const f32 lh = line_height();
			const f32 top = -getScrollOffset().y;
			const f32 viewH = getContentBounds().h;
			const f32 lineY = cast<f32>(line) * lh;
			if (lineY < top)
				scrollToLine(line);
			else if (lineY + lh > top + viewH)
			{
				updateScrollbarsSize();
				setScrollOffset({ getScrollOffset().x, -(lineY + lh - viewH) });
			}
		}

		void RichTextView::scrollToTop(void)
		{
			scrollToLine(0);
		}

		void RichTextView::scrollToBottom(void)
		{
			updateScrollbarsSize();
			setScrollOffset({ getScrollOffset().x, -getScrollArea().h });
			m_pendingScrollLine = -1;
		}

		i32 RichTextView::getFirstVisibleLine(void) const
		{
			const f32 lh = line_height();
			return lh > 0 ? cast<i32>(std::floor(-getScrollOffset().y / lh)) : 0;
		}

		i32 RichTextView::getLastVisibleLine(void) const
		{
			const f32 lh = line_height();
			if (lh <= 0)
				return 0;
			const i32 last = cast<i32>(std::floor((-getScrollOffset().y + getContentBounds().h) / lh));
			return std::min(last, m_buffer->getLineCount() - 1);
		}

		i32 RichTextView::getLineAt(const Vec2& position) const
		{
			const Rectangle view = getGlobalContentBounds();
			if (!view.contains(position, true))
				return -1;
			const f32 lh = line_height();
			if (lh <= 0)
				return -1;
			const i32 line = cast<i32>(std::floor((position.y - view.y - getScrollOffset().y) / lh));
			return (line >= 0 && line < m_buffer->getLineCount()) ? line : -1;
		}

		i32 RichTextView::getColumnAt(const Vec2& position) const
		{
			const Rectangle view = getGlobalContentBounds();
			if (!view.contains(position, true) || m_charSize.x <= 0)
				return -1;
			return cast<i32>(std::floor((position.x - view.x - getScrollOffset().x) / m_charSize.x));
		}

		Rectangle RichTextView::getLineBounds(i32 line) const
		{
			const Rectangle view = getGlobalContentBounds();
			const f32 lh = line_height();
			return { view.x, view.y + getScrollOffset().y + cast<f32>(line) * lh, view.w, lh };
		}

		Vec2 RichTextView::getCharSize(void) const
		{
			return m_charSize;
		}

		i32 RichTextView::getVisibleColumns(void) const
		{
			return m_charSize.x > 0 ? cast<i32>(getContentBounds().w / m_charSize.x) : 0;
		}

		i32 RichTextView::getVisibleRows(void) const
		{
			const f32 lh = line_height();
			return lh > 0 ? cast<i32>(getContentBounds().h / lh) : 0;
		}

		void RichTextView::selectLines(i32 first, i32 last)
		{
			const i32 count = m_buffer->getLineCount();
			if (count == 0)
			{
				clearSelection();
				return;
			}
			m_selectionStart = std::clamp(first, 0, count - 1);
			m_selectionEnd = std::clamp(last, 0, count - 1);
		}

		void RichTextView::copyToClipboard(void)
		{
			String text = hasSelection() ? m_buffer->getText(getSelectionFirst(), getSelectionLast()) : m_buffer->getText();
			SDL_SetClipboardText(text.c_str());
		}

		void RichTextView::applyTheme(const ostd::Stylesheet& theme)
		{
			setHoverColor(getThemeValue<Color>(theme, "hoverColor", getHoverColor()));
			setSelectionColor(getThemeValue<Color>(theme, "selectionColor", getSelectionColor()));
			setLinePadding(getThemeValue<f32>(theme, "linePadding", getLinePadding()));
			setScrollSpeed(getThemeValue<f32>(theme, "scrollSpeed", getScrollSpeed()));
			setScrollSmoothFactor(getThemeValue<f32>(theme, "scrollSmoothFactor", getScrollSmoothFactor()));
		}

		void RichTextView::update_metrics(BasicRenderer2D& gfx)
		{
			if (m_metricsFontSize == getFontSize() || !gfx.isValid())
				return;
			TTF_Font* font = gfx.getDefaultMonoFont(getFontSize());
			if (font == nullptr)
				return;
			const Vec2 size = gfx.getStringDimensions("MMMMMMMMMM", getFontSize(), font);
			if (size.x <= 0)
				return;
			m_charSize = { size.x / 10.0f, size.y };
			m_metricsFontSize = getFontSize();
		}

		f32 RichTextView::line_height(void) const
		{
			return m_charSize.y + m_linePadding;
		}

		bool RichTextView::is_at_bottom(void) const
		{
			const Rectangle area = getScrollArea();
			const Rectangle view = getContentBounds();
			return -getScrollOffset().y + view.h >= area.y + area.h - 1.0f;
		}

		void RichTextView::onUpdate(void)
		{
			update_metrics(getWindow().getGFX());
			const RichTextBuffer& buffer = *m_buffer;
			if (m_seenBuffer != &buffer)
			{
				m_seenBuffer = &buffer;
				m_seenVersion = buffer.getVersion();
				m_seenDroppedLines = buffer.getDroppedLineCount();
				m_seenLineCount = buffer.getLineCount();
				updateScrollbarsSize();
			}
			else if (m_seenVersion != buffer.getVersion())
			{
				// Lines dropped from the top (maxLines) would make the text jump up: the scroll
				// follows them, unless the view sticks to the bottom anyway.
				const u64 dropped = buffer.getDroppedLineCount() - m_seenDroppedLines;
				const bool stick = m_followTail && m_wasAtBottom;
				if (dropped > 0)
				{
					const i32 shift = cast<i32>(dropped);
					m_hoveredLine = (m_hoveredLine >= shift ? m_hoveredLine - shift : -1);
					if (hasSelection())
					{
						m_selectionStart -= shift;
						m_selectionEnd -= shift;
						if (std::max(m_selectionStart, m_selectionEnd) < 0)
							clearSelection();
						else
						{
							m_selectionStart = std::max(0, m_selectionStart);
							m_selectionEnd = std::max(0, m_selectionEnd);
						}
					}
					if (!stick)
					{
						updateScrollbarsSize();
						setScrollOffset(getScrollOffset() + Vec2 { 0, cast<f32>(shift) * line_height() });
					}
				}
				m_seenVersion = buffer.getVersion();
				m_seenDroppedLines = buffer.getDroppedLineCount();
				m_seenLineCount = buffer.getLineCount();
				if (stick)
					scrollToBottom();
			}
			ScrollableWidget::onUpdate();
			if (m_pendingScrollLine >= 0)
			{
				// Re-applied once the scrollbars know the content (scrolling right after filling
				// the buffer would be clamped to the old content otherwise).
				const i32 line = m_pendingScrollLine;
				scrollToLine(line, m_pendingScrollCentered);
				m_pendingScrollLine = -1;
			}
			m_wasAtBottom = is_at_bottom();
		}

		void RichTextView::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			update_metrics(gfx);
			TTF_Font* font = gfx.getDefaultMonoFont(getFontSize());
			if (font == nullptr)
				return;
			const Rectangle view = getGlobalContentBounds();
			const f32 lh = line_height();
			if (lh <= 0)
				return;
			gfx.pushClippingRect(getGlobalPureContentBounds(), true);
			gfx.pushFont(font);
			const Vec2 scroll = getScrollOffset();
			const i32 first = std::max(0, getFirstVisibleLine());
			const i32 last = getLastVisibleLine();
			for (i32 i = first; i <= last; i++)
			{
				const RichTextBuffer::Line* line = m_buffer->getLine(i);
				if (line == nullptr)
					break;
				const Rectangle lineBounds { view.x, view.y + scroll.y + cast<f32>(i) * lh, view.w, lh };
				if (hasSelection() && i >= getSelectionFirst() && i <= getSelectionLast())
					gfx.fillRect(lineBounds, m_selectionColor);
				else if (m_hoverHighlight && i == m_hoveredLine)
					gfx.fillRect(lineBounds, m_hoverColor);
				if (m_lineDecorator)
				{
					gfx.popFont();
					m_lineDecorator(*this, gfx, i, lineBounds);
					gfx.pushFont(font);
				}
				draw_line(gfx, *line, { view.x + scroll.x, lineBounds.y + m_linePadding * 0.5f });
			}
			gfx.popFont();
			gfx.popClippingRect();
		}

		void RichTextView::draw_line(BasicRenderer2D& gfx, const RichTextBuffer::Line& line, const Vec2& position)
		{
			const auto& cells = line.cells;
			const i32 count = cast<i32>(cells.size());
			const f32 cw = m_charSize.x;
			// Backgrounds: one rectangle per run of equal colors.
			for (i32 i = 0; i < count; )
			{
				const u32 bg = cells[i].background;
				i32 j = i + 1;
				while (j < count && cells[j].background == bg)
					j++;
				if (RichTextBuffer::packedAlpha(bg) > 0)
					gfx.fillRect({ position.x + cw * cast<f32>(i), position.y - m_linePadding * 0.5f, cw * cast<f32>(j - i), line_height() }, RichTextBuffer::unpackColor(bg));
				i = j;
			}
			// Text: one string per run of equal colors (ASCII only: other characters are drawn
			// one by one, at their column, in case the font's advance isn't the cell width).
			const u32 defaultColor = RichTextBuffer::packColor(getTextColor());
			auto l_color = [defaultColor](const RichTextBuffer::Cell& cell) {
				return RichTextBuffer::packedAlpha(cell.foreground) > 0 ? cell.foreground : defaultColor;
			};
			stdvec<u32> run;
			for (i32 i = 0; i < count; )
			{
				const u32 fg = l_color(cells[i]);
				const bool ascii = cells[i].codepoint < 0x80;
				i32 j = i;
				bool blank = true;
				run.clear();
				while (j < count)
				{
					if (l_color(cells[j]) != fg || (cells[j].codepoint < 0x80) != ascii || (!ascii && j > i))
						break;
					run.push_back(cells[j].codepoint);
					blank = blank && cells[j].codepoint == ' ';
					j++;
				}
				if (!blank)
					gfx.drawString(String::utf8::encode(run), { position.x + cw * cast<f32>(i), position.y }, RichTextBuffer::unpackColor(fg), getFontSize());
				i = j;
			}
		}

		void RichTextView::afterDraw(ogfx::BasicRenderer2D& gfx)
		{
			drawScrollbars(gfx);
		}

		void RichTextView::onMousePressed(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			if (isMouseInsideAnyScrollbar())
				return;
			const i32 line = getLineAt(pos);
			if (line < 0)
			{
				clearSelection();
				return;
			}
			const u64 now = now_ms();
			const bool doubleClick = (line == m_lastClickLine && now - m_lastClickMs <= DoubleClickMs);
			m_lastClickMs = doubleClick ? 0 : now;
			m_lastClickLine = line;
			if (event.mouse->button == MouseEventData::eButton::Left && m_selectionEnabled)
			{
				m_selectionStart = m_selectionEnd = line;
				m_selecting = true;
			}
			if (m_lineCallback)
				m_lineCallback(*this, line, getColumnAt(pos), event.mouse->button, doubleClick);
		}

		void RichTextView::onMouseReleased(const Event& event)
		{
			if (!m_selecting)
				return;
			m_selecting = false;
			// A click isn't a selection: only dragging over lines selects them.
			if (m_selectionStart == m_selectionEnd)
				clearSelection();
		}

		void RichTextView::onMouseMoved(const Event& event)
		{
			m_hoveredLine = getLineAt({ event.mouse->position_x, event.mouse->position_y });
		}

		void RichTextView::onMouseDragged(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			m_hoveredLine = getLineAt(pos);
			if (!m_selecting)
				return;
			const Rectangle view = getGlobalContentBounds();
			// Dragging past the top / bottom scrolls.
			if (pos.y < view.y)
				addScrollOffset({ 0, line_height() });
			else if (pos.y > view.y + view.h)
				addScrollOffset({ 0, -line_height() });
			const f32 lh = line_height();
			const i32 line = cast<i32>(std::floor((std::clamp(pos.y, view.y, view.y + view.h - 1) - view.y - getScrollOffset().y) / lh));
			m_selectionEnd = std::clamp(line, 0, std::max(0, m_buffer->getLineCount() - 1));
		}

		void RichTextView::onMouseExited(const Event& event)
		{
			m_hoveredLine = -1;
		}

		void RichTextView::onKeyPressed(const Event& event)
		{
			const auto& key = *event.keyboard;
			const f32 lh = line_height();
			if (key.isCopy())
				copyToClipboard();
			else if (key.isSelectAll())
				selectLines(0, m_buffer->getLineCount() - 1);
			else if (key.keyCode == KeyCode::Up)
				addScrollOffset({ 0, lh });
			else if (key.keyCode == KeyCode::Down)
				addScrollOffset({ 0, -lh });
			else if (key.keyCode == KeyCode::Pageup)
				addScrollOffset({ 0, lh * cast<f32>(std::max(1, getVisibleRows() - 1)) });
			else if (key.keyCode == KeyCode::Pagedown)
				addScrollOffset({ 0, -lh * cast<f32>(std::max(1, getVisibleRows() - 1)) });
			else if (key.keyCode == KeyCode::Home)
				scrollToTop();
			else if (key.keyCode == KeyCode::End)
				scrollToBottom();
			else if (key.keyCode == KeyCode::Escape)
				clearSelection();
			else
				return;
			event.handle();
		}

		Rectangle RichTextView::getContentExtents(void) const
		{
			const f32 w = cast<f32>(m_buffer->getLongestLine() + 1) * m_charSize.x;
			const f32 h = cast<f32>(m_buffer->getLineCount()) * line_height();
			return { 0, 0, w, h };
		}
		// ===================================================== RichTextView =====================================================





		// ===================================================== RichTextOutputHandler =====================================================
		void RichTextOutputHandler::setConsoleColor(const String& name, const Color& color)
		{
			m_palette[name.new_toLower().trim()] = color;
		}

		Color RichTextOutputHandler::getConsoleColor(const String& name) const
		{
			auto it = m_palette.find(name.new_toLower().trim());
			return it != m_palette.end() ? it->second : Color { 0, 0, 0, 0 };
		}

		void RichTextOutputHandler::loadDefaultPalette(void)
		{
			// A terminal-like scheme for dark backgrounds (the plain ConsoleColors are pure RGB
			// primaries: unreadable there, e.g. blue on black).
			m_palette.clear();
			m_palette["transparent"]   = { 0, 0, 0, 0 };
			m_palette["black"]         = { 16, 16, 16 };
			m_palette["darkgray"]      = { 115, 118, 115 };
			m_palette["brightgray"]    = { 170, 174, 170 };
			m_palette["white"]         = { 232, 236, 232 };
			m_palette["red"]           = { 235, 85, 85 };
			m_palette["brightred"]     = { 255, 120, 120 };
			m_palette["green"]         = { 90, 205, 105 };
			m_palette["brightgreen"]   = { 130, 240, 150 };
			m_palette["blue"]          = { 95, 145, 255 };
			m_palette["brightblue"]    = { 140, 180, 255 };
			m_palette["magenta"]       = { 205, 115, 225 };
			m_palette["brightmagenta"] = { 235, 160, 255 };
			m_palette["cyan"]          = { 75, 205, 215 };
			m_palette["brightcyan"]    = { 140, 232, 242 };
			m_palette["yellow"]        = { 230, 200, 75 };
			m_palette["brightyellow"]  = { 255, 230, 140 };
		}

		Color RichTextOutputHandler::resolve_name(const String& name, const Color& fallback, bool background) const
		{
			auto it = m_palette.find(name.new_toLower().trim());
			return it != m_palette.end() ? it->second : fallback;
		}

		Color RichTextOutputHandler::resolve(const ostd::TextStyleParser::tColor& color, bool background) const
		{
			// TextStyleParser tags every color with a console color name (custom ones as
			// "black"): only a color that really is that console color is remapped.
			if (color.consoleColor.len() == 0)
				return color.fullColor;
			const auto& fgColor = ostd::ConsoleColors::getFromName(color.consoleColor, false);
			const auto& bgColor = ostd::ConsoleColors::getFromName(color.consoleColor, true);
			if (!same_color(color.fullColor, fgColor.fullColor) && !same_color(color.fullColor, bgColor.fullColor))
				return color.fullColor;
			return resolve_name(color.consoleColor, color.fullColor, background);
		}

		void RichTextOutputHandler::write(const String& text)
		{
			if (m_buffer != nullptr)
				m_buffer->write(text, m_foreground, m_background);
		}

		RichTextOutputHandler& RichTextOutputHandler::bg(const Color& color)
		{
			m_background = color;
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::bg(const ostd::ConsoleColors::tConsoleColor& color)
		{
			m_background = resolve_name(color.name, color.fullColor, true);
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::bg(const String& color)
		{
			m_background = resolve_name(color, ostd::ConsoleColors::getFromName(color, true).fullColor, true);
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::fg(const Color& color)
		{
			m_foreground = color;
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::fg(const ostd::ConsoleColors::tConsoleColor& color)
		{
			m_foreground = resolve_name(color.name, color.fullColor, false);
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::fg(const String& color)
		{
			m_foreground = resolve_name(color, ostd::ConsoleColors::getFromName(color, false).fullColor, false);
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::pChar(char c)
		{
			write(String("").addChar(c));
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::pStyled(const String& styled)
		{
			return pStyled(ostd::TextStyleParser::parse(styled, { m_background, "", true }, { m_foreground, "", false }));
		}

		RichTextOutputHandler& RichTextOutputHandler::pStyled(const ostd::TextStyleParser::tStyledString& styled)
		{
			if (m_buffer == nullptr || !styled.validate())
				return *this;
			ostd::TextStyleParser::tStyledString resolved = styled;
			for (auto& color : resolved.foregroundColors)
				color.fullColor = resolve(color, false);
			for (auto& color : resolved.backgroundColors)
				color.fullColor = resolve(color, true);
			m_buffer->write(resolved);
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::pStyled(ostd::TextStyleBuilder::IRichStringBase& styled)
		{
			// The text that no rule colors gets the current colors.
			auto oldBg = styled.getDefaultBackgroundColor();
			auto oldFg = styled.getDefaultForegroundColor();
			styled.setDefaultBackgroundColor(ostd::TextStyleParser::tColor { m_background, "", true });
			styled.setDefaultForegroundColor(ostd::TextStyleParser::tColor { m_foreground, "", false });
			pStyled(styled.getStyledString());
			styled.setDefaultBackgroundColor(oldBg);
			styled.setDefaultForegroundColor(oldFg);
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::pObject(const ostd::Object& bo)
		{
			write(bo.toString());
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::p(const String& se) { write(se); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(u8 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(i8 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(u16 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(i16 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(u32 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(i32 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(u64 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(i64 i) { write(String("").add(i)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(f32 f, u8 precision) { write(String("").add(f, precision)); return *this; }
		RichTextOutputHandler& RichTextOutputHandler::p(f64 f, u8 precision) { write(String("").add(f, precision)); return *this; }

		RichTextOutputHandler& RichTextOutputHandler::nl(void)
		{
			write("\n");
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::tab(void)
		{
			write("\t");
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::flush(void)
		{
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::clear(void)
		{
			if (m_buffer != nullptr)
				m_buffer->clear();
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::reset(void)
		{
			m_foreground = m_defaultForeground;
			m_background = m_defaultBackground;
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::xy(IPoint position)
		{
			return xy(position.x, position.y);
		}

		RichTextOutputHandler& RichTextOutputHandler::xy(i32 x, i32 y)
		{
			if (m_buffer != nullptr)
				m_buffer->setCursor(x, y);
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::x(i32 x)
		{
			if (m_buffer != nullptr)
				m_buffer->setCursor(x, m_buffer->getCursorLine());
			return *this;
		}

		RichTextOutputHandler& RichTextOutputHandler::y(i32 y)
		{
			if (m_buffer != nullptr)
				m_buffer->setCursor(m_buffer->getCursorColumn(), y);
			return *this;
		}

		IPoint RichTextOutputHandler::getCursorPosition(void)
		{
			return { getCursorX(), getCursorY() };
		}

		void RichTextOutputHandler::getCursorPosition(i32& outX, i32& outY)
		{
			outX = getCursorX();
			outY = getCursorY();
		}

		i32 RichTextOutputHandler::getCursorX(void)
		{
			return m_buffer != nullptr ? m_buffer->getCursorColumn() : 0;
		}

		i32 RichTextOutputHandler::getCursorY(void)
		{
			return m_buffer != nullptr ? m_buffer->getCursorLine() : 0;
		}

		void RichTextOutputHandler::getConsoleSize(i32& outColumns, i32& outRows)
		{
			outColumns = m_consoleSize.x;
			outRows = m_consoleSize.y;
		}

		IPoint RichTextOutputHandler::getConsoleSize(void)
		{
			return m_consoleSize;
		}
		// ===================================================== RichTextOutputHandler =====================================================
	}
}
