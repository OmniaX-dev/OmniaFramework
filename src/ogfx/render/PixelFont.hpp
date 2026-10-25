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

#include <ostd/data/Object.hpp>
#include <ostd/string/String.hpp>
#include <ostd/math/Geometry.hpp>
#include <memory>

namespace ogfx
{
	// ============================================================================
	// PixelFont — a monospace bitmap font for gui::PixelCanvas.
	//
	// The image is a grid of equally sized glyph cells (16 x 6 by default), in ASCII
	// order from `firstCharacter` (32, the space, by default): with the defaults it
	// holds the 95 printable characters 32..126 (+ one unused cell). The cell size is
	// the image size divided by the grid size.
	//
	// Glyphs are light on dark (or on transparent): each pixel's coverage is its
	// brightness times its alpha, and is used to blend the text color over the
	// background, so antialiased fonts work too. Any format SDL_image reads is fine
	// (BMP, PNG, ...).
	//
	// The coverage of every glyph is extracted once when the font is loaded; scaled
	// copies (nearest-neighbour, for other cell sizes) are made on first use and
	// cached. Fonts don't need a window or renderer.
	// ============================================================================
	class PixelFont : public ostd::Object
	{
		public:
			inline PixelFont(void) { setTypeName("ogfx::PixelFont"); invalidate(); }
			inline PixelFont(const String& filePath, i32 columns = DefaultColumns, i32 rows = DefaultRows, u8 firstCharacter = DefaultFirstCharacter) : PixelFont() { loadFromFile(filePath, columns, rows, firstCharacter); }
			PixelFont(const PixelFont&) = delete;
			PixelFont& operator=(const PixelFont&) = delete;

			bool loadFromFile(const String& filePath, i32 columns = DefaultColumns, i32 rows = DefaultRows, u8 firstCharacter = DefaultFirstCharacter);
			void unload(void);

			inline bool isLoaded(void) const { return m_loaded; }
			inline i32 getCharWidth(void) const { return m_charWidth; }
			inline i32 getCharHeight(void) const { return m_charHeight; }
			inline IPoint getCharSize(void) const { return { m_charWidth, m_charHeight }; }
			// Width / height of a character cell.
			inline f32 getCharAspectRatio(void) const { return m_charHeight > 0 ? cast<f32>(m_charWidth) / cast<f32>(m_charHeight) : 1.0f; }
			inline u8 getFirstCharacter(void) const { return m_firstCharacter; }
			inline u32 getCharacterCount(void) const { return m_glyphCount; }
			inline bool hasCharacter(u8 c) const { return c >= m_firstCharacter && cast<u32>(c - m_firstCharacter) < m_glyphCount; }
			inline String getFilePath(void) const { return m_filePath; }
			// Incremented on every (re)load: lets users of the font notice the change.
			inline u32 getVersion(void) const { return m_version; }

			// Coverage (0..255) of a glyph, getCharWidth() * getCharHeight() bytes, row by row.
			// nullptr for characters the font doesn't have.
			const u8* getGlyph(u8 c) const;
			// Same, scaled to width x height (nearest neighbour). Scaled sets are cached.
			const u8* getGlyph(u8 c, i32 width, i32 height) const;

		private:
			struct GlyphSet
			{
				i32 width { 0 };
				i32 height { 0 };
				stdvec<u8> coverage; // glyph after glyph, width * height bytes each
			};
			const GlyphSet* get_glyph_set(i32 width, i32 height) const;

		public:
			inline static constexpr i32 DefaultColumns { 16 };
			inline static constexpr i32 DefaultRows { 6 };
			inline static constexpr u8 DefaultFirstCharacter { 32 };
			inline static constexpr u32 MaxCachedSizes { 8 };

		private:
			GlyphSet m_glyphs;
			mutable stdvec<std::unique_ptr<GlyphSet>> m_scaledGlyphs; // stable addresses
			String m_filePath { "" };
			i32 m_charWidth { 0 };
			i32 m_charHeight { 0 };
			u32 m_glyphCount { 0 };
			u8 m_firstCharacter { DefaultFirstCharacter };
			u32 m_version { 0 };
			bool m_loaded { false };
	};
}
