#include "BitmapFont.hpp"
#include <ogfx/ResourceManager.hpp>
#include <ogfx/Texture.hpp>
#include <ogfx/Errors.hpp>
#include <Logger.hpp>

//TODO: Add errors

namespace ogfx
{
	using namespace ostd;

	BitmapFont& BitmapFont::create(String filePath, bool monospace)
	{
		m_texture = ResourceManager::loadTexture(filePath, true, GL_NEAREST, GL_NEAREST);
		m_monoSpace = monospace;
		__load();
		return *this;
	}

	BitmapFont& BitmapFont::create(ResourceID texture, bool monospace)
	{
		m_texture = texture;
		m_monoSpace = monospace;
		__load();
		return *this;
	}

	TextureAtlasIndex BitmapFont::getChar(unsigned char c, Rectangle& outRect)
	{
		outRect = m_charBounds[(uint32_t)c];
		if (m_monoSpace) outRect.w = m_maxCharWidth + 4;
		return m_characters[(uint32_t)c];
	}
	
	float BitmapFont::getSpaceWidth(void)
	{
		if (m_monoSpace) return m_maxCharWidth;
		return m_spaceWidth;
	}

	void BitmapFont::setSpaceWidth(float w)
	{
		m_spaceWidth = w;
		if (!m_monoSpace)
			m_charBounds[(uint32_t)' '] = { (m_charSize.x - m_spaceWidth) / 2.0f, 5.0f, m_spaceWidth, m_charSize.y - 10 };
	}

	void BitmapFont::enableMonoSpace(bool mono)
	{
		if (isInvalid()) return; //TODO: Error
		create(m_texture, mono);
	}

	void BitmapFont::calc_char_bounds(void)
	{
		uint32_t xtiles = m_textureSize.x / m_charSize.x;
		uint32_t ytiles = m_textureSize.y / m_charSize.y;
		const auto& pixels = ResourceManager::getTexture(m_texture).getPixelData();
		uint8_t** sub_pixels = new uint8_t*[m_charSize.y];
		for (int32_t row = 0; row < m_charSize.y; row++)
			sub_pixels[row] = new uint8_t[m_charSize.x];
		for (int32_t y = ytiles - 1; y >= 0; y--)
		{
			for (int32_t x = 0; x < xtiles; x++)
			{
				int32_t xx = (x * m_charSize.x * 4);
				int32_t yy = (y * m_charSize.y * 4);
				for (int32_t row = 0; row < m_charSize.y; row++)
				{
					for (int32_t col = 0; col < m_charSize.x; col++)
					{
						int32_t real_x = xx + (col * 4);
						int32_t real_y = yy + (row * 4);
						int32_t _1d_coord = CONVERT_2D_1D(real_x, real_y, (uint32_t)m_textureSize.x);
						sub_pixels[row][col] = pixels[_1d_coord];
					}
				}
				auto rect = test_grid(sub_pixels, m_charSize.x, m_charSize.y);
				// std::cout << (char)m_charBounds.size() << "\n";
				// std::cout << rect.getPosition() << "\n";
				// std::cout << rect.getSize() << "\n\n";
				if (rect.w > m_maxCharWidth)
					m_maxCharWidth = rect.w;
				m_charBounds.push_back(rect);
			}
		}
		if (m_monoSpace)
		{
			for (auto& cb : m_charBounds)
			{
				float oldw = cb.w;
				float newx_offset = (m_maxCharWidth - oldw) / 2;
				cb.w = m_maxCharWidth;
				cb.x -= newx_offset;
				if (cb.x < 0) cb.x = 0;
			}
		}
	}

	Rectangle BitmapFont::test_grid(uint8_t** grid, int32_t width, int32_t height, uint8_t invalid_alpha)
	{
		int32_t top = 0, bottom = height, left = 0, right = width;
		bool found = false;
		// for (int32_t row = 0; row < height; row++)
		// {
		// 	for (int32_t col = 0; col < width; col++)
		// 	{
		// 		if (grid[row][col] != invalid_alpha)
		// 		{
		// 			top = row;
		// 			found = true;
		// 			break;
		// 		}
		// 	}
		// 	if (found) break;
		// }
		// if (!found) return { 0, 0, 0, 0 }; //TODO: ERROR
		found = false;
		for (int32_t col = 0; col < width; col++)
		{
			for (int32_t row = 0; row < height; row++)
			{
				if (grid[row][col] != invalid_alpha)
				{
					left = col;
					found = true;
					break;
				}
			}
			if (found) break;
		}
		if (!found) return { 0, 0, 0, 0 }; //TODO: ERROR
		found = false;
		for (int32_t col = width - 1; col >= 0; col--)
		{
			for (int32_t row = 0; row < width; row++)
			{
				if (grid[row][col] != invalid_alpha)
				{
					right = col;
					found = true;
					break;
				}
			}
			if (found) break;
		}
		if (!found) return { 0, 0, 0, 0 }; //TODO: ERROR
		// found = false;
		// for (int32_t row = height - 1; row >= 0; row--)
		// {
		// 	for (int32_t col = 0; col < width; col++)
		// 	{
		// 		if (grid[row][col] != invalid_alpha)
		// 		{
		// 			bottom = row;
		// 			found = true;
		// 			break;
		// 		}
		// 	}
		// 	if (found) break;
		// }
		Rectangle rect = Rectangle((float)left, (float)top, (float)(right - left), (float)(bottom - top));
		if (rect.x < 0) rect.x = 0; 
		if (rect.y < 0) rect.y = 0;
		if (rect.w > width) rect.w = width;
		if (rect.h > height) rect.h = height;
		return { rect.x, 0, rect.w, (float)height };
		return rect;
	}
	
	// void BitmapFont::calc_char_bounds(void)
	// {
	// 	uint32_t xtiles = m_textureSize.x / m_charSize.x;
	// 	uint32_t ytiles = m_textureSize.y / m_charSize.y;
	// 	const auto& pixels = ResourceManager::getTexture(m_texture).getPixelData();
	// 	for (int32_t y = ytiles - 1; y >= 0; y--)
	// 	{
	// 		for (int32_t x = 0; x < xtiles; x++)
	// 		{
	// 			int32_t xx = (x * m_charSize.x * 4);
	// 			int32_t yy = (y * m_charSize.y * 4);
	// 			float bl = m_charSize.x, bt = m_charSize.y, br = 0.0f, bb = 0.0f;
	// 			int32_t revy = 0;
	// 			for (int32_t ty = m_charSize.y - 1; ty >= 0; ty--, revy++)
	// 			{
	// 				for (int32_t tx = 0; tx < m_charSize.x; tx++)
	// 				{
	// 					uint32_t px = xx + (tx * 4);
	// 					uint32_t py = yy + (ty * 4);
	// 					uint8_t alpha = pixels[CONVERT_2D_1D(px, py, (uint32_t)m_textureSize.x)];
	// 					if (alpha != 0)
	// 					{
	// 						if (revy < m_charSize.y / 2)
	// 						{	if (revy < bt) bt = revy;	}
	// 						else if (revy > bb) bb = revy;

	// 						if (tx < m_charSize.x / 2)
	// 						{	if (tx < bl) bl = tx;	}
	// 						else if (tx > br) br = tx;
	// 					}
	// 				}
	// 			}
	// 			Rectangle rect(bl - 2, bt - 2, br - bl + 4, bb - bt + 4);
	// 			std::cout << (char)m_charBounds.size() << "\n";
	// 			std::cout << rect.getPosition() << "\n";
	// 			std::cout << rect.getSize() << "\n\n";
	// 			if (rect.x < 0) rect.x = 0;
	// 			if (rect.y < 0) rect.y = 0;
	// 			if (rect.w < 10) rect.w = 10;
	// 			if (rect.h < 10) rect.h = 10;
	// 			if (rect.w > m_maxCharWidth)
	// 				m_maxCharWidth = rect.w;
	// 			m_charBounds.push_back(rect);
	// 		}
	// 	}
	// 	if (m_monoSpace)
	// 	{
	// 		for (auto& cb : m_charBounds)
	// 		{
	// 			float oldw = cb.w;
	// 			float newx = m_maxCharWidth - oldw;
	// 			newx = cb.x - (newx / 2.0f);
	// 			cb.w = m_maxCharWidth;
	// 			if (newx >= 0) cb.x = newx;
	// 		}
	// 	}
	// }

	void BitmapFont::__load(void)
	{
		invalidate();
		m_characters.clear();
		m_charBounds.clear();
		m_maxCharWidth = 0.0f;
		if (m_texture == ResourceManager::InvalidResource)
		{
			ErrorHandler::pushError(BitmapFont::ERR_INVALID_TEXTURE);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("%s", err_str.c_str());
			return;
		}
		auto& tex = ResourceManager::getTexture(m_texture);
		if (tex.getWidth() % 16 != 0 || tex.getHeight() % 16 != 0)
		{
			ErrorHandler::pushError(BitmapFont::ERR_INVALID_TEXTURE_SIZE);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("%s", err_str.c_str());
			return;
		}
		m_charSize = { (float)(tex.getWidth() / 16.0f), (float)(tex.getHeight() / 16.0f) };
		m_textureSize = { (float)tex.getWidth(), (float)tex.getHeight() };
		calc_char_bounds();
		if (!m_monoSpace)
			m_charBounds[(uint32_t)' '] = { (m_charSize.x - m_spaceWidth) / 2.0f, 5.0f, m_spaceWidth, m_charSize.y - 10 };
		for (uint32_t y = 0; y < 16; y++)
		{
			for (uint32_t x = 0; x < 16; x++)
			{
				auto& bounds = m_charBounds[CONVERT_2D_1D(x, y, 16)];
				m_characters.push_back(tex.addTileInfo((x * m_charSize.x) + bounds.x,
													   (y * m_charSize.y) + bounds.y,
													    bounds.w,
														bounds.h));
			}
		}
		setTypeName("ox::BitmapFont");
		validate();
	}
}
