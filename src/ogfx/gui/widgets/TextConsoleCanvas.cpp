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

#include "TextConsoleCanvas.hpp"
#include "../../render/PixelFont.hpp"
#include <ostd/io/Logger.hpp>
#include <cstring>

namespace ogfx
{
	namespace gui
	{
		stdvec<u32>& TextConsoleCanvas::enterTextConsoleMode(stdvec<u32>& buffer, u32 offset, const ColorPalette256& palette, u8 charWidth, u8 charHeight)
		{
			m_textBuffer = &buffer;
			m_palette = &palette;
			m_paletteVersion = palette.getVersion();
			m_requestedCharWidth = charWidth;
			m_requestedCharHeight = charHeight;
			m_offset = offset;
			if (!layout())
			{
				OX_ERROR("TextConsoleCanvas: text mode needs a loaded font (setFont()) and a cell size that fits the canvas");
				m_textBuffer = nullptr;
				return buffer;
			}
			m_drawingDisabled = true;
			return buffer;
		}

		void TextConsoleCanvas::exitTextConsoleMode(void)
		{
			m_textBuffer = nullptr;
			m_drawingDisabled = false;
		}

		void TextConsoleCanvas::setOffset(u32 offsetInChars)
		{
			m_offset = std::min(offsetInChars, getScreenCellCount());
		}

		void TextConsoleCanvas::setPalette(const ColorPalette256& palette)
		{
			m_palette = &palette;
			m_paletteVersion = palette.getVersion();
			m_fullRedraw = true;
		}

		void TextConsoleCanvas::setDefaultColors(const Color& foreground, const Color& background)
		{
			const u32 fg = toPixel(foreground), bg = toPixel(background);
			if (fg == m_defaultForeground && bg == m_defaultBackground)
				return;
			m_defaultForeground = fg;
			m_defaultBackground = bg;
			m_fullRedraw = true;
		}

		u32 TextConsoleCanvas::writeString(stdvec<u32>& buffer, u32 index, const String& text, u8 foreground, u8 background)
		{
			for (const char ch : text.cpp_str())
			{
				if (index >= buffer.size())
					break;
				buffer[index++] = makeCell(cast<u8>(ch), foreground, background);
			}
			return index;
		}

		bool TextConsoleCanvas::layout(void)
		{
			const PixelFont* font = getFont();
			if (!font || !font->isLoaded() || !m_textBuffer)
				return false;
			const i32 fontW = font->getCharWidth(), fontH = font->getCharHeight();
			i32 cellW = m_requestedCharWidth, cellH = m_requestedCharHeight;
			if (cellW == 0 && cellH == 0)
			{
				cellW = fontW;
				cellH = fontH;
			}
			else if (cellW == 0)
			{
				cellW = std::max(1, cast<i32>(std::lround(cellH * cast<f32>(fontW) / fontH)));
			}
			else if (cellH == 0)
			{
				cellH = std::max(1, cast<i32>(std::lround(cellW * cast<f32>(fontH) / fontW)));
			}
			const i32 columns = m_width / cellW, rows = m_height / cellH;
			if (columns <= 0 || rows <= 0)
				return false;

			m_cellWidth = cellW;
			m_cellHeight = cellH;
			m_columns = cast<u32>(columns);
			m_rows = cast<u32>(rows);
			m_originX = (m_width - columns * cellW) / 2;
			m_originY = (m_height - rows * cellH) / 2;
			m_textBuffer->resize(cast<size_t>(getScreenCellCount()) * 2, 0);
			m_offset = std::min(m_offset, getScreenCellCount());
			m_drawnOffset = m_offset;
			m_drawnCells.assign(getScreenCellCount(), InvalidCell);
			m_layoutFont = font;
			m_layoutFontVersion = font->getVersion();
			m_layoutResolution = getResolution();
			m_fullRedraw = true;
			return true;
		}

		void TextConsoleCanvas::render_canvas_frame(void)
		{
			if (!m_textBuffer)
				return;
			const PixelFont* font = getFont();
			if (!font || !font->isLoaded())
				return;
			// The font or the resolution changed: lay the console out again (resizes the buffer).
			if (font != m_layoutFont || font->getVersion() != m_layoutFontVersion || getResolution().x != m_layoutResolution.x || getResolution().y != m_layoutResolution.y)
			{
				if (!layout())
					return;
			}
			const ColorPalette256& palette = m_palette ? *m_palette : DefaultPalette;
			if (palette.getVersion() != m_paletteVersion)
			{
				m_paletteVersion = palette.getVersion();
				m_fullRedraw = true;
			}

			const u32 screenCells = getScreenCellCount();
			// Scrolling by whole rows: move the pixels and the drawn-cell record along, so only
			// the rows that scrolled in need drawing.
			if (!m_fullRedraw && m_offset != m_drawnOffset)
			{
				const i64 delta = cast<i64>(m_offset) - cast<i64>(m_drawnOffset);
				const i64 rows = delta / cast<i64>(m_columns);
				if (delta % cast<i64>(m_columns) == 0 && std::abs(rows) < cast<i64>(m_rows))
				{
					scroll_pixels(cast<i32>(rows));
					const u32 moved = cast<u32>(std::abs(rows)) * m_columns;
					if (rows > 0)
					{
						std::memmove(m_drawnCells.data(), m_drawnCells.data() + moved, (screenCells - moved) * sizeof(u32));
						std::fill(m_drawnCells.end() - moved, m_drawnCells.end(), InvalidCell);
					}
					else
					{
						std::memmove(m_drawnCells.data() + moved, m_drawnCells.data(), (screenCells - moved) * sizeof(u32));
						std::fill(m_drawnCells.begin(), m_drawnCells.begin() + moved, InvalidCell);
					}
				}
				else
				{
					m_fullRedraw = true;
				}
			}
			if (m_fullRedraw)
			{
				std::fill(m_drawnCells.begin(), m_drawnCells.end(), InvalidCell);
				fill_margins();
			}

			const u8* glyphs = font->getGlyph(font->getFirstCharacter(), m_cellWidth, m_cellHeight);
			const u32* cells = m_textBuffer->data();
			const size_t cellCount = m_textBuffer->size();
			u32 redrawn = 0;
			for (u32 i = 0; i < screenCells; i++)
			{
				const size_t index = cast<size_t>(m_offset) + i;
				const u32 cell = index < cellCount ? (cells[index] & 0x00FFFFFF) : 0;
				if (m_drawnCells[i] == cell)
					continue;
				draw_cell(i % m_columns, i / m_columns, cell, *font, glyphs);
				m_drawnCells[i] = cell;
				redrawn++;
			}
			m_drawnOffset = m_offset;
			m_fullRedraw = false;
			m_lastRedrawnCells = redrawn;
		}

		void TextConsoleCanvas::draw_cell(u32 column, u32 row, u32 cell, const PixelFont& font, const u8* glyphs)
		{
			const ColorPalette256& palette = m_palette ? *m_palette : DefaultPalette;
			const u8 character = getCellCharacter(cell), fgIndex = getCellForeground(cell), bgIndex = getCellBackground(cell);
			const u32 fg = fgIndex != 0 ? palette[fgIndex] : m_defaultForeground;
			const u32 bg = bgIndex != 0 ? palette[bgIndex] : m_defaultBackground;
			const u32* lut = get_color_lut(fg, bg);

			const i32 x = m_originX + cast<i32>(column) * m_cellWidth;
			const i32 y = m_originY + cast<i32>(row) * m_cellHeight;
			const u8* glyph = (glyphs && character != ' ' && font.hasCharacter(character))
				? glyphs + cast<size_t>(character - font.getFirstCharacter()) * m_cellWidth * m_cellHeight
				: nullptr;
			for (i32 py = 0; py < m_cellHeight; py++)
			{
				u32* dst = m_pixels.data() + cast<size_t>(y + py) * m_width + x;
				if (!glyph)
				{
					std::fill(dst, dst + m_cellWidth, lut[0]);
					continue;
				}
				const u8* coverage = glyph + cast<size_t>(py) * m_cellWidth;
				for (i32 px = 0; px < m_cellWidth; px++)
					dst[px] = lut[coverage[px]];
			}
			expand_dirty(x, y, x + m_cellWidth, y + m_cellHeight);
		}

		const u32* TextConsoleCanvas::get_color_lut(u32 foreground, u32 background)
		{
			const u32 hash = (foreground * 0x9E3779B1u) ^ (background * 0x85EBCA77u);
			ColorLut& lut = m_colorLuts[(hash >> 16) % ColorLutCacheSize];
			if (lut.valid && lut.foreground == foreground && lut.background == background)
				return lut.colors.data();
			// Linear from background (coverage 0) to foreground (255), per channel, in 16.16
			// fixed point: no division per entry.
			i32 acc[4], step[4];
			for (u32 ch = 0; ch < 4; ch++)
			{
				const i32 f = cast<i32>((foreground >> (ch * 8)) & 0xFF), b = cast<i32>((background >> (ch * 8)) & 0xFF);
				acc[ch] = (b << 16) + 0x8000;
				step[ch] = ((f - b) * 65536) / 255;
			}
			for (u32 c = 0; c < 256; c++)
			{
				lut.colors[c] = (cast<u32>(acc[0] >> 16)) | (cast<u32>(acc[1] >> 16) << 8) | (cast<u32>(acc[2] >> 16) << 16) | (cast<u32>(acc[3] >> 16) << 24);
				for (u32 ch = 0; ch < 4; ch++)
					acc[ch] += step[ch];
			}
			lut.colors[0] = background;
			lut.colors[255] = foreground;
			lut.foreground = foreground;
			lut.background = background;
			lut.valid = true;
			return lut.colors.data();
		}

		void TextConsoleCanvas::scroll_pixels(i32 rows)
		{
			if (rows == 0)
				return;
			const i32 gridW = cast<i32>(m_columns) * m_cellWidth;
			const i32 gridH = cast<i32>(m_rows) * m_cellHeight;
			const i32 shift = rows * m_cellHeight;
			auto row_ptr = [this](i32 y) { return m_pixels.data() + cast<size_t>(m_originY + y) * m_width + m_originX; };
			if (shift > 0)
			{
				for (i32 y = 0; y < gridH - shift; y++)
					std::memcpy(row_ptr(y), row_ptr(y + shift), gridW * sizeof(u32));
			}
			else
			{
				for (i32 y = gridH - 1; y >= -shift; y--)
					std::memcpy(row_ptr(y), row_ptr(y + shift), gridW * sizeof(u32));
			}
			expand_dirty(m_originX, m_originY, m_originX + gridW, m_originY + gridH);
		}

		void TextConsoleCanvas::fill_margins(void)
		{
			// The cells cover the grid right after this; filling everything is simplest.
			std::fill(m_pixels.begin(), m_pixels.end(), m_defaultBackground);
			mark_all_dirty();
		}
	}
}
