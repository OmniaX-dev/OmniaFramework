#ifndef __BITMAP_FONT_HPP__
#define __BITMAP_FONT_HPP__

#include <ostd/BaseObject.hpp>
#include <ostd/Types.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Defines.hpp>

namespace ogfx 
{
	using namespace ostd; //TODO: Remove from header

	class BitmapFont : public BaseObject
	{
		public:
			inline BitmapFont(void) { invalidate(); }
			inline BitmapFont(String filePath, bool monospace = false) { create(filePath, monospace); }
			BitmapFont& create(String filePath, bool monospace = false);
			BitmapFont& create(ResourceID texture, bool monospace = false);
			TextureAtlasIndex getChar(unsigned char c, Rectangle& outRect);
			float getSpaceWidth(void);
			inline ResourceID getTexture(void) { return m_texture; }
			inline float getBaseCharHeight(void) { return m_charSize.y; }
			void setSpaceWidth(float w);
			inline bool isMonoSpace(void) { return m_monoSpace; }
			void enableMonoSpace(bool mono = true);

		private:
			void calc_char_bounds(void);
			void __load(void);
			Rectangle test_grid(uint8_t** grid, int32_t width, int32_t height, uint8_t invalid_alpha = 0);
			
		private:
			ResourceID m_texture;
			std::vector<TextureAtlasIndex> m_characters;
			std::vector<Rectangle> m_charBounds;
			Vec2 m_charSize;
			Vec2 m_textureSize;
			float m_spaceWidth { 20.0f };
			float m_maxCharWidth { 0.0f };
			bool m_monoSpace { false };

		
		public:
			inline static constexpr int32_t ERR_INVALID_TEXTURE = OX_BITMAPFONT_ERR_MASK + 0x0001;
			inline static constexpr int32_t ERR_INVALID_TEXTURE_SIZE = OX_BITMAPFONT_ERR_MASK + 0x0002;
	};
}

#endif
