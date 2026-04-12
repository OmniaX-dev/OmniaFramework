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

#include <ogfx/render/FontUtils.hpp>
#include <ogfx/utils/Animation.hpp>
#include <ostd/math/Geometry.hpp>
#include <ogfx/resources/Image.hpp>

namespace ogfx
{
	class WindowCore;
	class BasicRenderer2D
	{
		public: struct tErrors
		{
			inline static constexpr int32_t NoError = 0;
			inline static constexpr int32_t FailedToLoad = 1;
			inline static constexpr int32_t FailedToOpenFontFile = 2;
			inline static constexpr int32_t Uninitialized = 3;
			inline static constexpr int32_t InvalidState = 5;
			inline static constexpr int32_t TTFRenderTextBlendedFail = 6;
			inline static constexpr int32_t TTFCreateTextureFromSurfaceFail = 7;
			inline static constexpr int32_t NullFont = 8;
			inline static constexpr int32_t NoFont = 9;
			inline static constexpr int32_t FailedToOpenFontByteStrean = 10;
		};
		private: class SignalHandler : public ostd::BaseObject
		{
			public:
				SignalHandler(BasicRenderer2D& parent);
				void handleSignal(ostd::Signal& signal) override;

			private:
				BasicRenderer2D& m_parent;
		};
		public:
			BasicRenderer2D(void) = default;
			~BasicRenderer2D(void);
			int32_t init(WindowCore& window);
			void flushBatch(void);
			void endFrame(void);
			void pushClippingRect(const ostd::Rectangle& rect, bool additive = false);
			void popClippingRect(void);
			int32_t loadDefaultFont(int32_t fontSize = 0);
			void closeFont(void);
			int32_t openFont(const ostd::String& fontPath, int32_t fontSize = 0);
			int32_t openFont(const ostd::UByte resource_data[], uint32_t data_size, int32_t fontSize = 0);
			int32_t setFontSize(int32_t fontSize);
			ostd::IPoint getStringDimensions(const ostd::String& message, int32_t fontSize = 0);

			inline uint32_t getDrawCallCount(void) { return m_drawCallCount; }
			inline bool hasOpenFont(void) { return m_fontOpen; }
			inline TTF_Font* getSDLFont(void) { return m_font;  }
			inline bool isValid(void) { return m_initialized && m_fontOpen && (m_font != nullptr || m_fontFromMemory); }
			inline int32_t geterrorState(void) { return m_errorState; }
			inline int32_t getFontSize(void) { return m_fontSize; }
			inline WindowCore& getWindow(void) { return *m_window; }
			inline bool isInitialized(void) { return m_initialized; }

            void drawImage(const ogfx::Image& image, const ostd::Vec2& position, const ostd::Vec2& size = { 0, 0 }, const ostd::Rectangle& srcRect = { 0, 0, 0, 0 });
            void drawAnimation(const Animation& anim, const ostd::Vec2& position, const ostd::Vec2& size = { 0, 0 });

			void drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, int32_t fontSize = 0);
			void drawCenteredString(const ostd::String& str, const ostd::Vec2& center, const ostd::Color& color, int32_t fontSize = 0);

			void drawLine(const ostd::FLine& line, const ostd::Color& color, int32_t thickness = 1, bool rounded = true);

			void drawRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness = 1);
			void drawRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, float radius, int32_t thickness = 1);
			void drawCircle(const ostd::Vec2& center, float radius, const ostd::Color& color, int32_t thickness = 1);
			void drawCircle(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness = 1);
			void drawEllipse(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness = 1);

			void fillRect(const ostd::Rectangle& rect, const ostd::Color& color);
			void fillRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, float radius);
			void fillCircle(const ostd::Vec2& center, float radius, const ostd::Color& color);
			void fillCircle(const ostd::Rectangle& rect, const ostd::Color& color);
			void fillEllipse(const ostd::Rectangle& rect, const ostd::Color& color);

			void outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = 1);
			void outlinedRoundRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, float radius, int32_t outlineThickness = 1);
			void outlinedCircle(const ostd::Vec2& center, float radius, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = 1);
			void outlinedCircle(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = 1);
			void outlinedEllipse(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = 1);

		private:
			void init_arrays(void);
			void generate_half_circle(const ostd::Vec2& center, const ostd::Vec2& dir, float radius, int segments, const ostd::Color& color);
			void generate_quarter_circle(const ostd::Vec2& center, float radius, float thickness, float startAngle, const ostd::Color& color, int segments);
			void generate_circle_stroke(const ostd::Vec2& center, float radius, float thickness, const ostd::Color& color, int segments);
			void generate_ellipse_stroke(const ostd::Vec2& center, float radiusX, float radiusY, float thickness, float startAngle, float endAngle, const ostd::Color& color, int segments);
			void generate_filled_ellipse_stroke(const ostd::Vec2& center, float radiusX, float radiusY, float startAngle, const ostd::Color& color, int segments);
			void generate_filled_ellipse(const ostd::Vec2& center, float radiusX, float radiusY, const ostd::Color& color, int segments);
			void push_polygon(const ostd::Vec2* verts, const ostd::Vec2* texCoords, uint32_t vertCount, const uint32_t* inds, uint32_t indexCount, const ostd::Color& color, SDL_Texture* texture);
			void print_ttf_error(const ostd::String& funcName);
			inline int32_t set_error_state(int32_t err) { m_errorState = err; return m_errorState; }
			void renderText(const ostd::String& message, int32_t x, int32_t y, const ostd::Color& color, int32_t fontSize = 0);
			void renderCenteredText(const ostd::String& message, int32_t center_x, int32_t center_y, const ostd::Color& color, int32_t fontSize = 0);

		public:
			inline static constexpr int32_t MaxVertices { 8192 };
			inline static constexpr int32_t MaxIndices { (int32_t)(MaxVertices * 1.5f) };
		private:
			WindowCore* m_window { nullptr };
			ostd::ConsoleOutputHandler m_out;
			bool m_initialized { false };
			std::vector<ostd::Rectangle> m_clipStack;

			std::array<SDL_Vertex, MaxVertices> m_vertices;
			std::array<int, MaxIndices> m_indices;
			int32_t m_vertexCount { 0 };
			int32_t m_indexCount { 0 };
			SDL_Texture* m_texture { nullptr };

			uint32_t m_drawCallCount { 0 };
			bool m_fontOpen { false };

			TTF_Font* m_font { nullptr };
			int32_t m_errorState { tErrors::NoError };
			int32_t m_fontSize { DefaultFontSize };
			bool m_fontFromMemory { false };

			inline static constexpr int32_t DefaultFontSize { 16 };
	};
}
