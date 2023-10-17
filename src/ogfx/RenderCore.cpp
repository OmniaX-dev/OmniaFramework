#include "RenderCore.hpp"
#include <ogfx/ResourceManager.hpp>
//#include <omniax/core/GameObject.hpp>
#include <ogfx/Camera.hpp>

namespace ogfx
{
	using namespace ostd;

	void RenderCore::display(void)
	{
		for (auto& targ : s_renderQueue)
		{
			targ.exec();
			m_renderCmdCount += targ.cmdList.size();
			targ.cmdList.clear();
		}
	}

	// void RenderCore::newFrame(void)
	// {
	// 	s_renderQueue.clear();
	// 	tRComPair default_rtarg;
	// 	default_rtarg.rTarg = &Renderer2D::getDefaultRenderTarget();
	// 	s_renderQueue.push_back(default_rtarg);
	// }

	void RenderCore::initRenderQUeue(void)
	{
		s_renderQueue.reserve(100);
		tRComPair default_rtarg;
		default_rtarg.rTarg = &Renderer2D::getDefaultRenderTarget();
		s_renderQueue.push_back(default_rtarg);
	}

	RenderCore::RenderCore(void)
	{
	}

	void RenderCore::init(void)
	{
		m_textFont = ResourceManager::getDefaultBitmapFont();
		__store_text_parameters();
	}

	void RenderCore::setRenderTarget(const RenderTarget& target, bool follow_camera)
	{
		bool valid_cam = hasValidCamera();
		for (uint32_t i = 0; i < RenderCore::s_renderQueue.size(); i++)
		{
			auto& rtarg = RenderCore::s_renderQueue[i];
			if (rtarg.rTarg->getResourceID() == target.getResourceID())
			{
				m_currentRenderTarget = i;
				if (follow_camera && valid_cam)
					updateUniform_mat4f("u_viewProjMatrix", m_camera->getViewProjectionMatrix());
				else if (valid_cam)
					updateUniform_mat4f("u_viewProjMatrix", m_camera->getProjectionMatrix());
				return;
			}
		}
		tRComPair rtarg;
		rtarg.rTarg = &target;
		RenderCore::s_renderQueue.push_back(rtarg);
		m_currentRenderTarget = RenderCore::s_renderQueue.size() - 1;
		if (follow_camera && valid_cam)
			updateUniform_mat4f("u_viewProjMatrix", m_camera->getViewProjectionMatrix());
		else if (valid_cam)
			updateUniform_mat4f("u_viewProjMatrix", m_camera->getProjectionMatrix());
	}

	void RenderCore::setDefaultRenderTarget(void)
	{
		m_currentRenderTarget = 0;
		if (hasValidCamera())
			updateUniform_mat4f("u_viewProjMatrix", m_camera->getProjectionMatrix());
	}

	void RenderCore::bindShader(ResourceID shader)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newBindSHaderCommand(shader));
		m_currentShader = shader;
	}

	void RenderCore::clear(Color clearColor)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newClearCommand(clearColor));
	}

	void RenderCore::updateUniform_f(String uniform_name, float value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_f_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_i(String uniform_name, int32_t value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_i_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_b(String uniform_name, bool value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_b_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_vec2f(String uniform_name, const Vec2& value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_vec2_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_vec3f(String uniform_name, const Vec3& value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_vec3_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_vec4f(String uniform_name, const Vec4& value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_vec4_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_col(String uniform_name, const Color& value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_col_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_mat4f(String uniform_name, const glm::mat4& value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_mat4_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_arri(String uniform_name, const std::vector<int32_t>& value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_arri_Command(uniform_name, m_currentShader, value));
	}
	
	void RenderCore::updateUniform_arrf(String uniform_name, const std::vector<float>& value)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newUpdateUniform_arrf_Command(uniform_name, m_currentShader, value));
	}
	

	// void RenderCore::drawGameObject(GameObject& obj)
	// {
	// 	obj.draw(*this);
	// }

	void RenderCore::drawImage(TextureID texture, Vec2 position, Vec2 size, bool centereOrigin)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(Renderer2D::getStaticQuad(position, size, centereOrigin), { 255, 255, 255 }, texture));
	}
	
	void RenderCore::drawImage(ResourceID texture, Vec2 position, Vec2 size, bool centereOrigin)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(Renderer2D::getStaticQuad(position, size, centereOrigin), { 255, 255, 255 }, { texture, Texture::FullTextureCoords }));
	}
	
	void RenderCore::drawImage(TextureID texture, Vec2 position, Vec2 size, Color tint, bool centereOrigin)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(Renderer2D::getStaticQuad(position, size, centereOrigin), tint, texture));
	}
	
	void RenderCore::drawImage(ResourceID texture, Vec2 position, Vec2 size, Color tint, bool centereOrigin)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(Renderer2D::getStaticQuad(position, size, centereOrigin), tint, { texture, Texture::FullTextureCoords }));
	}
	
	void RenderCore::drawImage(TextureID texture, Color tint, Transform2D& transform)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(transform.getVertices(), tint, texture));
	}
	
	void RenderCore::drawImage(ResourceID texture, Color tint, Transform2D& transform)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(transform.getVertices(), tint, { texture, Texture::FullTextureCoords }));
	}
	
	void RenderCore::drawImage(TextureID texture, Transform2D& transform)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(transform.getVertices(), { 255, 255, 255 }, texture));
	}
	
	void RenderCore::drawImage(ResourceID texture, Transform2D& transform)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(transform.getVertices(), { 255, 255, 255 }, { texture, Texture::FullTextureCoords }));
	}
	
	void RenderCore::drawRenderTarget(const RenderTarget& target)
	{
		drawImage(target.getResourceID(), { 0.0f, 0.0f }, target.getSize());
	}
	
	void RenderCore::drawRenderTarget(const RenderTarget& target, Vec2 position, Vec2 size)
	{
		drawImage(target.getResourceID(), position, size);
	}
	
	void RenderCore::drawRenderTarget(const RenderTarget& target, Vec2 position, Vec2 size, Color tint)
	{
		drawImage(target.getResourceID(), position, size, tint);
	}
	
	void RenderCore::drawRenderTarget(const RenderTarget& target, Transform2D& transform)
	{
		drawImage(target.getResourceID(), transform);
	}
	
	void RenderCore::drawRenderTarget(const RenderTarget& target, Color tint, Transform2D& transform)
	{
		drawImage(target.getResourceID(), tint, transform);
	}
	
	void RenderCore::drawQuad(Vec2 position, Vec2 size, Color color)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(Renderer2D::getStaticQuad(position, size), color, { 0, 0 }));
	}
	
	void RenderCore::drawQuad(Vec2 position, Vec2 size, Color color, bool centeredOrigin)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(Renderer2D::getStaticQuad(position, size, centeredOrigin), color, { 0, 0 }));
	}
	
	void RenderCore::drawQuad(Color color, Transform2D& transform)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(transform.getVertices(), color, { 0, 0 }));
	}

	void RenderCore::drawQuad(const std::vector<Vec2>& vertices, const Color& tintColor, TextureID texture)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawQuadCommand(vertices, tintColor, texture));
	}

	void RenderCore::drawLine(const Vec2& start, const Vec2& end, float thikness, const Color& color)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawLineCommand(start, end, thikness, color));
	}
	
	void RenderCore::drawText(const String& text, Vec2 position)
	{
		RenderCore::s_renderQueue[m_currentRenderTarget].cmdList.push_back(RenderCommandPool::newDrawTextCommand(text, m_textFont, position, m_textColor, m_textHeight, m_textSpacing));
	}
	
	void RenderCore::drawText(const String& text, Vec2 position, float fontSize)
	{
		__store_text_parameters();
		m_textHeight = fontSize;
		drawText(text, position);
		__restore_text_parameters();
	}
	
	void RenderCore::drawText(const String& text, Vec2 position, Color color)
	{
		__store_text_parameters();
		m_textColor = color;
		drawText(text, position);
		__restore_text_parameters();
	}
	
	void RenderCore::drawText(const String& text, Vec2 position, float fontSize, Color color)
	{
		__store_text_parameters();
		m_textHeight = fontSize;
		m_textColor = color;
		drawText(text, position);
		__restore_text_parameters();
	}
	
	void RenderCore::drawText(const String& text, Vec2 position, ResourceID font, float fontSize)
	{
		__store_text_parameters();
		m_textHeight = fontSize;
		m_textFont = font;
		drawText(text, position);
		__restore_text_parameters();
	}
	
	void RenderCore::drawText(const String& text, Vec2 position, ResourceID font, Color color)
	{
		__store_text_parameters();
		m_textFont = font;
		m_textColor = color;
		drawText(text, position);
		__restore_text_parameters();
	}
	
	void RenderCore::drawText(const String& text, Vec2 position, ResourceID font, float fontSize, Color color)
	{
		__store_text_parameters();
		m_textFont = font;
		m_textColor = color;
		m_textHeight = fontSize;
		drawText(text, position);
		__restore_text_parameters();
	}
	
	void RenderCore::drawText(const String& text, Vec2 position, const Renderer2D::tTextInfo& params)
	{
		__store_text_parameters();
		setTextParameters(params);
		drawText(text, position);
		__restore_text_parameters();
	}
	
	void RenderCore::setTextParameters(const Renderer2D::tTextInfo& params)
	{
		m_textFont = params.font;
		m_textColor = params.color;
		m_textHeight = params.characterHeight;
		m_textSpacing = params.characterSpacing;
	}

	void RenderCore::__store_text_parameters(void)
	{
		m_textHeight_stored = m_textHeight;
		m_textSpacing_stored = m_textSpacing;
		m_textFont_stored = m_textFont;
		m_textColor_stored = m_textColor;
	}

	void RenderCore::__restore_text_parameters(void)
	{
		m_textHeight = m_textHeight_stored;
		m_textSpacing = m_textSpacing_stored;
		m_textFont = m_textFont_stored;
		m_textColor = m_textColor_stored;
	}

	bool RenderCore::hasValidCamera(void)
	{
		return m_camera != nullptr && m_camera->isValid();
	}

	OrthoCamera& RenderCore::getCamera(void)
	{
		if (hasValidCamera()) return *m_camera;
		return ((OrthoCamera&)BaseObject::InvalidRef());
	}
}