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

#include <ogfx/utils/Animation.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/data/Color.hpp>
#include <ogfx/resources/Image.hpp>
#include <ostd/io/IOHandlers.hpp>
#include <ogfx/render/FontGlyphAtlas.hpp>

namespace ogfx
{
	class WindowCore;
	class BasicRenderer2D
	{
		public: struct tErrors
		{
			inline static constexpr i32 NoError = 0;
			inline static constexpr i32 FailedToLoad = 1;
			inline static constexpr i32 FailedToOpenFontFile = 2;
			inline static constexpr i32 Uninitialized = 3;
			inline static constexpr i32 InvalidState = 5;
			inline static constexpr i32 TTFRenderTextBlendedFail = 6;
			inline static constexpr i32 TTFCreateTextureFromSurfaceFail = 7;
			inline static constexpr i32 NullFont = 8;
			inline static constexpr i32 NoFont = 9;
			inline static constexpr i32 FailedToOpenFontByteStrean = 10;
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
			inline BasicRenderer2D(void) {  }
			~BasicRenderer2D(void);
			i32 init(WindowCore& window);
			SDL_Renderer* getSDLRenderer(void) const;
			void flushBatch(void);
			void endFrame(void);
			void pushClippingRect(const Rectangle& rect, bool additive = false);
			void popClippingRect(void);
			i32 loadDefaultFont(i32 fontSize = 0);
			void closeFont(void);
			i32 openFont(const String& fontPath, i32 fontSize = 0);
			i32 openFont(const ostd::UByte resource_data[], u32 data_size, i32 fontSize = 0);
			i32 setFontSize(i32 fontSize);
			Vec2 getStringDimensions(const String& message, i32 fontSize = 0, TTF_Font* font = nullptr);

			inline u32 getDrawCallCount(void) { return m_lastFrameDrawCallCount; }
			inline bool hasOpenFont(void) { return m_fontOpen; }
			inline TTF_Font* getSDLFont(void) { return m_font;  }
			inline bool isValid(void) const { return m_initialized && m_fontOpen && (m_font != nullptr || m_fontFromMemory); }
			inline i32 geterrorState(void) { return m_errorState; }
			inline i32 getFontSize(void) { return m_fontSize; }
			inline WindowCore& getWindow(void) { return *m_window; }
			inline bool isInitialized(void) { return m_initialized; }
			inline FontGlyphAtlas& getFontGlyphAtlas(void) { return m_fontGlyphAtlas; }

			void drawImage(const ogfx::Image& image, const Vec2& position, const Vec2& size = { 0, 0 }, const Rectangle& srcRect = { 0, 0, 0, 0 });
			void drawAnimation(const Animation& anim, const Vec2& position, const Vec2& size = { 0, 0 });
			void fillGradientRect(const Rectangle& rect, ColorGradient& gradient);
			void drawString(const String& str, const Vec2& position, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);
			void drawCenteredString(const String& str, const Vec2& center, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);
			void drawCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);

			void drawLine(const FLine& line, const Color& color, i32 thickness = 1, bool rounded = true);
			void drawRect(const Rectangle& rect, const Color& color, i32 thickness = 1, bool topEdge = true, bool rightEdge = true, bool bottomEdge = true, bool leftEdge = true);
			void drawRect(const Vec2& center, const Vec2& size, const Color& color, i32 thickness = 1, bool topEdge = true, bool rightEdge = true, bool bottomEdge = true, bool leftEdge = true);
			void drawRoundRect(const Vec2& center, const Vec2& size, const Color& color, f32 radius, i32 thickness = 1);
			void drawRoundRect(const Rectangle& rect, const Color& color, f32 radius, i32 thickness = 1);
			void drawRoundRect(const Rectangle& rect, const Color& color, const Rectangle& radii, i32 thickness = 1);
			void drawRoundRect(const Vec2& center, const Vec2& size, const Color& color, const Rectangle& radii, i32 thickness = 1);
			void drawCircle(const Vec2& center, f32 radius, const Color& color, i32 thickness = 1);
			void drawCircle(const Rectangle& rect, const Color& color, i32 thickness = 1);
			void drawEllipse(const Rectangle& rect, const Color& color, i32 thickness = 1);
			void drawTriangle(const Triangle& tri, const Color& color, i32 thickness = 1);
			void drawTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& color, i32 thickness = 1);

			void fillRect(const Rectangle& rect, const Color& color);
			void fillRect(const Vec2& center, const Vec2& size, const Color& color);
			void fillRoundRect(const Rectangle& rect, const Color& color, f32 radius);
			void fillRoundRect(const Vec2& center, const Vec2& size, const Color& color, f32 radius);
			void fillRoundRect(const Rectangle& rect, const Color& color, const Rectangle& radii);
			void fillRoundRect(const Vec2& center, const Vec2& size, const Color& color, const Rectangle& radii);
			void fillCircle(const Vec2& center, f32 radius, const Color& color);
			void fillCircle(const Rectangle& rect, const Color& color);
			void fillEllipse(const Rectangle& rect, const Color& color);
			void fillTriangle(const Triangle& tri, const Color& color);
			void fillTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& color);

			void outlinedRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1, bool topEdge = true, bool rightEdge = true, bool bottomEdge = true, bool leftEdge = true);
			void outlinedRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1, bool topEdge = true, bool rightEdge = true, bool bottomEdge = true, bool leftEdge = true);
			void outlinedRoundRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, f32 radius, i32 outlineThickness = 1);
			void outlinedRoundRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, f32 radius, i32 outlineThickness = 1);
			void outlinedRoundRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, const Rectangle& radii, i32 outlineThickness = 1);
			void outlinedRoundRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, const Rectangle& radii, i32 outlineThickness = 1);
			void outlinedCircle(const Vec2& center, f32 radius, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);
			void outlinedCircle(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);
			void outlinedEllipse(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);
			void outlinedTriangle(const Triangle& tri, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);
			void outlinedTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);

		private:
			void init_arrays(void);
			void generate_half_circle(const Vec2& center, const Vec2& dir, f32 radius, i32 segments, const Color& color, bool use_null_tex = false);
			void generate_ellipse_stroke(const Vec2& center, f32 radiusX, f32 radiusY, f32 thickness, f32 startAngle, f32 endAngle, const Color& color, i32 segments, bool use_null_tex = false);
			void generate_filled_ellipse_stroke(const Vec2& center, f32 radiusX, f32 radiusY, f32 startAngle, const Color& color, i32 segments, bool use_null_tex = false);
			void generate_filled_ellipse(const Vec2& center, f32 radiusX, f32 radiusY, const Color& color, i32 segments, bool use_null_tex = false);
			void push_polygon(const Vec2* verts, const Vec2* texCoords, u32 vertCount, const u32* inds, u32 indexCount, const Color& color, SDL_Texture* texture, bool use_null_tex = false);
			void print_ttf_error(const String& funcName);
			inline i32 set_error_state(i32 err) { m_errorState = err; return m_errorState; }

		public:
			inline static constexpr i32 MaxVertices { 8192 };
			inline static constexpr i32 MaxIndices { (i32)(MaxVertices * 1.5f) };
		private:
			WindowCore* m_window { nullptr };
			ostd::ConsoleOutputHandler m_out;
			bool m_initialized { false };
			stdvec<Rectangle> m_clipStack;

			std::array<SDL_Vertex, MaxVertices> m_vertices;
			std::array<i32, MaxIndices> m_indices;
			i32 m_vertexCount { 0 };
			i32 m_indexCount { 0 };
			SDL_Texture* m_texture { nullptr };

			u32 m_drawCallCount { 0 };
			u32 m_lastFrameDrawCallCount { 0 };
			bool m_fontOpen { false };

			TTF_Font* m_font { nullptr };
			i32 m_errorState { tErrors::NoError };
			i32 m_fontSize { DefaultFontSize };
			bool m_fontFromMemory { false };
			FontGlyphAtlas m_fontGlyphAtlas;
			SDL_Texture* m_lastUsedGlyphAtlasTex { nullptr };
			SignalHandler m_sigHandler { *this };

			inline static constexpr i32 DefaultFontSize { 16 };
	};
}
