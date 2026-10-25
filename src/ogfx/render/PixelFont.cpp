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

#include "PixelFont.hpp"
#include "../utils/SDLInclude.hpp"
#include <ostd/io/Logger.hpp>

namespace ogfx
{
	bool PixelFont::loadFromFile(const String& filePath, i32 columns, i32 rows, u8 firstCharacter)
	{
		unload();
		if (columns <= 0 || rows <= 0)
		{
			OX_ERROR("PixelFont: invalid grid size %dx%d", columns, rows);
			return false;
		}
		SDL_Surface* loaded = IMG_Load(filePath.c_str());
		if (!loaded)
		{
			OX_ERROR("PixelFont: failed to load %s: %s", filePath.c_str(), SDL_GetError());
			return false;
		}
		SDL_Surface* surface = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_ARGB8888);
		SDL_DestroySurface(loaded);
		if (!surface)
		{
			OX_ERROR("PixelFont: failed to convert %s: %s", filePath.c_str(), SDL_GetError());
			return false;
		}
		const i32 charWidth = surface->w / columns;
		const i32 charHeight = surface->h / rows;
		if (charWidth <= 0 || charHeight <= 0)
		{
			OX_ERROR("PixelFont: %s (%dx%d) is too small for a %dx%d grid", filePath.c_str(), surface->w, surface->h, columns, rows);
			SDL_DestroySurface(surface);
			return false;
		}
		if (surface->w % columns != 0 || surface->h % rows != 0)
			OX_WARN("PixelFont: %s is %dx%d, not a multiple of the %dx%d grid; the extra pixels are ignored", filePath.c_str(), surface->w, surface->h, columns, rows);

		const u32 glyphCount = std::min<u32>(cast<u32>(columns * rows), 256u - firstCharacter);
		m_glyphs.width = charWidth;
		m_glyphs.height = charHeight;
		m_glyphs.coverage.assign(cast<size_t>(glyphCount) * charWidth * charHeight, 0);
		const u8* base = cast<const u8*>(surface->pixels);
		for (u32 g = 0; g < glyphCount; g++)
		{
			const i32 cellX = cast<i32>(g % cast<u32>(columns)) * charWidth;
			const i32 cellY = cast<i32>(g / cast<u32>(columns)) * charHeight;
			u8* out = m_glyphs.coverage.data() + cast<size_t>(g) * charWidth * charHeight;
			for (i32 y = 0; y < charHeight; y++)
			{
				const u32* row = reinterpret_cast<const u32*>(base + cast<size_t>(cellY + y) * surface->pitch) + cellX;
				for (i32 x = 0; x < charWidth; x++)
				{
					const u32 p = row[x];
					const u32 a = p >> 24, r = (p >> 16) & 0xFF, g2 = (p >> 8) & 0xFF, b = p & 0xFF;
					const u32 luminance = (r * 77 + g2 * 150 + b * 29) >> 8;
					*out++ = cast<u8>((luminance * a + 127) / 255);
				}
			}
		}
		SDL_DestroySurface(surface);

		m_charWidth = charWidth;
		m_charHeight = charHeight;
		m_glyphCount = glyphCount;
		m_firstCharacter = firstCharacter;
		m_filePath = filePath;
		m_loaded = true;
		m_version++;
		validate();
		return true;
	}

	void PixelFont::unload(void)
	{
		m_glyphs = GlyphSet();
		m_scaledGlyphs.clear();
		m_charWidth = 0;
		m_charHeight = 0;
		m_glyphCount = 0;
		m_filePath = "";
		if (m_loaded)
			m_version++;
		m_loaded = false;
		invalidate();
	}

	const u8* PixelFont::getGlyph(u8 c) const
	{
		if (!m_loaded || !hasCharacter(c))
			return nullptr;
		return m_glyphs.coverage.data() + cast<size_t>(c - m_firstCharacter) * m_charWidth * m_charHeight;
	}

	const u8* PixelFont::getGlyph(u8 c, i32 width, i32 height) const
	{
		if (!m_loaded || !hasCharacter(c))
			return nullptr;
		const GlyphSet* set = get_glyph_set(width, height);
		if (!set)
			return nullptr;
		return set->coverage.data() + cast<size_t>(c - m_firstCharacter) * set->width * set->height;
	}

	const PixelFont::GlyphSet* PixelFont::get_glyph_set(i32 width, i32 height) const
	{
		if (width <= 0 || height <= 0)
			return nullptr;
		if (width == m_charWidth && height == m_charHeight)
			return &m_glyphs;
		for (auto& set : m_scaledGlyphs)
		{
			if (set->width == width && set->height == height)
				return set.get();
		}
		if (m_scaledGlyphs.size() >= MaxCachedSizes)
			m_scaledGlyphs.erase(m_scaledGlyphs.begin());

		// Nearest neighbour, all glyphs at once: the source row/column of each destination
		// row/column is computed once and shared by every glyph.
		auto set = std::make_unique<GlyphSet>();
		set->width = width;
		set->height = height;
		set->coverage.resize(cast<size_t>(m_glyphCount) * width * height);
		stdvec<i32> srcX(width), srcY(height);
		for (i32 x = 0; x < width; x++)
			srcX[x] = std::min(cast<i32>((x + 0.5f) * m_charWidth / width), m_charWidth - 1);
		for (i32 y = 0; y < height; y++)
			srcY[y] = std::min(cast<i32>((y + 0.5f) * m_charHeight / height), m_charHeight - 1);
		u8* out = set->coverage.data();
		for (u32 g = 0; g < m_glyphCount; g++)
		{
			const u8* glyph = m_glyphs.coverage.data() + cast<size_t>(g) * m_charWidth * m_charHeight;
			for (i32 y = 0; y < height; y++)
			{
				const u8* row = glyph + cast<size_t>(srcY[y]) * m_charWidth;
				for (i32 x = 0; x < width; x++)
					*out++ = row[srcX[x]];
			}
		}
		m_scaledGlyphs.push_back(std::move(set));
		return m_scaledGlyphs.back().get();
	}
}
