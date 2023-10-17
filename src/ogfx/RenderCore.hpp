#ifndef __RENDER_CORE_HPP__
#define __RENDER_CORE_HPP__

#include <ostd/BaseObject.hpp>
#include <ostd/Types.hpp>
#include <ostd/Geometry.hpp>
#include <ogfx/DataStructures.hpp>
#include <ostd/Color.hpp>
#include <ogfx/Renderer2D.hpp>
#include <ogfx/RenderCommands.hpp>
#include <ogfx/RenderTarget.hpp>
// #include <omniax/runtime/RTData.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove from header

	class RenderTarget;
	// class GameObject;
	class OrthoCamera;
	class RenderCore : public BaseObject
	{
		private: struct tRComPair
		{
			std::vector<RenderCommand*> cmdList;
			const RenderTarget* rTarg { nullptr };

			inline tRComPair(void)
			{
				cmdList.reserve(100);
				rTarg = nullptr;
			}

			inline void exec(void)
			{
				if (rTarg == nullptr || cmdList.size() == 0) return;
				Renderer2D::setRenderTarget(*rTarg);
				uint8_t err = tRenderCommandErrors::NoError;
				for (auto& cmd : cmdList)
				{
					if ((err = cmd->execute()) != tRenderCommandErrors::NoError)
						OX_WARN("RenderCommand Error: RenderTarget=%d Command=%d; Result=%d", rTarg->getResourceID(), cmd->getCommandID(), err);
				}
				// cmdList.clear();
			}
		};

		public:
			static void display(void);
			// static void newFrame(void);
			static void initRenderQUeue(void);

			RenderCore(void);
			void init(void);
			void setRenderTarget(const RenderTarget& target, bool follow_camera = true);
			void setDefaultRenderTarget(void);
			void bindShader(ResourceID shader);
			void clear(Color clearColor);

			void updateUniform_f(String uniform_name, float value);
			void updateUniform_i(String uniform_name, int32_t value);
			void updateUniform_b(String uniform_name, bool value);
			void updateUniform_vec2f(String uniform_name, const Vec2& value);
			void updateUniform_vec3f(String uniform_name, const Vec3& value);
			void updateUniform_vec4f(String uniform_name, const Vec4& value);
			void updateUniform_col(String uniform_name, const Color& value);
			void updateUniform_mat4f(String uniform_name, const glm::mat4& value);
			void updateUniform_arri(String uniform_name, const std::vector<int32_t>& value);
			void updateUniform_arrf(String uniform_name, const std::vector<float>& value);

			// void drawGameObject(GameObject& obj);

			void drawImage(TextureID texture, Vec2 position, Vec2 size, bool centereOrigin = false);
			void drawImage(ResourceID texture, Vec2 position, Vec2 size, bool centereOrigin = false);
			void drawImage(TextureID texture, Vec2 position, Vec2 size, Color tint, bool centereOrigin = false);
			void drawImage(ResourceID texture, Vec2 position, Vec2 size, Color tint, bool centereOrigin = false);
			void drawImage(TextureID texture, Color tint, Transform2D& transform);
			void drawImage(ResourceID texture, Color tint, Transform2D& transform);
			void drawImage(TextureID texture, Transform2D& transform);
			void drawImage(ResourceID texture, Transform2D& transform);
			
			void drawRenderTarget(const RenderTarget& target);
			void drawRenderTarget(const RenderTarget& target, Vec2 position, Vec2 size);
			void drawRenderTarget(const RenderTarget& target, Vec2 position, Vec2 size, Color tint);
			void drawRenderTarget(const RenderTarget& target, Transform2D& transform);
			void drawRenderTarget(const RenderTarget& target, Color tint, Transform2D& transform);

			void drawQuad(Vec2 position, Vec2 size, Color color);
			void drawQuad(Vec2 position, Vec2 size, Color color, bool centeredOrigin);
			void drawQuad(Color color, Transform2D& transform);
			void drawQuad(const std::vector<Vec2>& vertices, const Color& tintColor, TextureID texture);
			
			void drawLine(const Vec2& start, const Vec2& end, float thikness, const Color& color);

			void drawText(const String& text, Vec2 position);
			void drawText(const String& text, Vec2 position, float fontSize);
			void drawText(const String& text, Vec2 position, Color color);
			void drawText(const String& text, Vec2 position, float fontSize, Color color);
			void drawText(const String& text, Vec2 position, ResourceID font, float fontSize);
			void drawText(const String& text, Vec2 position, ResourceID font, Color color);
			void drawText(const String& text, Vec2 position, ResourceID font, float fontSize, Color color);
			void drawText(const String& text, Vec2 position, const Renderer2D::tTextInfo& params);

			void setTextParameters(const Renderer2D::tTextInfo& params);
			inline void setFontSize(float size) { m_textHeight = size; }
			inline void setTextSpacing(float spacing) { m_textSpacing = spacing; }
			inline void setFont(ResourceID font) { m_textFont = font; }
			inline void setFont(ResourceID font, float size) { m_textFont = font; m_textHeight = size; }
			inline void setFont(ResourceID font, float size, float spacing) { m_textFont = font; m_textHeight = size; m_textSpacing = spacing; }
			inline void setFontColor(Color color) { m_textColor = color; }

			inline float getFontSize(void) { return m_textHeight; }
			inline float getTextSpacing(void) { return m_textSpacing; }
			inline ResourceID getFont(void) { return m_textFont; }
			inline Color getTextColor(void) { return m_textColor; }

			inline ResourceID getCurrentShaderID(void) { return m_currentShader; }
			inline uint32_t getCurrentRenderTarget(void) { return m_currentRenderTarget; }
			inline void setRenderTarget(uint32_t rt) { if (rt < RenderCore::s_renderQueue.size()) m_currentRenderTarget = rt; }

			inline void setCamera(OrthoCamera& camera) { m_camera = &camera; }
			inline static void resetRenderCmdCount(void) { m_renderCmdCount = 0; }
			inline static uint32_t getRenderCmdCount(void) { return m_renderCmdCount; }
			bool hasValidCamera(void);
			OrthoCamera& getCamera(void);

		private:
			void __store_text_parameters(void);
			void __restore_text_parameters(void);

		private:
			float m_textHeight { 32.0f };
			float m_textSpacing { 5.0f };
			ResourceID m_textFont { 0 };
			Color m_textColor { 255, 255, 255 };

			float m_textHeight_stored { 32.0f };
			float m_textSpacing_stored { 5.0f };
			ResourceID m_textFont_stored { 0 };
			Color m_textColor_stored { 255, 255, 255 };

			inline static std::vector<tRComPair> s_renderQueue;
			uint32_t m_currentRenderTarget { 0 };
			ResourceID m_currentShader { 0 };

			inline static uint32_t m_renderCmdCount { 0 };
			
			OrthoCamera* m_camera { nullptr };
	};
}

#endif