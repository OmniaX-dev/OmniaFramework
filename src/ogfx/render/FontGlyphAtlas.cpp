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
		if (!font || m_currentAtlasCount <= 0)
			return false;

		// 1. Extract codepoint from the ostd::String
		auto cps = glyphStr.getUTF8Codepoints();
		if (cps.size() != 1)
			return false; // must be exactly one character

		uint32_t codepoint = cps[0];

		GlyphKey key { codepoint, uint64_t(font), fontSize };
		if (auto it = m_uvs.find(key) != m_uvs.end())
		    return true;

		// 2. Convert the ostd::String to UTF-8 for SDL_ttf
		std::string utf8 = glyphStr.cpp_str();

		// 3. Rasterize glyph using SDL_ttf
		SDL_Surface* surf = TTF_RenderText_Blended(font, utf8.c_str(), utf8.length(), SDL_Color{255, 255, 255, 255});

		if (!surf)
			return false;

		// SDL_Surface* converted = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
		// SDL_DestroySurface(surf);
		// surf = converted;

		int gw = surf->w;
		int gh = surf->h;

		// 4. Get current atlas
		SDL_Texture* atlas = m_atlases[m_currentAtlasCount - 1];

		// SDL_Log("atlas format = %s", SDL_GetPixelFormatName(atlas->format));
		// SDL_Log("surf format = %s", SDL_GetPixelFormatName(surf->format));

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

		uint8_t* pixels = (uint8_t*)surf->pixels;
		int pitch = surf->pitch;

		// for (int y = 0; y < surf->h; y++)
		// {
		// 	uint32_t* row = (uint32_t*)(pixels + y * pitch);

		// 	for (int x = 0; x < surf->w; x++)
		// 	{
		// 		uint32_t px = row[x];

		// 		uint8_t a = (px >> 24) & 0xFF;  // ABGR → A is highest byte
		// 		uint8_t r = (px >> 16) & 0xFF;
		// 		uint8_t g = (px >> 8)  & 0xFF;
		// 		uint8_t b = (px)       & 0xFF;

		// 		std::cout << (int)r << "," << (int)g << "," << (int)b << "," << (int)a << "  ";

		// 		row[x] = (a << 24) | (r << 16) | (g << 8) | b;
		// 	}
		// }

		// 7. Upload glyph bitmap into atlas
		SDL_Rect dstRect { m_penX, m_penY, gw, gh };

		void* lockedPixels = nullptr;
		int lockedPitch = 0;
		if (SDL_LockTexture(atlas, &dstRect, &lockedPixels, &lockedPitch))
		{
			SDL_LockSurface(surf);
		    uint8_t* src = (uint8_t*)surf->pixels;
		    uint8_t* dst = (uint8_t*)lockedPixels;
		    for (int y = 0; y < gh; y++)
		    {
		        memcpy(dst, src, gw * 4);  // 4 bytes per pixel (ARGB8888)
		        src += surf->pitch;
		        dst += lockedPitch;
		    }
			SDL_UnlockSurface(surf);
		    SDL_UnlockTexture(atlas);
		}

		// SDL_UpdateTexture(atlas, &dstRect, surf->pixels, surf->pitch);
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
		uv.size = { static_cast<float>(gw), static_cast<float>(gh) };

		// 9. Store in hashmap
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
		SDL_Texture* tex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, AtlasTextureDimension, AtlasTextureDimension);

		if (!tex)
			return false;

		// 3. Set blend mode (important for alpha glyphs)
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);

		// 4. Clear the texture to transparent
		{
			void* pixels = nullptr;
			int pitch = 0;
			if (SDL_LockTexture(tex, nullptr, &pixels, &pitch) == 0)
			{
				memset(pixels, 0x00, pitch * AtlasTextureDimension);
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

	void FontGlyphAtlas::save_atlas_to_png(SDL_Renderer* renderer, SDL_Texture* atlas, const char* filename)
	{
	    // 1. Get texture dimensions and format
	    float w, h;
	    SDL_GetTextureSize(atlas, &w, &h);
	    int iw = (int)w, ih = (int)h;

	    // 2. Create a target surface
	    SDL_Surface* surf = SDL_CreateSurface(iw, ih, SDL_PIXELFORMAT_ARGB8888);
	    if (!surf) return;

	    // 3. Read pixels back from the texture
	    SDL_Rect rect { 0, 0, iw, ih };

	    // For STREAMING textures, lock/read directly:
	    void* pixels = nullptr;
	    int pitch = 0;
	    if (SDL_LockTexture(atlas, nullptr, &pixels, &pitch))
	    {
	        uint8_t* src = (uint8_t*)pixels;
	        uint8_t* dst = (uint8_t*)surf->pixels;
	        for (int y = 0; y < ih; y++)
	        {
	            memcpy(dst, src, iw * 4);
	            src += pitch;
	            dst += surf->pitch;
	        }
	        SDL_UnlockTexture(atlas);
	    }

	    // 4. Save
	    IMG_SavePNG(surf, filename);  // needs SDL3_image
	    // or: SDL_SaveBMP(surf, "atlas_debug.bmp");

	    SDL_DestroySurface(surf);
	}
}
