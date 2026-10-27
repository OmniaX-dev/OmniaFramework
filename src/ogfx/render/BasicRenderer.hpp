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

#include <ogfx/utils/Animation.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/data/Color.hpp>
#include <ogfx/resources/Image.hpp>
#include <ostd/io/IOHandlers.hpp>
#include <ogfx/render/FontGlyphAtlas.hpp>
#include <ogfx/render/RenderTarget.hpp>
#include <ogfx/render/Shader.hpp>
#include <ostd/io/StaticHashMap.hpp>
#include <memory>

namespace ogfx
{
	class WindowCore;
	class RenderContext;
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
		private: class SignalHandler : public ostd::Object
		{
			public:
				SignalHandler(BasicRenderer2D& parent);
				void handleSignal(ostd::Signal& signal) override;

			private:
				BasicRenderer2D& m_parent;
		};
		private: struct BatchState
		{
			SDL_Texture* texture { nullptr };
			SDL_BlendMode blend { SDL_BLENDMODE_BLEND };
			SDL_GPURenderState* shader { nullptr };

			inline bool operator==(const BatchState& other) const { return texture == other.texture && blend == other.blend && shader == other.shader; }
			inline bool operator!=(const BatchState& other) const { return !(*this == other); }
		};
		private: struct FlatMesh
		{
			stdvec<Vec2> positions;
			stdvec<SDL_FColor> colors;
			stdvec<u32> indices;

			inline void clear(void) { positions.clear(); colors.clear(); indices.clear(); }
			inline u32 addVertex(const Vec2& position, const SDL_FColor& color) { positions.push_back(position); colors.push_back(color); return cast<u32>(positions.size() - 1); }
			inline void addTriangle(u32 a, u32 b, u32 c) { indices.push_back(a); indices.push_back(b); indices.push_back(c); }
		};
		private: class FontSizeGuard
		{
			public:
				inline FontSizeGuard(BasicRenderer2D& parent) : m_parent(parent), m_oldFontSize(parent.m_fontSize) {  }
				inline ~FontSizeGuard(void) { m_parent.setFontSize(m_oldFontSize); }

			private:
				BasicRenderer2D& m_parent;
				i32 m_oldFontSize;
		};
		private: struct MeshProcessor;
		public:
			BasicRenderer2D(void);
			~BasicRenderer2D(void);
			i32 init(WindowCore& window);
			SDL_Renderer* getSDLRenderer(void) const;
			// ID of the window this renderer draws into (0 before init()).
			SDL_WindowID getWindowID(void) const;
			void flushBatch(void);
			void endFrame(void);
			void pushClippingRect(const Rectangle& rect, bool additive = false);
			void popClippingRect(void);
			i32 loadDefaultMonoFont(i32 fontSize = 0);
			i32 loadDefaultFont(i32 fontSize = 0);
			void closeFont(void);
			i32 openFont(const String& fontPath, i32 fontSize = 0);
			i32 openFont(const ostd::UByte resource_data[], u32 data_size, i32 fontSize = 0);
			i32 setFontSize(i32 fontSize);
			Vec2 getStringDimensions(const String& message, i32 fontSize = 0, TTF_Font* font = nullptr);
			stdvec<Vec2> getStringDimensionsPerCharacter(const String& message, i32 fontSize = 0, TTF_Font* font = nullptr);
			// The embedded Ubuntu Mono, opened once next to the current font (the current font is
			// left alone). nullptr if it can't be opened.
			TTF_Font* getDefaultMonoFont(void);
			// Makes the string functions (drawString(), getStringDimensions(), ...) use `font`
			// until the matching popFont(), e.g. to draw monospace text with the same renderer
			// (and in the same batch) as everything else. The font must stay open meanwhile.
			void pushFont(TTF_Font* font);
			void popFont(void);

			inline u32 getDrawCallCount(void) { return m_lastFrameDrawCallCount; }
			inline u32 getCacheHitCount(void) { return m_cacheHitCount; }
			inline u32 getCacheMissCount(void) { return m_cacheMissCount; }
			inline bool hasOpenFont(void) { return m_fontOpen; }
			inline TTF_Font* getSDLFont(void) { return m_font;  }
			inline bool isValid(void) const { return m_initialized && m_fontOpen && (m_font != nullptr || m_fontFromMemory); }
			inline i32 geterrorState(void) { return m_errorState; }
			inline i32 getFontSize(void) { return m_fontSize; }
			inline WindowCore& getWindow(void) { return *m_window; }
			inline bool isInitialized(void) { return m_initialized; }
			inline FontGlyphAtlas& getFontGlyphAtlas(void) { return m_fontGlyphAtlas; }
			// Logical-to-pixel scale of the window (system DPI scale * user scale).
			f32 getPixelScale(void) const;
			// Debug helper: does what SDL_EVENT_RENDER_DEVICE_RESET does (every texture is destroyed
			// and re-created: glyph atlases, render targets, images), to test the recovery path.
			void simulateDeviceReset(void);

			// ===== RENDER TARGETS =====
			// Binds a target (targets nest, as a stack). The target starts with an empty clip
			// stack; the previous one is restored by endRenderTarget(). Returns false (and
			// changes nothing) if the target can't be bound: not created with this renderer,
			// already bound, or its texture could not be created.
			bool beginRenderTarget(RenderTarget& target, bool clear = true, const Color& clearColor = Colors::Transparent);
			void endRenderTarget(void);
			inline bool isRenderingToTarget(void) const { return !m_targetStack.empty(); }
			inline RenderTarget* getCurrentRenderTarget(void) const { return m_targetStack.empty() ? nullptr : m_targetStack.back().target; }
			// Draws a target like an image (premultiplied-alpha compositing). size {0,0} = the
			// target's logical size; srcRect is in the target's logical units.
			void drawRenderTarget(const RenderTarget& target, const Vec2& position, const Vec2& size = { 0, 0 }, const Color& tint = Colors::White, const Rectangle& srcRect = { 0, 0, 0, 0 });

			// ===== SHADERS =====
			// True when this renderer can use shaders (it runs on SDL's GPU renderer).
			bool areShadersSupported(void) const;
			// Everything drawn until endShader() goes through the shader. Shaders nest (a stack);
			// see ogfx::ScopedShader. Without shader support this warns once and draws unshaded.
			void beginShader(Shader& shader);
			void endShader(void);
			inline Shader* getCurrentShader(void) const { return m_shaderStack.empty() ? nullptr : m_shaderStack.back(); }

			// ===== EFFECTS =====
			// Gaussian blur of `source` into `destination` (two separable passes through an
			// internal, cached, intermediate target; destination is resized to the source). radius
			// is in logical units: how far the blur reaches (up to MaxBlurRadius pixels). Returns
			// false when shaders aren't supported.
			bool blurRenderTarget(const RenderTarget& source, RenderTarget& destination, f32 radius);
			// Draws a target blurred, like drawRenderTarget(); the blurred copy lives in internal
			// targets cached by size. Without shader support it draws the target unblurred.
			void drawRenderTargetBlurred(const RenderTarget& target, const Vec2& position, f32 radius, const Vec2& size = { 0, 0 }, const Color& tint = Colors::White, const Rectangle& srcRect = { 0, 0, 0, 0 });

			void drawImage(const ogfx::Image& image, const Vec2& position, const Vec2& size = { 0, 0 }, const Color& tint = Colors::White, const Rectangle& srcRect = { 0, 0, 0, 0 });
			void drawAnimation(const Animation& anim, const Vec2& position, const Vec2& size = { 0, 0 }, const Color& tint = Colors::White);
			void fillGradientRect(const Rectangle& rect, ColorGradient& gradient);
			void drawString(const String& str, const Vec2& position, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);
			void drawCenteredString(const String& str, const Vec2& center, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);
			void drawCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);
			void drawVCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);
			void drawHCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize = 0, f32 scale = 1.0f);

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

		private: struct BlurTargets
		{
			Vec2 size { 0, 0 };
			std::unique_ptr<RenderTarget> pass;
			std::unique_ptr<RenderTarget> result;
			u64 lastUsedFrame { 0 };
		};
		private: struct KernKey
		{
			u64 font { 0 };
			u32 size { 0 };
			u32 left { 0 };
			u32 right { 0 };
			inline bool operator==(const KernKey& other) const noexcept { return font == other.font && size == other.size && left == other.left && right == other.right; }
		};
		private: struct KernKeyHasher
		{
			inline size_t operator()(const KernKey& k) const noexcept
			{
				size_t h = std::hash<u64>()(k.font);
				h ^= std::hash<u64>()((cast<u64>(k.left) << 32) | k.right) + 0x9e3779b9 + (h << 6) + (h >> 2);
				h ^= std::hash<u32>()(k.size) + 0x9e3779b9 + (h << 6) + (h >> 2);
				return h;
			}
		};
		private: struct TargetBinding
		{
			RenderTarget* target { nullptr };
			stdvec<Rectangle> savedClipStack;
		};
		private:
			// Called by RenderContext for SDL_EVENT_RENDER_* events of this renderer's window.
			void __on_render_event(u32 eventType);
			// Called by RenderContext right before the SDL renderer is destroyed.
			void __release_device_resources(void);
			void discard_batch(void);
			void close_mono_font(void);
			void apply_clip_rect(void);
			void bind_sdl_target(RenderTarget* target);
			void clear_texture(SDL_Texture* texture);
			void unwind_render_targets(void);
			void register_target(RenderTarget& target);
			void unregister_target(RenderTarget& target);
			void register_image(Image& image);
			void unregister_image(Image& image);
			void register_shader(Shader& shader);
			void unregister_shader(Shader& shader, SDL_GPURenderState* state);
			void flush_if_using(SDL_GPURenderState* state);
			void update_active_shader(void);
			bool ensure_blur_shaders(void);
			BlurTargets& get_blur_targets(const Vec2& size);

			// Single choke point: every vertex the renderer draws goes through here. A change of
			// batch state (texture, blend, ...) flushes the pending batch first.
			void emit(const BatchState& state, const SDL_Vertex* verts, u32 vertCount, const i32* inds, u32 indexCount);
			void emit_textured_quad(SDL_Texture* texture, const Vec2 verts[4], const Vec2 uvs[4], const SDL_FColor& color, SDL_BlendMode blend = SDL_BLENDMODE_BLEND);
			// Welds, anti-aliases (silhouette fringe) and emits a flat-colored mesh sampling the atlas white texel.
			void submit_flat_mesh(const FlatMesh& mesh, bool antialias = true);
			BatchState flat_batch_state(void) const;

			bool mesh_line(FlatMesh& mesh, const Vec2& p1, const Vec2& p2, f32 thickness, bool rounded, const SDL_FColor& color);
			void mesh_filled_arc(FlatMesh& mesh, const Vec2& center, f32 radiusX, f32 radiusY, f32 startAngle, f32 endAngle, i32 segments, const SDL_FColor& color);
			void mesh_arc_stroke(FlatMesh& mesh, const Vec2& center, f32 radiusX, f32 radiusY, f32 thickness, f32 startAngle, f32 endAngle, i32 segments, const SDL_FColor& color);
			void print_ttf_error(const String& funcName);
			// Cached TTF_GetGlyphKerning(); the font size is only switched on a cache miss.
			i32 get_kerning(TTF_Font* font, i32 fontSize, u32 left, u32 right);
			inline i32 set_error_state(i32 err) { m_errorState = err; return m_errorState; }

		public:
			inline static constexpr i32 MaxVertices { 8192 };
			inline static constexpr i32 MaxIndices { (i32)(MaxVertices * 1.5f) };
			// Largest blur reach, in pixels (the built-in blur shader's tap limit).
			inline static constexpr i32 MaxBlurRadius { 96 };
			inline static constexpr i32 MaxBlurCacheEntries { 4 };
		private:
			WindowCore* m_window { nullptr };
			ostd::ConsoleOutputHandler m_out;
			bool m_initialized { false };
			stdvec<Rectangle> m_clipStack;

			std::array<SDL_Vertex, MaxVertices> m_vertices;
			std::array<i32, MaxIndices> m_indices;
			i32 m_vertexCount { 0 };
			i32 m_indexCount { 0 };
			BatchState m_batchState;
			FlatMesh m_mesh;
			stdvec<SDL_Vertex> m_meshVerts;
			stdvec<i32> m_meshInds;
			stdvec<Vec2> m_contourA;
			stdvec<Vec2> m_contourB;
			std::unique_ptr<MeshProcessor> m_meshProcessor;

			u32 m_drawCallCount { 0 };
			u32 m_lastFrameDrawCallCount { 0 };
			bool m_fontOpen { false };

			TTF_Font* m_font { nullptr };
			TTF_Font* m_monoFont { nullptr };
			stdvec<TTF_Font*> m_fontStack;
			i32 m_errorState { tErrors::NoError };
			i32 m_fontSize { DefaultFontSize };
			bool m_fontFromMemory { false };
			FontGlyphAtlas m_fontGlyphAtlas;
			SDL_Texture* m_lastUsedGlyphAtlasTex { nullptr };
			SignalHandler m_sigHandler { *this };

			ostd::StaticHashMap<String, stdvec<Vec2>> m_strDimsCache;
			// Kerning per (font, size, pair): asking SDL_ttf every draw makes FreeType reload
			// and re-hint both glyphs each time, which dominated text-heavy frames.
			std::unordered_map<KernKey, i32, KernKeyHasher> m_kernCache;
			i32 m_cacheHitCount { 0 };
			i32 m_cacheMissCount { 0 };

			bool m_deviceLost { false };
			stdvec<TargetBinding> m_targetStack;
			stdvec<RenderTarget*> m_targets;
			stdvec<Image*> m_images;
			stdvec<Shader*> m_shaderStack;
			stdvec<Shader*> m_shaders;
			SDL_GPURenderState* m_activeShaderState { nullptr };
			std::unique_ptr<Shader> m_blurShaderH;
			std::unique_ptr<Shader> m_blurShaderV;
			stdvec<std::unique_ptr<BlurTargets>> m_blurCache;
			u64 m_frameIndex { 0 };
			bool m_warnedNoBlur { false };

			inline static constexpr i32 DefaultFontSize { 16 };
			inline static constexpr i32 MaxStringDimsCacheSize { 512 };
			inline static constexpr size_t MaxKernCacheSize { 1 << 16 };

			friend class RenderContext;
			friend class RenderTarget;
			friend class Image;
			friend class Shader;
	};
}
