#include "Renderer2D.hpp"
#include "GLBuffers.hpp"
#include "Shader.hpp"
#include "Errors.hpp"
#include "DataStructures.hpp"
#include "RenderTarget.hpp"
#include <ostd/Logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ostd/Signals.hpp>
//#include <omniax/runtime/RTData.hpp>
#include <cstring>

namespace ogfx
{
	using namespace ostd;

	void Renderer2D::Text::init(void)
	{
		Text::font = ResourceManager::getDefaultBitmapFont();
	}

	void Renderer2D::Text::draw(const String& text, const Vec2& position, const Color& color)
	{
		if (ResourceManager::getBitmapFont(Renderer2D::Text::font).isInvalid())
		{
			ErrorHandler::pushError(Renderer2D::Text::ERR_INVALID_BITMAPFONT);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("Renderer2D::Text::draw(const String&, const Vec2&, const Color&): %s", err_str.c_str());
			return;
		}
		Renderer2D::drawText(text, Renderer2D::Text::font, position, color, Renderer2D::Text::characterHeight, Renderer2D::Text::characterSpacing);
	}

	void Renderer2D::Text::draw(const String& text, const Vec2& position, const tTextInfo& info)
	{
		tTextInfo oldInfo { Renderer2D::Text::characterHeight,
							Renderer2D::Text::characterSpacing,
							Renderer2D::Text::font,
							{ 0, 0, 0, 0 } };
		Renderer2D::Text::characterHeight = info.characterHeight;
		Renderer2D::Text::characterSpacing = info.characterSpacing;
		Renderer2D::Text::font = info.font;
		draw(text, position, info.color);
		Renderer2D::Text::characterHeight = oldInfo.characterHeight;
		Renderer2D::Text::characterSpacing = oldInfo.characterSpacing;
		Renderer2D::Text::font = oldInfo.font;
	}

	void Renderer2D::Text::drawCentered(const String& text, const Vec2& position, const Color& color)
	{
		auto size = Renderer2D::Text::getStringBounds(text);
		Vec2 pos = position - (size / 2.0f);
		Renderer2D::Text::draw(text, pos, color);
	}

	void Renderer2D::Text::drawCentered(const String& text, const Vec2& position, const tTextInfo& info)
	{
		tTextInfo oldInfo { Renderer2D::Text::characterHeight,
							Renderer2D::Text::characterSpacing,
							Renderer2D::Text::font,
							{ 0, 0, 0, 0 } };
		Renderer2D::Text::characterHeight = info.characterHeight;
		Renderer2D::Text::characterSpacing = info.characterSpacing;
		Renderer2D::Text::font = info.font;
		drawCentered(text, position, info.color);
		Renderer2D::Text::characterHeight = oldInfo.characterHeight;
		Renderer2D::Text::characterSpacing = oldInfo.characterSpacing;
		Renderer2D::Text::font = oldInfo.font;
	}

	Vec2 Renderer2D::Text::getStringBounds(const String& text, Vec2 padding, float characterHeight, float spacing)
	{
		auto& _font = ResourceManager::getBitmapFont(Renderer2D::Text::font);
		if (_font.isInvalid())
		{
			ErrorHandler::pushError(Renderer2D::Text::ERR_INVALID_BITMAPFONT);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("ogfx::Renderer2D::Text::getStringBounds(const String&, const Vec2&, float, float): %s", err_str.c_str());
			return { 0.0f, 0.0f };
		}
		if (characterHeight == 0.0f) characterHeight = Renderer2D::Text::characterHeight;
		if (spacing == 0.0f) spacing = characterSpacing;
		const float baseCharHeight = _font.getBaseCharHeight();
		float scale = characterHeight / baseCharHeight;
		Vec2 bounds;
		Rectangle cbounds;
		if (spacing < 0) spacing = 0;
		for (auto& c : text)
		{
			_font.getChar(c, cbounds);
			bounds.x += (cbounds.w * scale) + (spacing * scale);
			float ny = cbounds.h * scale;
			if (ny > bounds.y) bounds.y = ny;
		}
		bounds += (padding * 2);
		return bounds;
	}

	// Vec2 Renderer2D::Text::getStringBounds(const String& text, Vec2 padding, float characterHeight, float spacing)
	// {
	// 	auto& _font = ResourceManager::getBitmapFont(Renderer2D::Text::font);
	// 	if (_font.isInvalid())
	// 	{
	// 		ErrorHandler::pushError(Renderer2D::Text::ERR_INVALID_BITMAPFONT);
	// 		String err_str = ErrorHandler::getLastErrorString();
	// 		OX_ERROR("ogfx::Renderer2D::Text::getStringBounds(const String&, const Vec2&, float, float): %s", err_str.c_str());
	// 		return { 0.0f, 0.0f };
	// 	}
	// 	if (characterHeight == 0.0f) characterHeight = Renderer2D::Text::characterHeight;
	// 	if (spacing == 0.0f) spacing = characterSpacing;
	// 	const float baseCharHeight = _font.getBaseCharHeight();
	// 	float scale = characterHeight / baseCharHeight;
	// 	Vec2 bounds;
	// 	Rectangle cbounds;
	// 	for (auto& c : text)
	// 	{
	// 		_font.getChar(c, cbounds);
	// 		bounds.x += cbounds.w + spacing;
	// 	}
	// 	bounds.x *= scale;
	// 	bounds.y *= scale;
	// 	bounds += (padding * 2);
	// 	std::cout << bounds << "\n";
	// 	return bounds;
	// }

	static constexpr size_t MaxQuadCount = 16384;
	static constexpr size_t MaxVertexCount = MaxQuadCount * 4;
	static constexpr size_t MaxIndexCount = MaxQuadCount * 6;
	static constexpr size_t MaxTextures = 16;

	struct tRendererData
	{
		VertexArray vao { false };
		VertexBuffer vbo;
		ElementBuffer ebo;

		uint32_t whiteTexture { 0 };
		int32_t whiteTextureSlot { 0 };

		uint32_t indexCount { 0 };

		tVertex2D* buffer { nullptr };
		tVertex2D* bufferPtr { nullptr };

		uint32_t texSlots[MaxTextures];
		uint32_t texSlotIndex { 1 };

		Texture* currentTex { nullptr };
		ResourceID currentTexResID { ResourceManager::InvalidResource };
		float currentTexIndex { 0.0f };
		Shader* currentShader { nullptr };
		ResourceID currentShaderResID { ResourceManager::InvalidResource };

		Renderer2D::tRenderStats renderStats;
		const RenderTarget* currentRenderTarget { nullptr };
	};

	static tRendererData s_rendererData;

	void Renderer2D::init(void)
	{
		s_rendererData.buffer = new tVertex2D[MaxVertexCount];
		s_rendererData.vao.create();
		s_rendererData.vao.bind();

		s_rendererData.vbo.create(nullptr, MaxVertexCount * sizeof(tVertex2D), true);
		s_rendererData.vao.addBuffer(s_rendererData.vbo, tVertex2D::getVertexBufferLayout());

		int32_t indices[MaxIndexCount];
		uint32_t offset = 0;
		for (size_t i = 0; i < MaxIndexCount; i += 6)
		{
			indices[i + 0] = 0 + offset;
			indices[i + 1] = 1 + offset;
			indices[i + 2] = 2 + offset;
			
			indices[i + 3] = 2 + offset;
			indices[i + 4] = 3 + offset;
			indices[i + 5] = 0 + offset;

			offset += 4;
		}

		s_rendererData.ebo.create(indices, MaxIndexCount);
		s_rendererData.vao.setElementBuffer(s_rendererData.ebo);

		GLCall(glGenTextures(1, &s_rendererData.whiteTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, s_rendererData.whiteTexture));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		uint32_t color = 0xffffffff;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

		s_rendererData.texSlots[0] = s_rendererData.whiteTexture;
		for (size_t i = 1; i < MaxTextures; i++)
			s_rendererData.texSlots[i] = 0;

		s_rendererData.currentTex = nullptr;
		s_rendererData.currentTexResID = ResourceManager::InvalidResource;
		s_rendererData.currentTexIndex = 0.0f;
		s_rendererData.currentShaderResID = ResourceManager::InvalidResource;
		s_rendererData.currentShader = nullptr;
		s_rendererData.currentRenderTarget = nullptr;
	}

	void Renderer2D::shutdown(void)
	{
		GLCall(glDeleteTextures(1, &s_rendererData.whiteTexture));
		//delete[] s_rendererData.buffer; //TODO: Fix memory leak (had a crash here, that's why it's currently disabled)
	}
	
	void Renderer2D::beginBatch(void)
	{
		s_rendererData.bufferPtr = s_rendererData.buffer;
	}
	
	void Renderer2D::endBatch(void)
	{
		GLsizeiptr size = (uint8_t*)s_rendererData.bufferPtr - (uint8_t*)s_rendererData.buffer;
		s_rendererData.vbo.bind();
		GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, size, s_rendererData.buffer));
	}
	
	void Renderer2D::flush(void)
	{
		for (uint32_t i = 0; i < s_rendererData.texSlotIndex; i++)
		{
			GLCall(glActiveTexture(GL_TEXTURE0 + i));
			GLCall(glBindTexture(GL_TEXTURE_2D, s_rendererData.texSlots[i]));
		}
		s_rendererData.vao.bind();
		GLCall(glDrawElements(GL_TRIANGLES, s_rendererData.indexCount, GL_UNSIGNED_INT, nullptr));
		s_rendererData.renderStats.drawCalls++;

		s_rendererData.indexCount = 0;
		s_rendererData.texSlotIndex = 1;
	}
	
	const Renderer2D::tRenderStats& Renderer2D::getRenderStats(void)
	{
		return s_rendererData.renderStats;
	}
	
	void Renderer2D::resetStats(void)
	{
		memset(&s_rendererData.renderStats, 0, sizeof(Renderer2D::tRenderStats));
	}

	void Renderer2D::drawQuad(const Transform2D& transform, const Color& color)
	{
		Renderer2D::drawQuad(transform.getVertices(), color);
	}

    void Renderer2D::drawQuad(const Transform2D& transform, ResourceID texture, TextureAtlasIndex tile_index, const Color& tint)
	{
		Renderer2D::drawQuad(transform.getVertices(), tint, texture, tile_index);
	}

	void Renderer2D::drawQuad(const std::vector<Vec2>& vertices, const Color& tintColor, ResourceID texture, TextureAtlasIndex tile_index)
	{
		if (vertices.size() != 4) return;
		if (s_rendererData.indexCount >= MaxIndexCount)
		{
			Renderer2D::endBatch();
			Renderer2D::flush();
			Renderer2D::beginBatch();
		}

		float z_val = 0.0f;
		float texIndex = 0.0f;
		auto texCoords = Texture::tTexCoords { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

		if (texture != ResourceManager::InvalidResource)
		{
			if (s_rendererData.texSlotIndex >= MaxTextures)
			{
				Renderer2D::endBatch();
				Renderer2D::flush();
				Renderer2D::beginBatch();
			}
			if (s_rendererData.currentTexResID != texture || s_rendererData.currentTex == nullptr || s_rendererData.currentTexResID == ResourceManager::InvalidResource)
			{
				s_rendererData.currentTex = &(ResourceManager::getTexture(texture));
				if (s_rendererData.currentTex->isInvalid())
				{
					ErrorHandler::pushError(Renderer2D::ERR_INVALID_TEXTURE);
					String err_str = ErrorHandler::getLastErrorString();
					OX_ERROR("ogfx::Renderer2D::drawQuad(const std::vector<Vec2>&, const Color&, ResourceID, TextureAtlasIndex): %s", err_str.c_str());
					return;
				}
				s_rendererData.currentTexResID = texture;
				s_rendererData.currentTexIndex = 0.0f;
				for (uint32_t i = 1; i < s_rendererData.texSlotIndex; i++)
				{
					if (s_rendererData.texSlots[i] == s_rendererData.currentTex->getOpenGLID())
					{
						s_rendererData.currentTexIndex = (float)i;
						break;
					}
				}

				if (s_rendererData.currentTexIndex == 0.0f)
				{
					s_rendererData.currentTexIndex = (float)s_rendererData.texSlotIndex;
					s_rendererData.texSlots[s_rendererData.texSlotIndex] = s_rendererData.currentTex->getOpenGLID();
					s_rendererData.texSlotIndex++;
				}
			}

			texCoords = s_rendererData.currentTex->getTile(tile_index);
			texIndex = s_rendererData.currentTexIndex;
		}
		
		s_rendererData.bufferPtr->color = tintColor.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = texCoords.topLeft;
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { vertices[0], z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = tintColor.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = texCoords.topRight;
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { vertices[1], z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = tintColor.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = texCoords.bottomRight;
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { vertices[2], z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = tintColor.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = texCoords.bottomLeft;
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { vertices[3], z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.indexCount += 6;
		s_rendererData.renderStats.quadCount++;
	}

	void Renderer2D::drawLine(const Vec2& start, const Vec2& end, float thickness, const Color& color)
	{
		if (s_rendererData.indexCount >= MaxIndexCount)
		{
			Renderer2D::endBatch();
			Renderer2D::flush();
			Renderer2D::beginBatch();
		}

		float texIndex = 0.0f;
		float z_val = 0.0f;

		Vec2 unitDirection = (start - end).normalize();
		Vec2 unitPerpendicular(-unitDirection.y, unitDirection.x);
		Vec2 offset = unitPerpendicular * (thickness / 2.f);

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 0.0f, 1.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { start + offset, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 1.0f, 1.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { end + offset, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 1.0f, 0.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { end - offset, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 0.0f, 0.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { start - offset, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.indexCount += 6;
		s_rendererData.renderStats.quadCount++;
	}

	void Renderer2D::drawTriangle(const Triangle& triangle, const Color& color)
	{
		if (s_rendererData.indexCount >= MaxIndexCount)
		{
			Renderer2D::endBatch();
			Renderer2D::flush();
			Renderer2D::beginBatch();
		}

		float texIndex = 0.0f;
		float z_val = 0.0f;

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 0.0f, 1.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { triangle.vA, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 1.0f, 1.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { triangle.vB, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 1.0f, 0.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { triangle.vC, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.bufferPtr->color = color.getNormalizedColor();
		s_rendererData.bufferPtr->texCoords = { 0.0f, 0.0f };
		s_rendererData.bufferPtr->texIndex = texIndex;
		s_rendererData.bufferPtr->position = { triangle.vC, z_val };
		s_rendererData.bufferPtr++;

		s_rendererData.indexCount += 6;
		s_rendererData.renderStats.quadCount++;
	}
	
	Shader& Renderer2D::bindShader(ResourceID shader)
	{
		if (shader != s_rendererData.currentShaderResID || s_rendererData.currentShader == nullptr || s_rendererData.currentShaderResID == ResourceManager::InvalidResource)
		{
			s_rendererData.currentShader = &(ResourceManager::getShader(shader));
			if (s_rendererData.currentShader->isInvalid())
			{
				ErrorHandler::pushError(Renderer2D::ERR_INVALID_SHADER);
				String err_str = ErrorHandler::getLastErrorString();
				OX_ERROR("ogfx::Renderer2D::bindShader(...): %s", err_str.c_str());
				return (Shader&)BaseObject::InvalidRef();
			}
			s_rendererData.currentShaderResID = shader;
			if (s_rendererData.indexCount > 0)
			{
				Renderer2D::endBatch();
				Renderer2D::flush();
			}
			s_rendererData.currentShader->bind();
			s_rendererData.renderStats.shaderBinds++;
			Renderer2D::beginBatch();
			return *s_rendererData.currentShader;
		}
		return *s_rendererData.currentShader;
	}

	void Renderer2D::drawSingle(const VertexArray& vao, const Shader& shader)
	{
		shader.bind();
		vao.bind();
		GLCall(glDrawElements(GL_TRIANGLES, vao.getElementCount(), GL_UNSIGNED_INT, 0));
	}

	void Renderer2D::drawText(const String& text, ResourceID font, Vec2 position, Color color, float charHeight, float spacing)
	{
		auto& _font = ResourceManager::getBitmapFont(font);
		const float baseCharHeight = _font.getBaseCharHeight();
		float scale = charHeight / baseCharHeight;
		float nextCharX = position.x;
		spacing *= scale;
		for (uint8_t i = 0; i < text.length(); i++)
		{
			Rectangle bounds;
			TextureAtlasIndex tile = _font.getChar(text[i], bounds);
			bounds.mulSize(scale, scale);
			bounds.mulPos(scale, scale);
			Renderer2D::drawQuad(Renderer2D::getStaticQuad({ nextCharX, position.y + bounds.y }, bounds.getSize(), false), color, _font.getTexture(), tile);
			nextCharX += bounds.w + spacing;
		}
	}

	void Renderer2D::clear(const Color& color, uint32_t gl_mask) 
    {
        const auto fcol = color.getNormalizedColor();
        glClearColor(fcol.r, fcol.g, fcol.b, fcol.a);
        glClear(gl_mask);
    }

	void Renderer2D::setRenderTarget(const RenderTarget& target)
	{
		if (target.isInvalid())
		{
			if (s_rendererData.currentRenderTarget != nullptr)
				setDefaultRenderTarget();
			return;
		}
		s_rendererData.currentRenderTarget = &target;
		if (s_rendererData.indexCount > 0)
		{
			Renderer2D::endBatch();
			Renderer2D::flush();
		}
		Renderer2D::beginBatch();
		s_rendererData.currentRenderTarget->bind();
		glViewport(0, 0, s_rendererData.currentRenderTarget->getSize().x, s_rendererData.currentRenderTarget->getSize().y);
		s_rendererData.renderStats.renderBufferBinds++;
		//Renderer2D::beginBatch();
	}

	void Renderer2D::setDefaultRenderTarget(void)
	{
		if (s_rendererData.indexCount > 0)
		{
			Renderer2D::endBatch();
			Renderer2D::flush();
		}
		Renderer2D::beginBatch();
		s_rendererData.currentRenderTarget = nullptr;
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		//TODO: Fix
		//glViewport(0, 0, (float)RTData::windowSize.x, (float)RTData::windowSize.y);
		s_rendererData.renderStats.renderBufferBinds++;
		//Renderer2D::beginBatch();
	}

	void Renderer2D::enableDepthTest(bool enable)
	{
		if (enable) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
	}

	const RenderTarget& Renderer2D::getCurrentRenderTarget(void)
	{
		if (s_rendererData.currentRenderTarget == nullptr)
			return (const RenderTarget&)BaseObject::InvalidRef();
		return *s_rendererData.currentRenderTarget;
	}

	const Shader& Renderer2D::getCurrentShader(void)
	{
		if (s_rendererData.currentShader == nullptr)
			return (const Shader&)BaseObject::InvalidRef();
		return *s_rendererData.currentShader;
	}

	ResourceID Renderer2D::getCurrentShaderID(void)
	{
		return s_rendererData.currentShaderResID;
	}

	std::vector<Vec2> Renderer2D::getStaticQuad(Vec2 position, Vec2 size, bool centered)
	{
		std::vector<Vec2> vertices;
		if (centered)
		{
			Vec2 s = (size / 2.0f);
			vertices.push_back({ position.x - s.x, position.y - s.y });
			vertices.push_back({ position.x + s.x, position.y - s.y });
			vertices.push_back({ position.x + s.x, position.y + s.y });
			vertices.push_back({ position.x - s.x, position.y + s.y });
		}
		else
		{
			vertices.push_back({ position.x, position.y });
			vertices.push_back({ position.x + size.x, position.y });
			vertices.push_back({ position.x + size.x, position.y + size.y });
			vertices.push_back({ position.x, position.y + size.y });
		}
		return vertices;
	}
}