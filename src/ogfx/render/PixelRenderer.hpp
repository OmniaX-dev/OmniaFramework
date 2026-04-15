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


namespace ogfx
{
	class WindowCore;
	class PixelRenderer : public ostd::BaseObject
	{
		public: class TextRenderer
		{
			private:
				inline static stdumap<char, i32> characterMap;

			public:
				static void initialize(void);
				static bool drawString(const String& str, u32 column, u32 row, u32* screenPixels, i32 screenWidth, i32 screenHeight, u32* fontPixels,  const ostd::Color& color = { 255, 255, 255, 255 }, const ostd::Color& background = { 255, 255, 255, 0 });

			private:
				static i32 getCharacterIndex(char c);
				static ostd::Color applyTint(const ostd::Color& baseColor, const ostd::Color& tintColor);
				static void drawCharacter(u8* screenPixels, i32 screenWidth, i32 screenHeight, u8* fontPixels, i32 x, i32 y, char c, const ostd::Color& color = { 255, 255, 255, 255 }, const ostd::Color& background = { 255, 255, 255, 0 });

			public:
				inline static constexpr i32 FONT_CHAR_W = 11;
				inline static constexpr i32 FONT_CHAR_H = 26;
				inline static constexpr i32 FONT_V_CHARS = 6;
				inline static constexpr i32 FONT_H_CHARS = 16;
				inline static constexpr i32 CONSOLE_CHARS_H = 90;
				inline static constexpr i32 CONSOLE_CHARS_V = 21;

				inline static i8 s_cursor_pos_x = 0;
		};
		public: class Font
		{
			public:
				inline Font(void) {  }
				inline Font(const String& fontPath)
				{
					init(fontPath);
				}
				inline void init(const String& fontPath)
				{
					ostd::ConsoleOutputHandler out;
					m_fontSurface = SDL_LoadBMP(fontPath.c_str());
					if (m_fontSurface == NULL)
						out.bg(ostd::ConsoleColors::Red).p("Error loading pixel font.").reset().nl();
					m_fontPixels = (u32*)m_fontSurface->pixels;
				}
				inline ~Font(void)
				{
					SDL_DestroySurface(m_fontSurface);
				}

			public:
				SDL_Surface* m_fontSurface { nullptr };
				u32* m_fontPixels { nullptr };
		};
		public:
			inline PixelRenderer(void) { invalidate(); }
			~PixelRenderer(void);
			void initialize(WindowCore& parent);
			void handleSignal(ostd::Signal& signal) override;
			void updateBuffer(void);
			void displayBuffer(void);
			inline u32* getScreenPixels(void) { return m_pixels; }

			void clear(const ostd::Color& color);

		private:
			u32* m_pixels { nullptr };
			SDL_Texture* m_texture { nullptr };
			WindowCore* m_parent { nullptr };
			i32 m_windowWidth { 0 };
			i32 m_windowHeight { 0 };
	};
}
