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

#pragma once

#include <ostd/string/String.hpp>
#include <ostd/math/Geometry.hpp>
#include <ogfx/utils/SDLInclude.hpp>

namespace ogfx
{
	class BasicRenderer2D;
	class FontGlyphAtlas
	{
		public: struct GlyphInfo
		{
			ostd::Vec2 uvs[4];
			SDL_Texture* atlas { nullptr };
			ostd::Vec2 size;
			uint32_t codepoint { 0 };
			int32_t advance { 0 };
		};
		struct GlyphKey
		{
		    uint32_t codepoint;
		    uint64_t fontID;
		    uint32_t pixelSize;

		    bool operator==(const GlyphKey& other) const noexcept
			{
		        return codepoint == other.codepoint &&
		               fontID == other.fontID &&
		               pixelSize == other.pixelSize;
		    }
		};
		struct GlyphKeyHasher
		{
			size_t operator()(const GlyphKey& k) const noexcept
			{
			    size_t h = std::hash<uint32_t>()(k.codepoint);
			    h ^= std::hash<uint64_t>()(k.fontID) + 0x9e3779b9 + (h << 6) + (h >> 2);
			    h ^= std::hash<uint32_t>()(k.pixelSize) + 0x9e3779b9 + (h << 6) + (h >> 2);
			    return h;
			}
		};
		public:
			inline FontGlyphAtlas(void) {  }
			inline FontGlyphAtlas(BasicRenderer2D& renderer) { init(renderer); }
			FontGlyphAtlas init(BasicRenderer2D& renderer);
			const std::vector<const GlyphInfo*> processString(const ostd::String& str, TTF_Font* font, uint32_t fontSize);

		private:
			bool rasterize_glyph(const ostd::String& glyphStr, TTF_Font* font, uint32_t fontSize, const GlyphInfo** outGlyph);
			bool create_new_atlas(void);

		public:
			inline static constexpr uint32_t AtlasTextureDimension { 8192 };
			inline static constexpr uint32_t MaxAtlasCount { 16 };

		public:
			std::unordered_map<GlyphKey, GlyphInfo, GlyphKeyHasher> m_uvs;
			SDL_Texture* m_atlases[FontGlyphAtlas::MaxAtlasCount];
			int32_t m_currentAtlasCount { 0 };
			BasicRenderer2D* m_renderer { nullptr };
			int32_t m_penX { 0 };
		    int32_t m_penY { 0 };
		    int32_t m_rowHeight { 0 };
	};
}
