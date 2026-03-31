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

#include <ogfx/utils/SDLInclude.hpp>
#include <ostd/utils/Signals.hpp>
#include <ostd/io/IOHandlers.hpp>

namespace ogfx
{
	class TTFRenderer
	{
		public: struct tErrors
		{
			inline static constexpr int32_t NoError = 0;
			inline static constexpr int32_t FailedToLoad = 1;
			inline static constexpr int32_t FailedToOpenFontFile = 2;
			inline static constexpr int32_t Uninitialized = 3;
			inline static constexpr int32_t NullRenderer = 4;
			inline static constexpr int32_t InvalidState = 5;
			inline static constexpr int32_t TTFRenderTextBlendedFail = 6;
			inline static constexpr int32_t TTFCreateTextureFromSurfaceFail = 7;
			inline static constexpr int32_t NullFont = 8;
			inline static constexpr int32_t NoFont = 9;
			inline static constexpr int32_t FailedToOpenFontByteStrean = 10;
		};
		public:
			TTFRenderer(void) = default;
			~TTFRenderer(void);
			inline TTFRenderer(SDL_Renderer* renderer) { init(renderer); }
			int32_t init(SDL_Renderer* renderer);
			int32_t loadDefaultFont(int32_t fontSize = 0);
			void closeFont(void);
			int32_t openFont(const ostd::String& fontPath, int32_t fontSize = 0);
			int32_t openFont(const ostd::UByte resource_data[], uint32_t data_size, int32_t fontSize = 0);
			int32_t setFontSize(int32_t fontSize);
			void renderText(const ostd::String& message, int32_t x, int32_t y, const ostd::Color& color, int32_t fontSize = 0);
			void renderCenteredText(const ostd::String& message, int32_t center_x, int32_t center_y, const ostd::Color& color, int32_t fontSize = 0);
			ostd::IPoint getStringDimensions(const ostd::String& message, int32_t fontSize = 0);
			inline bool isInitialized(void) { return TTFRenderer::m_initialized; }
			inline bool hasOpenFont(void) { return m_fontOpen; }
			inline TTF_Font* getSDLFont(void) { return m_font;  }
			inline bool isValid(void) { return TTFRenderer::m_initialized && m_fontOpen && (m_font != nullptr || m_fontFromMemory) && m_renderer != nullptr; }
			inline int32_t geterrorState(void) { return m_errorState; }
			inline int32_t getFontSize(void) { return m_fontSize; }

		private:
			void print_ttf_error(const ostd::String& funcName);
			inline int32_t set_error_state(int32_t err) { m_errorState = err; return m_errorState; }

		private:
			ostd::ConsoleOutputHandler m_out;
			bool m_initialized { false };
			bool m_fontOpen { false };
			TTF_Font* m_font { nullptr };
			SDL_Renderer* m_renderer { nullptr };
			int32_t m_errorState { tErrors::NoError };
			int32_t m_fontSize { DefaultFontSize };
			bool m_fontFromMemory { false };

			inline static constexpr int32_t DefaultFontSize { 16 };
	};
}
