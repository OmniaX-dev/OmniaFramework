#include "PixelRenderer.hpp"
#include "WindowBase.hpp"
#include <ostd/Utils.hpp>

namespace ogfx
{
	void PixelRenderer::TextRenderer::initialize(void)
	{
		for (char c = ' '; c <= '~'; c++)
			characterMap[c] = getCharacterIndex(c);
	}

	bool PixelRenderer::TextRenderer::drawString(ostd::String str, uint32_t column, uint32_t row, uint32_t* screenPixels, int32_t screenWidth, int32_t screenHeight, uint32_t* fontPixels,  ostd::Color color, ostd::Color background)
	{
		ostd::String se(str);
		if (se == "") return false;
		if (row >= CONSOLE_CHARS_V) return false;
		if (column >= CONSOLE_CHARS_H) return false;
		if (column + str.len() > CONSOLE_CHARS_H) return false;
		int32_t x = column * FONT_CHAR_W;
		int32_t y = row * FONT_CHAR_H;
		for (auto& c : str)
		{
			drawCharacter((uint8_t*)screenPixels, screenWidth, screenHeight, (uint8_t*)fontPixels, x, y, c, color, background);
			x += FONT_CHAR_W;
		}
		s_cursor_pos_x = x;
		if (s_cursor_pos_x >= CONSOLE_CHARS_H)
			s_cursor_pos_x = 0;
		return true;
	}

	int32_t PixelRenderer::TextRenderer::getCharacterIndex(char c)
	{
		using namespace ostd;

		int32_t charIndex = (int)c - 32;
		IPoint charCoords = CONVERT_1D_2D(charIndex, FONT_H_CHARS);
		charCoords.x *= FONT_CHAR_W * 4;
		charCoords.y *= FONT_CHAR_H;
		charIndex = CONVERT_2D_1D(charCoords.x, charCoords.y, (FONT_H_CHARS * FONT_CHAR_W * 4));

		return charIndex;
	}

	ostd::Color PixelRenderer::TextRenderer::applyTint(ostd::Color baseColor, ostd::Color tintColor)
	{
		auto nBase = baseColor.getNormalizedColor();
		auto nTint = tintColor.getNormalizedColor();

		float r = nBase.r * nTint.r;
		float g = nBase.r * nTint.g;
		float b = nBase.r * nTint.b;

		ostd::Color::FloatCol nTinted(r, g, b, 1.0f);

		return ostd::Color(nTinted);
	}

	void PixelRenderer::TextRenderer::drawCharacter(uint8_t* screenPixels, int32_t screenWidth, int32_t screenHeight, uint8_t* fontPixels, int32_t x, int32_t y, char c, ostd::Color color, ostd::Color background)
	{
		using namespace ostd;
		int32_t charIndex = characterMap[c];
		IPoint charCoords = CONVERT_1D_2D(charIndex, (FONT_CHAR_W * FONT_H_CHARS * 4));

		int32_t screenx = x * 4, screeny = y;

		ostd::Color tintedColor;

		bool applyBackground = false;
		for (int32_t y = charCoords.y; y < charCoords.y + (FONT_CHAR_H); y += 1)
		{
			for (int32_t x = charCoords.x; x < charCoords.x + (FONT_CHAR_W * 4); x += 4)
			{
				int32_t index = CONVERT_2D_1D(x, y, (FONT_CHAR_W * FONT_H_CHARS * 4));
				int32_t screenIndex = CONVERT_2D_1D(screenx, screeny, (screenWidth * 4));
				screenx += 4;
				if (fontPixels[index] == 0x00 && fontPixels[index + 1] == 0x00 && fontPixels[index + 2] == 0x00)
				{
					if (background.a == 0)
						continue;
					applyBackground = true;
				}
				if (applyBackground)
				{
					screenPixels[screenIndex + 0] = background.b;
					screenPixels[screenIndex + 1] = background.g;
					screenPixels[screenIndex + 2] = background.r;
					screenPixels[screenIndex + 3] = 255;
					applyBackground = false;
					continue;
				}
				tintedColor = applyTint({ fontPixels[index], fontPixels[index + 1], fontPixels[index + 2], 255 }, color);
				screenPixels[screenIndex + 0] = tintedColor.b;
				screenPixels[screenIndex + 1] = tintedColor.g;
				screenPixels[screenIndex + 2] = tintedColor.r;
				screenPixels[screenIndex + 3] = fontPixels[index + 3];
			}
			screeny += 1;
			screenx = x * 4;
		}
	}




	PixelRenderer::~PixelRenderer(void)
	{
		if (isInvalid()) return;
		ostd::Utils::destroyArray(m_pixels);
		SDL_DestroyTexture(m_texture);
	}

	void PixelRenderer::initialize(WindowBase& parent)
	{
		if (isValid()) return; //TODO: Error
		if (!parent.isValid() || !parent.isInitialized())
			return; //TODO: Error
		m_parent = &parent;
		m_pixels = ostd::Utils::createArray<uint32_t>(parent.getWindowWidth() * parent.getWindowHeight());
		m_texture = SDL_CreateTexture(parent.getSDLRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, parent.getWindowWidth(), parent.getWindowHeight());
		m_windowWidth = parent.getWindowWidth();
		m_windowHeight = parent.getWindowHeight();
		setTypeName("ogfx::PixelRenderer");
		enableSignals();
		connectSignal(ostd::tBuiltinSignals::WindowResized);
		validate();
	}

	void PixelRenderer::handleSignal(ostd::tSignal& signal)
	{
		if (isInvalid()) return;
		if (signal.ID == ostd::tBuiltinSignals::WindowResized)
		{
			m_pixels = ostd::Utils::resizeArray<uint32_t>(m_pixels, m_parent->getWindowWidth() * m_parent->getWindowHeight());
			SDL_DestroyTexture(m_texture);
			m_texture = SDL_CreateTexture(m_parent->getSDLRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, m_parent->getWindowWidth(), m_parent->getWindowHeight());
			m_windowWidth = m_parent->getWindowWidth();
			m_windowHeight = m_parent->getWindowHeight();
			updateBuffer();
			displayBuffer();
		}
	}

	void PixelRenderer::updateBuffer(void)
	{
		if (isInvalid()) return;
		SDL_UpdateTexture(m_texture, NULL, m_pixels, m_windowWidth * 4);
	}

	void PixelRenderer::displayBuffer(void)
	{
		if (isInvalid()) return;
		SDL_Rect rect { 0, 0, m_windowWidth, m_windowHeight };
		SDL_RenderCopy(m_parent->getSDLRenderer(), m_texture, NULL, &rect);
	}

	void PixelRenderer::clear(const ostd::Color& color)
	{
		if (isInvalid()) return;
		for (int32_t y = 0; y < m_windowHeight; y++)
		{
			for (uint32_t x = 0; x < m_windowWidth; x++)
			{
				int32_t index = CONVERT_2D_1D(x, y, m_windowWidth);
				m_pixels[index] = color.asInteger(ostd::Color::eColorFormat::ARGB);
			}
		}
	}
}
