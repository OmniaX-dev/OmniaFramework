/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2025  OmniaX-Dev

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

#include "FontGlyphAtlas.hpp"
#include "BasicRenderer.hpp"
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace ogfx
{
	FontGlyphAtlas FontGlyphAtlas::init(BasicRenderer2D& renderer)
	{
		m_renderer = &renderer;
		m_uvs.reserve(4096);
		for (i32 i = 0; i < FontGlyphAtlas::MaxAtlasCount; i++)
			m_atlases[i] = nullptr;
		m_currentAtlasCount = 0;
		create_new_atlas();
		return *this;
	}

	const stdvec<const FontGlyphAtlas::GlyphInfo*> FontGlyphAtlas::processString(const String& str, TTF_Font* font, u32 fontSize)
	{
		if (m_currentAtlasCount <= 0)
	        return {};

	    // Pass 1: ensure all glyphs are rasterized (map may rehash here)
	    for (auto& c : str)
	    {
	        const GlyphInfo* dummy;
	        if (!rasterize_glyph(String("").addChar(c), font, fontSize, &dummy))
	            return {};
	    }

	    // Pass 2: collect stable pointers (no more insertions, no rehash risk)
	    stdvec<const GlyphInfo*> glyphs;
	    glyphs.reserve(str.len());
	    for (auto& c : str)
	    {
	        auto cps = String("").addChar(c).getUTF8Codepoints();
	        if (cps.size() != 1) return {};
	        GlyphKey key { cps[0], u64(font), fontSize };
	        glyphs.push_back(&m_uvs[key]);
	    }
	    return glyphs;
	}

	bool FontGlyphAtlas::rasterize_glyph(const String& glyphStr, TTF_Font* font, u32 fontSize, const GlyphInfo** outGlyph)
	{
		if (!font || m_currentAtlasCount <= 0)
			return false;

		auto cps = glyphStr.getUTF8Codepoints();
		if (cps.size() != 1)
			return false;

		u32 codepoint = cps[0];

		GlyphKey key { codepoint, u64(font), fontSize };
		auto it = m_uvs.find(key);
		if (it != m_uvs.end())
		{
			*outGlyph = &(it->second);
		    return true;
		}

		SDL_Surface* surf = TTF_RenderText_Blended(font, glyphStr.c_str(), glyphStr.len(), SDL_Color{255, 255, 255, 255});
		if (!surf)
			return false;

		i32 gw = surf->w;
		i32 gh = surf->h;

		i32 minx, maxx, miny, maxy, advance;
		TTF_GetGlyphMetrics(font, codepoint, &minx, &maxx, &miny, &maxy, &advance);

		GlyphInfo glyph;
		glyph.advance = advance;
		glyph.codepoint = codepoint;

		SDL_Texture* atlas = m_atlases[m_currentAtlasCount - 1];

		if (m_penX + gw > i32(AtlasTextureDimension))
		{
			m_penX = 0;
			m_penY += m_rowHeight;
			m_rowHeight = 0;
		}

		if (m_penY + gh > i32(AtlasTextureDimension))
		{
			SDL_DestroySurface(surf);

			if (!create_new_atlas())
				return false;

			atlas = m_atlases[m_currentAtlasCount - 1];
			m_penX = 0;
			m_penY = 0;
			m_rowHeight = 0;
		}

		SDL_Rect dstRect { m_penX, m_penY, gw, gh };
		SDL_LockSurface(surf);
		SDL_UpdateTexture(atlas, &dstRect, surf->pixels, surf->pitch);
		SDL_UnlockSurface(surf);
		SDL_DestroySurface(surf);


		f32 u0 = f32(m_penX) / f32(AtlasTextureDimension);
		f32 v0 = f32(m_penY) / f32(AtlasTextureDimension);
		f32 u1 = f32(m_penX + gw) / f32(AtlasTextureDimension);
		f32 v1 = f32(m_penY + gh) / f32(AtlasTextureDimension);

		glyph.atlas = atlas;
		glyph.uvs[0] = { u0, v0 };
		glyph.uvs[1] = { u1, v0 };
		glyph.uvs[2] = { u1, v1 };
		glyph.uvs[3] = { u0, v1 };
		glyph.size = { cast<f32>(gw), cast<f32>(gh) };

		m_uvs[key] = glyph;
		*outGlyph = &(m_uvs[key]);

		m_penX += gw;
		m_rowHeight = std::max(m_rowHeight, gh);
		return true;
	}

	bool FontGlyphAtlas::create_new_atlas(void)
	{
		if (m_currentAtlasCount >= i32(MaxAtlasCount))
			return false;

		if (!m_renderer)
			return false;

		SDL_Renderer* sdlRenderer = m_renderer->getSDLRenderer();
		if (!sdlRenderer)
			return false;

		SDL_Texture* tex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, AtlasTextureDimension, AtlasTextureDimension);
		if (!tex)
			return false;

		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);


		{ // Clear the texture to transparent
			void* pixels = nullptr;
			i32 pitch = 0;
			if (SDL_LockTexture(tex, nullptr, &pixels, &pitch) == 0)
			{
				memset(pixels, 0x00, pitch * AtlasTextureDimension);
				SDL_UnlockTexture(tex);
			}
		}

		m_atlases[m_currentAtlasCount] = tex;
		m_currentAtlasCount++;

		m_penX = 0;
		m_penY = 0;
		m_rowHeight = 0;

		return true;
	}
}
