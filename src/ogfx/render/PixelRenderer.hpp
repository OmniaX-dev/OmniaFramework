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

#include <ostd/io/IOHandlers.hpp>
#include <ostd/string/String.hpp>
#include <ostd/data/Color.hpp>
#include <ogfx/utils/SDLInclude.hpp>
#include <unordered_map>

namespace ogfx
{
	class WindowCore;
	class PixelRenderer : public ostd::BaseObject
	{
		public: class TextRenderer
		{
			private:
				inline static std::unordered_map<char, int32_t> characterMap;

			public:
				static void initialize(void);
				static bool drawString(ostd::String str, uint32_t column, uint32_t row, uint32_t* screenPixels, int32_t screenWidth, int32_t screenHeight, uint32_t* fontPixels,  ostd::Color color = { 255, 255, 255, 255 }, ostd::Color background = { 255, 255, 255, 0 });

			private:
				static int32_t getCharacterIndex(char c);
				static ostd::Color applyTint(ostd::Color baseColor, ostd::Color tintColor);
				static void drawCharacter(uint8_t* screenPixels, int32_t screenWidth, int32_t screenHeight, uint8_t* fontPixels, int32_t x, int32_t y, char c, ostd::Color color = { 255, 255, 255, 255 }, ostd::Color background = { 255, 255, 255, 0 });

			public:
				inline static constexpr int32_t FONT_CHAR_W = 11;
				inline static constexpr int32_t FONT_CHAR_H = 26;
				inline static constexpr int32_t FONT_V_CHARS = 6;
				inline static constexpr int32_t FONT_H_CHARS = 16;
				inline static constexpr int32_t CONSOLE_CHARS_H = 90;
				inline static constexpr int32_t CONSOLE_CHARS_V = 21;

				inline static int8_t s_cursor_pos_x = 0;
		};
		public: class Font
		{
			public:
				inline Font(void) {  }
				inline Font(const ostd::String& fontPath)
				{
					init(fontPath);
				}
				inline void init(const ostd::String& fontPath)
				{
					ostd::ConsoleOutputHandler out;
					m_fontSurface = SDL_LoadBMP(fontPath.c_str());
					if (m_fontSurface == NULL)
						out.bg(ostd::ConsoleColors::Red).p("Error loading pixel font.").reset().nl();
					m_fontPixels = (uint32_t*)m_fontSurface->pixels;
				}
				inline ~Font(void)
				{
					SDL_DestroySurface(m_fontSurface);
				}

			public:
				SDL_Surface* m_fontSurface { nullptr };
				uint32_t* m_fontPixels { nullptr };
		};
		public:
			inline PixelRenderer(void) { invalidate(); }
			~PixelRenderer(void);
			void initialize(WindowCore& parent);
			void handleSignal(ostd::tSignal& signal) override;
			void updateBuffer(void);
			void displayBuffer(void);
			inline uint32_t* getScreenPixels(void) { return m_pixels; }

			void clear(const ostd::Color& color);

		private:
			uint32_t* m_pixels { nullptr };
			SDL_Texture* m_texture { nullptr };
			WindowCore* m_parent { nullptr };
			int32_t m_windowWidth { 0 };
			int32_t m_windowHeight { 0 };
	};
}
