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
#include <ogfx/utils/SDLInclude.hpp>
#include <ostd/math/Geometry.hpp>

namespace ogfx
{
	class BasicRenderer2D;
	class Image : public ostd::Object
	{
		public:
			inline Image(void) { invalidate(); }
			inline Image(const String& filePath, BasicRenderer2D& gfx) { loadFromFile(filePath, gfx); }
			inline ~Image(void) { destroy(); }
			// An Image owns its texture: copies would destroy it twice.
			Image(const Image&) = delete;
			Image& operator=(const Image&) = delete;
			Image(Image&& other) noexcept;
			Image& operator=(Image&& other) noexcept;
			void destroy(void);
			Image& loadFromFile(const String& filePath, BasicRenderer2D& gfx);
			// Creates a blank width x height image whose pixels are set from memory with
			// updatePixels() (a streaming texture; 32-bit 0xAARRGGBB pixels, straight alpha).
			Image& create(BasicRenderer2D& gfx, i32 width, i32 height);
			// Uploads `area` (all of the image when empty) from `pixels`, a buffer covering the
			// whole image (getSize().x pixels per row). Only for images made with create().
			bool updatePixels(const u32* pixels, const IRect& area = { 0, 0, 0, 0 });
			// Filtering when the image is drawn scaled: smooth (bilinear, the default) or
			// nearest-neighbour (pixel art).
			void setSmoothFiltering(bool smooth);
			inline bool isSmoothFiltering(void) const { return m_smoothFiltering; }
			// True for an image made with create() whose pixels were lost (a GPU device reset
			// re-created its texture blank): upload all of them again. Cleared by a full
			// updatePixels().
			inline bool contentsLost(void) const { return m_contentsLost; }
			inline Vec2 getSize(void) const { return { m_width, m_height }; }
			inline bool isLoaded(void) const { return m_loaded; }
			inline SDL_Texture* getSDLTexture(void) const { return m_sdl_texture; }
			inline String getFilePath(void) const { return m_filePath; }

		private:
			// Called by the renderer the image was loaded with.
			void __reload(void);              // device reset: the texture is gone, load it again
			void __release_texture(void);     // the SDL renderer is about to be destroyed
			void __detach_from_renderer(void); // the BasicRenderer2D itself is being destroyed
			bool create_streaming_texture(void);

		private:
			BasicRenderer2D* m_renderer { nullptr };
			SDL_Texture* m_sdl_texture { nullptr };
			String m_filePath { "" };
			f32 m_width { 0 };
			f32 m_height { 0 };
			bool m_loaded { false };
			bool m_streaming { false };
			bool m_smoothFiltering { true };
			bool m_contentsLost { false };

			friend class BasicRenderer2D;
	};
}
