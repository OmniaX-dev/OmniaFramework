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

#define DEBUG(n) std::cout << (int)n << "\n";

namespace ogfx
{
	FontGlyphAtlas FontGlyphAtlas::init(BasicRenderer2D& renderer)
	{
		m_renderer = &renderer;
		m_uvs.reserve(4096);
		for (int32_t i = 0; i < FontGlyphAtlas::MaxAtlasCount; i++)
			m_atlases[i] = nullptr;
		m_currentAtlasCount = 0;
		create_new_atlas();
		return *this;
	}

	bool FontGlyphAtlas::rasterize_glyph(const ostd::String& glyphStr, TTF_Font* font, uint32_t fontSize)
	{
		DEBUG(1);
		std::cout << (uint64_t)font << "\n";
		if (!font || m_currentAtlasCount <= 0)
			return false;

		DEBUG(2);
		// 1. Extract codepoint from the ostd::String
		auto cps = glyphStr.getUTF8Codepoints();
		if (cps.size() != 1)
			return false; // must be exactly one character

		DEBUG(3);
		uint32_t codepoint = cps[0];

		// 2. Convert the ostd::String to UTF-8 for SDL_ttf
		std::string utf8 = glyphStr.cpp_str();

		// 3. Rasterize glyph using SDL_ttf
		SDL_Surface* surf = TTF_RenderText_Blended(font, utf8.c_str(), utf8.length(), SDL_Color{255, 255, 255, 255});

		if (!surf)
			return false;

		DEBUG(4);
		int gw = surf->w;
		int gh = surf->h;

		// 4. Get current atlas
		SDL_Texture* atlas = m_atlases[m_currentAtlasCount - 1];

		// 5. Shelf packing: move to next row if needed
		if (m_penX + gw > int(AtlasTextureDimension))
		{
			m_penX = 0;
			m_penY += m_rowHeight;
			m_rowHeight = 0;
		}

		// 6. If still doesn't fit → create new atlas
		if (m_penY + gh > int(AtlasTextureDimension))
		{
			SDL_DestroySurface(surf);

			if (!create_new_atlas())
				return false; // no more atlases available

			atlas = m_atlases[m_currentAtlasCount - 1];
			m_penX = 0;
			m_penY = 0;
			m_rowHeight = 0;
		}

		DEBUG(5);
		// 7. Upload glyph bitmap into atlas
		SDL_Rect dstRect { m_penX, m_penY, gw, gh };
		SDL_UpdateTexture(atlas, &dstRect, surf->pixels, surf->pitch);
		SDL_DestroySurface(surf);

		// 8. Compute UVs
		float u0 = float(m_penX) / float(AtlasTextureDimension);
		float v0 = float(m_penY) / float(AtlasTextureDimension);
		float u1 = float(m_penX + gw) / float(AtlasTextureDimension);
		float v1 = float(m_penY + gh) / float(AtlasTextureDimension);

		GlyphUV uv;
		uv.atlas = atlas;
		uv.uvs[0] = { u0, v0 };
		uv.uvs[1] = { u1, v0 };
		uv.uvs[2] = { u1, v1 };
		uv.uvs[3] = { u0, v1 };

		// 9. Store in hashmap
		GlyphKey key { codepoint, uint64_t(font), fontSize };
		m_uvs[key] = uv;

		// 10. Advance shelf packing cursor
		m_penX += gw;
		m_rowHeight = std::max(m_rowHeight, gh);
		return true;
	}

	bool FontGlyphAtlas::create_new_atlas(void)
	{
		// 1. Check if we can create another atlas
		if (m_currentAtlasCount >= int(MaxAtlasCount))
			return false;

		if (!m_renderer)
			return false;

		SDL_Renderer* sdlRenderer = m_renderer->getSDLRenderer();
		if (!sdlRenderer)
			return false;

		// 2. Create the texture
		SDL_Texture* tex = SDL_CreateTexture(
			sdlRenderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			AtlasTextureDimension,
			AtlasTextureDimension
		);

		if (!tex)
			return false;

		// 3. Set blend mode (important for alpha glyphs)
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

		// 4. Clear the texture to transparent
		{
			void* pixels = nullptr;
			int pitch = 0;
			if (SDL_LockTexture(tex, nullptr, &pixels, &pitch) == 0)
			{
				memset(pixels, 0, pitch * AtlasTextureDimension);
				SDL_UnlockTexture(tex);
			}
		}

		// 5. Store the atlas
		m_atlases[m_currentAtlasCount] = tex;
		m_currentAtlasCount++;

		// 6. Reset packing cursor for this new atlas
		m_penX = 0;
		m_penY = 0;
		m_rowHeight = 0;

		return true;
	}

	bool FontGlyphAtlas::save_atlas_png(int atlasIndex, const char* filename)
	{
		if (atlasIndex < 0 || atlasIndex >= m_currentAtlasCount)
			return false;

		SDL_Texture* tex = m_atlases[atlasIndex];
		if (!tex)
			return false;

		// --- 1. Query texture properties (SDL3 way) ---
		SDL_PropertiesID props = SDL_GetTextureProperties(tex);
		if (!props)
			return false;

		int width  = (int)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
		int height = (int)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);
		Uint32 format = (Uint32)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_RGBA32);

		if (width <= 0 || height <= 0)
			return false;

		// --- 2. Create a surface to receive the pixels ---
		SDL_Surface* surf = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
		if (!surf)
			return false;

		// --- 3. Lock texture and copy pixels ---
		void* pixels = nullptr;
		int pitch = 0;

		if (!SDL_LockTexture(tex, nullptr, &pixels, &pitch))
		{
			SDL_DestroySurface(surf);
			return false;
		}

		// Copy row-by-row (pitch may differ)
		uint8_t* dst = static_cast<uint8_t*>(surf->pixels);
		uint8_t* src = static_cast<uint8_t*>(pixels);

		for (int y = 0; y < height; y++)
			memcpy(dst + y * surf->pitch, src + y * pitch, width * 4);

		SDL_UnlockTexture(tex);

		// --- 4. Save PNG ---
		bool ok = (IMG_SavePNG(surf, filename) == 0);

		SDL_DestroySurface(surf);
		return ok;
	}
}
