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

#include <ogfx/gui/widgets/PixelCanvas.hpp>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// TextConsoleCanvas — a PixelCanvas that can show a grid of character cells.
		//
		//   PixelFont font { "font.bmp" };                  // 16 x 6 glyph grid
		//   TextConsoleCanvas console { window, 640, 480 };
		//   console.setFont(font);
		//   stdvec<u32> text;                                // owned by you
		//   console.enterTextConsoleMode(text, 0, palette, 8, 20);
		//   text[0] = TextConsoleCanvas::makeCell('A', 10, 0);
		//
		// Text mode: the canvas shows the cells of an external buffer, starting at a cell
		// offset. Each u32 cell is, from the most significant byte:
		//     reserved (unused) | character | foreground color | background color
		// The colors are indices into a ColorPalette256. Index 0 is transparent in every
		// palette, and means "the console's default color" here (setDefaultColors()).
		//
		// The buffer is resized to two screens of cells (getColumns() * getRows() * 2) and
		// is read at every canvas frame: just write cells, nothing else is needed. setOffset()
		// scrolls through it (up to one screen). The number of columns and rows comes from
		// the canvas resolution and the cell size (the font's, unless given; the cells are
		// centered on the canvas).
		//
		// While text mode is on, the regular drawing functions and raw pixel access of
		// PixelCanvas are disabled (no-ops, getPixels() returns nullptr): the pixels belong to
		// the console. exitTextConsoleMode() turns the widget back into a plain PixelCanvas.
		//
		// Rendering: the glyphs are scaled to the cell size once (cached by the font), only
		// the cells that changed since the previous canvas frame are redrawn (and uploaded),
		// and scrolling by whole rows moves the existing pixels instead of redrawing them.
		// ============================================================================
		class TextConsoleCanvas : public PixelCanvas
		{
			public:
				inline TextConsoleCanvas(Window& window, i32 width = DefaultWidth, i32 height = DefaultHeight) : PixelCanvas(window, width, height) { setTypeName("ogfx::gui::TextConsoleCanvas"); }

				// Starts text mode on `buffer` (owned by the caller, kept by pointer until text
				// mode ends: it must outlive it), resized to getColumns() * getRows() * 2 cells,
				// and returned. charWidth/charHeight are the cell size: 0 for both = the font's,
				// one of them 0 = derived from the other with the font's aspect ratio. The palette
				// is kept by pointer too. Needs a font (setFont()); returns the buffer unchanged,
				// without entering text mode, if there is none.
				stdvec<u32>& enterTextConsoleMode(stdvec<u32>& buffer, u32 offset = 0, const ColorPalette256& palette = DefaultPalette, u8 charWidth = 0, u8 charHeight = 0);
				void exitTextConsoleMode(void);
				inline bool isTextConsoleMode(void) const { return m_textBuffer != nullptr; }

				// First cell shown (clamped to getColumns() * getRows(), i.e. one screen).
				void setOffset(u32 offsetInChars);
				inline u32 getOffset(void) const { return m_offset; }
				void setPalette(const ColorPalette256& palette);
				inline const ColorPalette256* getPalette(void) const { return m_palette; }
				// Used for the cells whose foreground / background index is 0.
				void setDefaultColors(const Color& foreground, const Color& background);
				inline Color getDefaultForeground(void) const { return toColor(m_defaultForeground); }
				inline Color getDefaultBackground(void) const { return toColor(m_defaultBackground); }
				// Redraws every cell on the next canvas frame (normally not needed).
				inline void redrawAll(void) { m_fullRedraw = true; }

				inline u32 getColumns(void) const { return m_columns; }
				inline u32 getRows(void) const { return m_rows; }
				inline u32 getScreenCellCount(void) const { return m_columns * m_rows; }
				inline IPoint getCellSize(void) const { return { m_cellWidth, m_cellHeight }; }
				// Top-left of the cell grid on the canvas (it is centered).
				inline IPoint getGridOrigin(void) const { return { m_originX, m_originY }; }
				// Cells redrawn by the last canvas frame.
				inline u32 getLastRedrawnCellCount(void) const { return m_lastRedrawnCells; }

				// ===== Cells =====
				static constexpr u32 makeCell(u8 character, u8 foreground = 0, u8 background = 0) { return (cast<u32>(character) << 16) | (cast<u32>(foreground) << 8) | cast<u32>(background); }
				static constexpr u8 getCellCharacter(u32 cell) { return cast<u8>((cell >> 16) & 0xFF); }
				static constexpr u8 getCellForeground(u32 cell) { return cast<u8>((cell >> 8) & 0xFF); }
				static constexpr u8 getCellBackground(u32 cell) { return cast<u8>(cell & 0xFF); }
				// Writes text into a cell buffer from cell `index` ('\n' is not interpreted);
				// returns the index after the last cell written.
				static u32 writeString(stdvec<u32>& buffer, u32 index, const String& text, u8 foreground = 0, u8 background = 0);

			protected:
				void render_canvas_frame(void) override;

			private:
				bool layout(void);
				void scroll_pixels(i32 rows);
				void fill_margins(void);
				void draw_cell(u32 column, u32 row, u32 cell, const PixelFont& font, const u8* glyphs);

			private:
				// Never a real cell value: the reserved byte of the cells is masked out.
				inline static constexpr u32 InvalidCell { 0xFFFFFFFF };

				stdvec<u32>* m_textBuffer { nullptr };
				const ColorPalette256* m_palette { nullptr };
				u32 m_paletteVersion { 0 };
				u32 m_offset { 0 };
				u32 m_drawnOffset { 0 };
				u8 m_requestedCharWidth { 0 };
				u8 m_requestedCharHeight { 0 };
				i32 m_cellWidth { 0 };
				i32 m_cellHeight { 0 };
				u32 m_columns { 0 };
				u32 m_rows { 0 };
				i32 m_originX { 0 };
				i32 m_originY { 0 };
				u32 m_defaultForeground { toPixel(192, 192, 192) };
				u32 m_defaultBackground { toPixel(0, 0, 0) };
				bool m_fullRedraw { true };
				u32 m_lastRedrawnCells { 0 };

				// What the layout was computed for: a change re-lays-out the console.
				const PixelFont* m_layoutFont { nullptr };
				u32 m_layoutFontVersion { 0 };
				IPoint m_layoutResolution { 0, 0 };

				// Cell value drawn at each screen position.
				stdvec<u32> m_drawnCells;
				// coverage -> final pixel, for a foreground/background pair. A small cache keyed by the
				// pair: a screen rarely uses more than a few pairs, so they're built once.
				struct ColorLut
				{
					u32 foreground { 0 };
					u32 background { 0 };
					bool valid { false };
					std::array<u32, 256> colors;
				};
				const u32* get_color_lut(u32 foreground, u32 background);
				inline static constexpr u32 ColorLutCacheSize { 64 };
				std::array<ColorLut, ColorLutCacheSize> m_colorLuts;
		};
	}
}
