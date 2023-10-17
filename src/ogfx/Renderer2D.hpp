#ifndef __RENDERER_2D_HPP__
#define __RENDERER_2D_HPP__

#include <ostd/Geometry.hpp>
#include <ostd/Color.hpp>
#include <ogfx/Texture.hpp>
#include <ogfx/DataStructures.hpp>
#include <ogfx/ResourceManager.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove from header

	class VertexArray;
	class Shader;
	class RenderTarget;
	class Renderer2D
	{
		public: struct tRenderStats
		{
			uint32_t drawCalls { 0 };
			uint32_t quadCount { 0 };
			uint32_t renderBufferBinds { 0 };
			uint32_t shaderBinds { 0 };
		};

		public: struct tTextInfo
		{
			float characterHeight { 32.0f };
			float characterSpacing { 5.0f };
			ResourceID font { ResourceManager::InvalidResource };
			Color color { 255, 255, 255 };
		};

		public: class Text
		{
			public:
				static void init(void);
				static void draw(const String& text, const Vec2& position, const Color& color);
				static void draw(const String& text, const Vec2& position, const tTextInfo& info);
				static void drawCentered(const String& text, const Vec2& position, const Color& color);
				static void drawCentered(const String& text, const Vec2& position, const tTextInfo& info);
				static Vec2 getStringBounds(const String& text, Vec2 padding = { 0.0f, 0.0f }, float characterHeight = 0.0f, float spacing = 0.0f);
				inline static void setFont(ResourceID _font) { font = _font; }

			public:
				inline static ResourceID font { ResourceManager::InvalidResource };
				inline static float characterHeight { 32.0f };
				inline static float characterSpacing { 5.0f };

			public:
				inline static constexpr int32_t ERR_INVALID_BITMAPFONT = OX_RENDERER2D_TEXT_ERR_MASK + 0x0001;
		};

		public:
			static void init(void);
			static void shutdown(void);

			static void beginBatch(void);
			static void endBatch(void);
			static void flush(void);

			static const tRenderStats& getRenderStats(void);
			static void resetStats(void);

			static void drawQuad(const Transform2D& transform, const Color& color);
			static void drawQuad(const Transform2D& transform, ResourceID texture, TextureAtlasIndex tile_index = Texture::FullTextureCoords, const Color& tint = { 255, 255, 255 });
			static void drawQuad(const std::vector<Vec2>& vertices, const Color& tintColor, ResourceID texture = ResourceManager::InvalidResource, TextureAtlasIndex tile_index = Texture::FullTextureCoords);
			static void drawLine(const Vec2& start, const Vec2& end, float thickness, const Color& color);
			static void drawTriangle(const Triangle& triangle, const Color& color);
			static void drawSingle(const VertexArray& vao, const Shader& shader);
			static void drawText(const String& text, ResourceID font, Vec2 position, Color color, float charHeight, float spacing = 5.0f);

			static Shader& bindShader(ResourceID shader);

			static void clear(const Color& color, uint32_t gl_mask = GL_COLOR_BUFFER_BIT);
			static void setRenderTarget(const RenderTarget& target);
			static void setDefaultRenderTarget(void);
			static void enableDepthTest(bool enable = true);

			static const RenderTarget& getCurrentRenderTarget(void);
			static const Shader& getCurrentShader(void);
			static ResourceID getCurrentShaderID(void);
			inline static const RenderTarget& getDefaultRenderTarget(void) { return (const RenderTarget&)BaseObject::InvalidRef(); }

			static std::vector<Vec2> getStaticQuad(Vec2 position, Vec2 size, bool centered = true);

		public:
			inline static constexpr int32_t ERR_INVALID_TEXTURE = OX_RENDERER2D_ERR_MASK + 0x0001;
			inline static constexpr int32_t ERR_INVALID_SHADER = OX_RENDERER2D_ERR_MASK + 0x0002;
	};

	typedef RenderTarget RenderTexture;
}

#endif