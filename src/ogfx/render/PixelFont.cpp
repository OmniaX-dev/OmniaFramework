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

		// Per axis, the source pixels of each destination pixel and their weights (16.16 fixed
		// point, summing to 1): growing (or the same size), the nearest source pixel; shrinking,
		// every source pixel under the destination pixel, weighted by how much of it is covered.
		// Computed once and shared by every glyph.
		struct Tap { i32 source; u32 weight; };
		auto l_taps = [](i32 sourceSize, i32 destSize, stdvec<u32>& first, stdvec<Tap>& taps) {
			first.assign(cast<size_t>(destSize) + 1, 0);
			taps.clear();
			const f64 ratio = cast<f64>(sourceSize) / destSize;
			for (i32 d = 0; d < destSize; d++)
			{
				first[d] = cast<u32>(taps.size());
				if (destSize >= sourceSize)
				{
					taps.push_back({ std::min(cast<i32>((d + 0.5) * ratio), sourceSize - 1), 65536 });
					continue;
				}
				const f64 from = d * ratio, to = (d + 1) * ratio;
				u32 total = 0;
				size_t largest = taps.size();
				for (i32 s = cast<i32>(from); s < sourceSize && s < to; s++)
				{
					const f64 covered = std::min<f64>(s + 1, to) - std::max<f64>(s, from);
					const u32 weight = cast<u32>(std::lround(covered / ratio * 65536.0));
					if (weight == 0)
						continue;
					if (largest == taps.size() || weight > taps[largest].weight)
						largest = taps.size();
					taps.push_back({ s, weight });
					total += weight;
				}
				taps[largest].weight += 65536 - total;	// (the rounding: the weights sum to exactly 1)
			}
			first[destSize] = cast<u32>(taps.size());
		};
		stdvec<u32> firstX, firstY;
		stdvec<Tap> tapsX, tapsY;
		l_taps(m_charWidth, width, firstX, tapsX);
		l_taps(m_charHeight, height, firstY, tapsY);

		// Horizontally into `rows` (source height x destination width, 16.16), then vertically.
		auto set = std::make_unique<GlyphSet>();
		set->width = width;
		set->height = height;
		set->coverage.resize(cast<size_t>(m_glyphCount) * width * height);
		stdvec<u32> rows(cast<size_t>(m_charHeight) * width);
		u8* out = set->coverage.data();
		for (u32 g = 0; g < m_glyphCount; g++)
		{
			const u8* glyph = m_glyphs.coverage.data() + cast<size_t>(g) * m_charWidth * m_charHeight;
			for (i32 y = 0; y < m_charHeight; y++)
			{
				const u8* src = glyph + cast<size_t>(y) * m_charWidth;
				u32* dst = rows.data() + cast<size_t>(y) * width;
				for (i32 x = 0; x < width; x++)
				{
					u32 sum = 0;
					for (u32 t = firstX[x]; t < firstX[x + 1]; t++)
						sum += src[tapsX[t].source] * tapsX[t].weight;
					dst[x] = sum;		// at most 255 * 65536
				}
			}
			for (i32 y = 0; y < height; y++)
			{
				for (i32 x = 0; x < width; x++)
				{
					u64 sum = 0;
					for (u32 t = firstY[y]; t < firstY[y + 1]; t++)
						sum += cast<u64>(rows[cast<size_t>(tapsY[t].source) * width + x]) * tapsY[t].weight;
					*out++ = cast<u8>(std::min<u64>((sum + (1ull << 31)) >> 32, 255));
				}
			}
		}
		m_scaledGlyphs.push_back(std::move(set));
		return m_scaledGlyphs.back().get();
	}
}
