#include "RenderCommands.hpp"
#include <ogfx/RenderCore.hpp>

namespace ogfx
{
	using namespace ostd;

	static constexpr uint32_t DEFAULT_POOL_BLOCK_SIZE = 512;
	static constexpr uint32_t SMALL_POOL_BLOCK_SIZE = 64;

	void RenderCommandPool::init(void)
	{
		s_clearCmdPool.reserve(DEFAULT_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
			s_clearCmdPool.push_back(new rcom::Clear({ 0, 0, 0, 0 }));
		s_drawQuadPool.reserve(DEFAULT_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
			s_drawQuadPool.push_back(new rcom::DrawQuad({}, { 0, 0, 0, 0 }, { 0, 0 }));
		s_drawLinePool.reserve(DEFAULT_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
			s_drawLinePool.push_back(new rcom::DrawLine({ 0, 0 }, { 0, 0 }, 0.0f, { 0, 0, 0, 0 }));
		s_drawTextPool.reserve(DEFAULT_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
			s_drawTextPool.push_back(new rcom::DrawText("", 0, Vec2(0.0f, 0.0f), Color(0, 0, 0, 0), 0.0f, 0.0f));
		s_bindShaderPool.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_bindShaderPool.push_back(new rcom::BindShader(0));

		s_updUfQueue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUfQueue.push_back(new rcom::UpdateUniform<float>("", 0, 0.0f));
		s_updUiQueue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUiQueue.push_back(new rcom::UpdateUniform<int32_t>("", 0, 0));
		s_updUbQueue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUbQueue.push_back(new rcom::UpdateUniform<bool>("", 0, false));
		s_updUvec2Queue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUvec2Queue.push_back(new rcom::UpdateUniform<Vec2>("", 0, { 0.0f, 0.0f }));
		s_updUvec3Queue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUvec3Queue.push_back(new rcom::UpdateUniform<Vec3>("", 0, { 0.0f, 0.0f, 0.0f }));
		s_updUvec4Queue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUvec4Queue.push_back(new rcom::UpdateUniform<Vec4>("", 0, { 0.0f, 0.0f, 0.0f, 0.0f }));
		s_updUmat4Queue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUmat4Queue.push_back(new rcom::UpdateUniform<glm::mat4>("", 0, glm::mat4(0.0f)));
		s_updUcolQueue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUcolQueue.push_back(new rcom::UpdateUniform<Color>("", 0, { 0, 0, 0, 0 }));
		s_updUarriQueue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUarriQueue.push_back(new rcom::UpdateUniform<std::vector<int32_t>>("", 0, { }));
		s_updUarrfQueue.reserve(SMALL_POOL_BLOCK_SIZE);
		for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
			s_updUarrfQueue.push_back(new rcom::UpdateUniform<std::vector<float>>("", 0, { }));

		s_initialized = true;
	}

	void RenderCommandPool::destroy(void)
	{
		return; //TODO: Fix memory leak (got a crash when caling this function, that's why it's currently disabled)
		if (!s_initialized) return;
		for (auto& clr : s_clearCmdPool)
			delete clr;
		for (auto& drwQuad : s_drawQuadPool)
			delete drwQuad;
		for (auto& drwLine : s_drawLinePool)
			delete drwLine;
		for (auto& drwText : s_drawTextPool)
			delete drwText;
		for (auto& bindUnif : s_bindShaderPool)
			delete bindUnif;

		for (auto& upd : s_updUfQueue)
			delete upd;
		for (auto& upd : s_updUiQueue)
			delete upd;
		for (auto& upd : s_updUbQueue)
			delete upd;
		for (auto& upd : s_updUvec2Queue)
			delete upd;
		for (auto& upd : s_updUvec3Queue)
			delete upd;
		for (auto& upd : s_updUvec4Queue)
			delete upd;
		for (auto& upd : s_updUmat4Queue)
			delete upd;
		for (auto& upd : s_updUcolQueue)
			delete upd;
		for (auto& upd : s_updUarriQueue)
			delete upd;
		for (auto& upd : s_updUarrfQueue)
			delete upd;
	}

	void RenderCommandPool::newFrame(void)
	{
		s_nextClrCmd = 0;
		s_nextDrwQuadCmd = 0;
		s_nextDrwLineCmd = 0;
		s_nextDrwTextCmd = 0;
		s_nextBindShaderCmd = 0;
		
		s_nextUpdaUfCmd = 0;
		s_nextUpdaUiCmd = 0;
		s_nextUpdaUbCmd = 0;
		s_nextUpdaUvec2Cmd = 0;
		s_nextUpdaUvec3Cmd = 0;
		s_nextUpdaUvec4Cmd = 0;
		s_nextUpdaUmat4Cmd = 0;
		s_nextUpdaUcolCmd = 0;
		s_nextUpdaUarriCmd = 0;
		s_nextUpdaUarrfCmd = 0;
	}

	RenderCommand* RenderCommandPool::newClearCommand(const Color& clearColor)
	{
		if (s_nextClrCmd == s_clearCmdPool.size())
		{
			s_clearCmdPool.reserve(s_clearCmdPool.size() + DEFAULT_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
				s_clearCmdPool.push_back(new rcom::Clear({ 0, 0, 0, 0 }));
			OX_DEBUG("Reallocated ClearCommand pool to size=%d.", s_clearCmdPool.size());
		}
		rcom::Clear* cmd = s_clearCmdPool[s_nextClrCmd++];
		cmd->create(clearColor);
		return (RenderCommand*)cmd;
	}

	RenderCommand* RenderCommandPool::newDrawQuadCommand(const std::vector<Vec2>& vertices, const Color& color, TextureID texture)
	{
		if (s_nextDrwQuadCmd == s_drawQuadPool.size())
		{
			s_drawQuadPool.reserve(s_drawQuadPool.size() + DEFAULT_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
				s_drawQuadPool.push_back(new rcom::DrawQuad({}, { 0, 0, 0, 0 }, { 0, 0 }));
			OX_DEBUG("Reallocated DrawQuadCommand pool to size=%d.", s_drawQuadPool.size());
		}
		rcom::DrawQuad* cmd = s_drawQuadPool[s_nextDrwQuadCmd++];
		cmd->create(vertices, color, texture);
		return (RenderCommand*)cmd;
	}

	RenderCommand* RenderCommandPool::newDrawLineCommand(const Vec2& start, const Vec2& end, float thikness, const Color& color)
	{
		if (s_nextDrwLineCmd == s_drawLinePool.size())
		{
			s_drawLinePool.reserve(s_drawLinePool.size() + DEFAULT_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
				s_drawLinePool.push_back(new rcom::DrawLine({ 0, 0 }, { 0, 0 }, 0.0f, { 0, 0, 0, 0 }));
			OX_DEBUG("Reallocated DrawLineCommand pool to size=%d.", s_drawLinePool.size());
		}
		rcom::DrawLine* cmd = s_drawLinePool[s_nextDrwLineCmd++];
		cmd->create(start, end, thikness, color);
		return (RenderCommand*)cmd;
	}

	RenderCommand* RenderCommandPool::newDrawTextCommand(const String& text, ResourceID font, const Vec2& position, const Color& color, float charHeight, float spacing)
	{
		if (s_nextDrwTextCmd == s_drawTextPool.size())
		{
			s_drawTextPool.reserve(s_drawTextPool.size() + DEFAULT_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
				s_drawTextPool.push_back(new rcom::DrawText("", 0, Vec2(0.0f, 0.0f), Color(0, 0, 0, 0), 0.0f, 0.0f));
			OX_DEBUG("Reallocated DrawTextCommand pool to size=%d.", s_drawTextPool.size());
		}
		rcom::DrawText* cmd = s_drawTextPool[s_nextDrwTextCmd++];
		cmd->create(text, font, position, color, charHeight, spacing);
		return (RenderCommand*)cmd;
	}

	RenderCommand* RenderCommandPool::newBindSHaderCommand(ResourceID shader)
	{
		if (s_nextBindShaderCmd == s_bindShaderPool.size())
		{
			s_bindShaderPool.reserve(s_bindShaderPool.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < DEFAULT_POOL_BLOCK_SIZE; i++)
				s_bindShaderPool.push_back(new rcom::BindShader(0));
			OX_DEBUG("Reallocated BindShaderCommand pool to size=%d.", s_bindShaderPool.size());
		}
		rcom::BindShader* cmd = s_bindShaderPool[s_nextBindShaderCmd++];
		cmd->create(shader);
		return (RenderCommand*)cmd;
	}

	RenderCommand* RenderCommandPool::newUpdateUniform_f_Command(const String& name, ResourceID shader, float value)
	{
		if (s_nextUpdaUfCmd == s_updUfQueue.size())
		{
			s_updUfQueue.reserve(s_updUfQueue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUfQueue.push_back(new rcom::UpdateUniform<float>("", 0, 0.0f));
			OX_DEBUG("Reallocated UpdateUniform_f_Command pool to size=%d.", s_updUfQueue.size());
		}
		auto cmd = s_updUfQueue[s_nextUpdaUfCmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_i_Command(const String& name, ResourceID shader, int32_t value)
	{
		if (s_nextUpdaUiCmd == s_updUiQueue.size())
		{
			s_updUiQueue.reserve(s_updUiQueue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUiQueue.push_back(new rcom::UpdateUniform<int32_t>("", 0, 0));
			OX_DEBUG("Reallocated UpdateUniform_i_Command pool to size=%d.", s_updUiQueue.size());
		}
		auto cmd = s_updUiQueue[s_nextUpdaUiCmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_b_Command(const String& name, ResourceID shader, bool value)
	{
		if (s_nextUpdaUbCmd == s_updUbQueue.size())
		{
			s_updUbQueue.reserve(s_updUbQueue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUbQueue.push_back(new rcom::UpdateUniform<bool>("", 0, false));
			OX_DEBUG("Reallocated UpdateUniform_b_Command pool to size=%d.", s_updUbQueue.size());
		}
		auto cmd = s_updUbQueue[s_nextUpdaUbCmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_vec2_Command(const String& name, ResourceID shader, Vec2 value)
	{
		if (s_nextUpdaUvec2Cmd == s_updUvec2Queue.size())
		{
			s_updUvec2Queue.reserve(s_updUvec2Queue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUvec2Queue.push_back(new rcom::UpdateUniform<Vec2>("", 0, { 0.0f, 0.0f }));
			OX_DEBUG("Reallocated UpdateUniform_vec2_Command pool to size=%d.", s_updUvec2Queue.size());
		}
		auto cmd = s_updUvec2Queue[s_nextUpdaUvec2Cmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_vec3_Command(const String& name, ResourceID shader, Vec3 value)
	{
		if (s_nextUpdaUvec3Cmd == s_updUvec3Queue.size())
		{
			s_updUvec3Queue.reserve(s_updUvec3Queue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUvec3Queue.push_back(new rcom::UpdateUniform<Vec3>("", 0, { 0.0f, 0.0f }));
			OX_DEBUG("Reallocated UpdateUniform_vec3_Command pool to size=%d.", s_updUvec3Queue.size());
		}
		auto cmd = s_updUvec3Queue[s_nextUpdaUvec3Cmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_vec4_Command(const String& name, ResourceID shader, Vec4 value)
	{
		if (s_nextUpdaUvec4Cmd == s_updUvec4Queue.size())
		{
			s_updUvec4Queue.reserve(s_updUvec4Queue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUvec4Queue.push_back(new rcom::UpdateUniform<Vec4>("", 0, { 0.0f, 0.0f }));
			OX_DEBUG("Reallocated UpdateUniform_vec4_Command pool to size=%d.", s_updUvec4Queue.size());
		}
		auto cmd = s_updUvec4Queue[s_nextUpdaUvec4Cmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_mat4_Command(const String& name, ResourceID shader, const glm::mat4& value)
	{
		if (s_nextUpdaUmat4Cmd == s_updUmat4Queue.size())
		{
			s_updUmat4Queue.reserve(s_updUmat4Queue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUmat4Queue.push_back(new rcom::UpdateUniform<glm::mat4>("", 0, glm::mat4(0.0f)));
			OX_DEBUG("Reallocated UpdateUniform_mat4_Command pool to size=%d.", s_updUmat4Queue.size());
		}
		auto cmd = s_updUmat4Queue[s_nextUpdaUmat4Cmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_col_Command(const String& name, ResourceID shader, const Color& value)
	{
		if (s_nextUpdaUcolCmd == s_updUcolQueue.size())
		{
			s_updUcolQueue.reserve(s_updUcolQueue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUcolQueue.push_back(new rcom::UpdateUniform<Color>("", 0, { 0, 0, 0, 0 }));
			OX_DEBUG("Reallocated UpdateUniform_col_Command pool to size=%d.", s_updUcolQueue.size());
		}
		auto cmd = s_updUcolQueue[s_nextUpdaUcolCmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_arri_Command(const String& name, ResourceID shader, const std::vector<int32_t>& value)
	{
		if (s_nextUpdaUarriCmd == s_updUarriQueue.size())
		{
			s_updUarriQueue.reserve(s_updUarriQueue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUarriQueue.push_back(new rcom::UpdateUniform<std::vector<int32_t>>("", 0, { }));
			OX_DEBUG("Reallocated UpdateUniform_arri_Command pool to size=%d.", s_updUarriQueue.size());
		}
		auto cmd = s_updUarriQueue[s_nextUpdaUarriCmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	
	RenderCommand* RenderCommandPool::newUpdateUniform_arrf_Command(const String& name, ResourceID shader, const std::vector<float>& value)
	{
		if (s_nextUpdaUarrfCmd == s_updUarrfQueue.size())
		{
			s_updUarrfQueue.reserve(s_updUarrfQueue.size() + SMALL_POOL_BLOCK_SIZE);
			for (uint32_t i = 0; i < SMALL_POOL_BLOCK_SIZE; i++)
				s_updUarrfQueue.push_back(new rcom::UpdateUniform<std::vector<float>>("", 0, { }));
			OX_DEBUG("Reallocated UpdateUniform_arrf_Command pool to size=%d.", s_updUarrfQueue.size());
		}
		auto cmd = s_updUarrfQueue[s_nextUpdaUarrfCmd++];
		cmd->create(name, shader, value);
		return (RenderCommand*)cmd;
	}
	

	namespace rcom
	{
		BindShader::BindShader(ResourceID shaderID)
		{
			setTypeName("ox::rcom::BindShader");
			m_commandID = tRenderCommands::BindShader;
			validate();
			create(shaderID);
		}
		BindShader& BindShader::create(ResourceID shaderID)
		{
			m_shaderID = shaderID;
			return *this;
		}
		uint8_t BindShader::execute(void)
		{
			if (isInvalid()) return tRenderCommandErrors::InvalidatedRenderCommand;
			if (Renderer2D::bindShader(m_shaderID).isInvalid())
				return tRenderCommandErrors::InvalidShader;
			return tRenderCommandErrors::NoError;
		}



		Clear::Clear(const Color& clearColor)
		{
			setTypeName("ox::rcom::Clear");
			validate();
			m_commandID = tRenderCommands::Clear;
			create(clearColor);
		}
		Clear& Clear::create(const Color& clearColor)
		{
			m_clearColor = clearColor;
			return *this;
		}
		uint8_t Clear::execute(void)
		{
			if (isInvalid()) return tRenderCommandErrors::InvalidatedRenderCommand;
			Renderer2D::clear(m_clearColor);
			return tRenderCommandErrors::NoError;
		}



		DrawQuad::DrawQuad(const std::vector<Vec2>& vertices, const Color& color, TextureID texture)
		{
			setTypeName("ox::rcom::DrawQuad");
			m_commandID = tRenderCommands::DrawQuad;
			validate();
			create(vertices, color, texture);
		}
		DrawQuad& DrawQuad::create(const std::vector<Vec2>& vertices, const Color& color, TextureID texture)
		{
			m_color = color;
			m_texture = texture;
			m_vertices = vertices;
			return *this;
		}
		uint8_t DrawQuad::execute(void)
		{
			if (isInvalid()) return tRenderCommandErrors::InvalidatedRenderCommand;
			Renderer2D::drawQuad(m_vertices, m_color, m_texture.texture, m_texture.tile);
			return tRenderCommandErrors::NoError;
		}



		DrawLine::DrawLine(const Vec2& start, const Vec2& end, float thikness, const Color& color)
		{
			setTypeName("ox::rcom::DrawLine");
			m_commandID = tRenderCommands::DrawLine;
			validate();
			create(start, end, thikness, color);
		}
		DrawLine& DrawLine::create(const Vec2& start, const Vec2& end, float thikness, const Color& color)
		{
			m_start = start;
			m_end = end;
			m_thickness = thikness;
			m_color = color;
			return *this;
		}
		uint8_t DrawLine::execute(void)
		{
			if (isInvalid()) return tRenderCommandErrors::InvalidatedRenderCommand;
			Renderer2D::drawLine(m_start, m_end, m_thickness, m_color);
			return tRenderCommandErrors::NoError;
		}


		DrawText::DrawText(const String& text, ResourceID font, const Vec2& position, const Color& color, float charHeight, float spacing)
		{
			setTypeName("ox::rcom::DrawText");
			m_commandID = tRenderCommands::DrawText;
			validate();
			create(text, font, position, color, charHeight, spacing);
		}
		DrawText& DrawText::create(const String& text, ResourceID font, const Vec2& position, const Color& color, float charHeight, float spacing)
		{
			m_text = text;
			m_textInfo.font = font;
			m_textInfo.characterHeight = charHeight;
			m_textInfo.color = color;
			m_textInfo.characterSpacing = spacing;
			m_position = position;
			return *this;
		}
		uint8_t DrawText::execute(void)
		{
			if (isInvalid()) return tRenderCommandErrors::InvalidatedRenderCommand;
			Renderer2D::Text::draw(m_text, m_position, m_textInfo);
			return tRenderCommandErrors::NoError;
		}
	}
} // namespace ox
