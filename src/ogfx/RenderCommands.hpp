#ifndef __RENDER_COMMANDS_HPP__
#define __RENDER_COMMANDS_HPP__

#include <ostd/BaseObject.hpp>
#include <ostd/Types.hpp>
#include <ostd/Color.hpp>
#include <ogfx/Renderer2D.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove from header

	struct tRenderCommands
	{
		inline static constexpr uint8_t Invalid = 0x00;
		inline static constexpr uint8_t BindShader = 0x10;
		inline static constexpr uint8_t UpdateUniform = 0x11;
		inline static constexpr uint8_t DrawQuad = 0x12;
		inline static constexpr uint8_t DrawText = 0x13;
		inline static constexpr uint8_t Clear = 0x14;
		inline static constexpr uint8_t DrawLine = 0x15;
	};

	struct tRenderCommandErrors 
	{
		inline static constexpr uint8_t NoError = 0x00;
		inline static constexpr uint8_t InvalidShader = 0x01;
		inline static constexpr uint8_t UnknownUniformType = 0x02;
		inline static constexpr uint8_t InvalidatedRenderCommand = 0x03;
	};

	class RenderCommand : public BaseObject
	{
		public:
			inline RenderCommand(void) { m_commandID = tRenderCommands::Invalid; invalidate(); }
			inline virtual ~RenderCommand(void) {  }
			virtual uint8_t execute(void) = 0;
			inline uint8_t getCommandID(void) { return m_commandID; }

		protected:
			uint8_t m_commandID;
	};

	namespace rcom
	{
		class Clear;
		class DrawQuad;
		class DrawLine;
		class DrawText;
		class BindShader;
		template <typename T>
		class UpdateUniform;
	}
	class RenderCommandPool
	{
		public:
			static void init(void);
			static void destroy(void);
			static void newFrame(void);

			static RenderCommand* newClearCommand(const Color& clearColor);
			static RenderCommand* newDrawQuadCommand(const std::vector<Vec2>& vertices, const Color& color, TextureID texture);
			static RenderCommand* newDrawLineCommand(const Vec2& start, const Vec2& end, float thikness, const Color& color);
			static RenderCommand* newDrawTextCommand(const String& text, ResourceID font, const Vec2& position, const Color& color, float charHeight, float spacing);
			static RenderCommand* newBindSHaderCommand(ResourceID shader);

			static RenderCommand* newUpdateUniform_f_Command(const String& name, ResourceID shader, float value);
			static RenderCommand* newUpdateUniform_i_Command(const String& name, ResourceID shader, int32_t value);
			static RenderCommand* newUpdateUniform_b_Command(const String& name, ResourceID shader, bool value);
			static RenderCommand* newUpdateUniform_vec2_Command(const String& name, ResourceID shader, Vec2 value);
			static RenderCommand* newUpdateUniform_vec3_Command(const String& name, ResourceID shader, Vec3 value);
			static RenderCommand* newUpdateUniform_vec4_Command(const String& name, ResourceID shader, Vec4 value);
			static RenderCommand* newUpdateUniform_mat4_Command(const String& name, ResourceID shader, const glm::mat4& value);
			static RenderCommand* newUpdateUniform_col_Command(const String& name, ResourceID shader, const Color& value);
			static RenderCommand* newUpdateUniform_arri_Command(const String& name, ResourceID shader, const std::vector<int32_t>& value);
			static RenderCommand* newUpdateUniform_arrf_Command(const String& name, ResourceID shader, const std::vector<float>& value);

		private:
			inline static std::vector<rcom::Clear*> s_clearCmdPool;
			inline static uint32_t s_nextClrCmd { 0 };
			inline static std::vector<rcom::DrawQuad*> s_drawQuadPool;
			inline static uint32_t s_nextDrwQuadCmd { 0 };
			inline static std::vector<rcom::DrawText*> s_drawTextPool;
			inline static uint32_t s_nextDrwTextCmd { 0 };
			inline static std::vector<rcom::DrawLine*> s_drawLinePool;
			inline static uint32_t s_nextDrwLineCmd { 0 };
			inline static std::vector<rcom::BindShader*> s_bindShaderPool;
			inline static uint32_t s_nextBindShaderCmd { 0 };

			inline static std::vector<rcom::UpdateUniform<float>*> s_updUfQueue;
			inline static uint32_t s_nextUpdaUfCmd { 0 };
			inline static std::vector<rcom::UpdateUniform<int32_t>*> s_updUiQueue;
			inline static uint32_t s_nextUpdaUiCmd { 0 };
			inline static std::vector<rcom::UpdateUniform<bool>*> s_updUbQueue;
			inline static uint32_t s_nextUpdaUbCmd { 0 };
			inline static std::vector<rcom::UpdateUniform<Vec2>*> s_updUvec2Queue;
			inline static uint32_t s_nextUpdaUvec2Cmd { 0 };
			inline static std::vector<rcom::UpdateUniform<Vec3>*> s_updUvec3Queue;
			inline static uint32_t s_nextUpdaUvec3Cmd { 0 };
			inline static std::vector<rcom::UpdateUniform<Vec4>*> s_updUvec4Queue;
			inline static uint32_t s_nextUpdaUvec4Cmd { 0 };
			inline static std::vector<rcom::UpdateUniform<glm::mat4>*> s_updUmat4Queue;
			inline static uint32_t s_nextUpdaUmat4Cmd { 0 };
			inline static std::vector<rcom::UpdateUniform<Color>*> s_updUcolQueue;
			inline static uint32_t s_nextUpdaUcolCmd { 0 };
			inline static std::vector<rcom::UpdateUniform<std::vector<int32_t>>*> s_updUarriQueue;
			inline static uint32_t s_nextUpdaUarriCmd { 0 };
			inline static std::vector<rcom::UpdateUniform<std::vector<float>>*> s_updUarrfQueue;
			inline static uint32_t s_nextUpdaUarrfCmd { 0 };

			inline static bool s_initialized { false };
	};

	namespace rcom
	{
		class BindShader : public RenderCommand
		{
			public:
				BindShader(ResourceID shaderID);
				BindShader& create(ResourceID shaderID);
				uint8_t execute(void) override;

			private:
				ResourceID m_shaderID;
		};
		class Clear : public RenderCommand
		{
			public:
				Clear(const Color& clearColor);
				Clear& create(const Color& clearColor);
				uint8_t execute(void) override;

			private:
				Color m_clearColor;
		};
		class DrawQuad : public RenderCommand
		{
			public:
				DrawQuad(const std::vector<Vec2>& vertices, const Color& color, TextureID texture);
				DrawQuad& create(const std::vector<Vec2>& vertices, const Color& color, TextureID texture);
				uint8_t execute(void) override;

			private:
				Color m_color;
				std::vector<Vec2> m_vertices;
				TextureID m_texture;
		};
		class DrawLine : public RenderCommand
		{
			public:
				DrawLine(const Vec2& start, const Vec2& end, float thikness, const Color& color);
				DrawLine& create(const Vec2& start, const Vec2& end, float thikness, const Color& color);
				uint8_t execute(void) override;

			private:
				Vec2 m_start;
				Vec2 m_end;
				float m_thickness;
				Color m_color;
		};
		class DrawText : public RenderCommand
		{
			public:
				DrawText(const String& text, ResourceID font, const Vec2& position, const Color& color, float charHeight, float spacing);
				DrawText& create(const String& text, ResourceID font, const Vec2& position, const Color& color, float charHeight, float spacing);
				uint8_t execute(void) override;

			private:
				Renderer2D::tTextInfo m_textInfo;
				String m_text;
				Vec2 m_position;
		};
		template <typename T>
		class UpdateUniform : public RenderCommand
		{
			public:
				inline UpdateUniform(String name, ResourceID shaderID, T value)
				{
					m_commandID = tRenderCommands::UpdateUniform;
					setTypeName("ox::rcom::UpdateUniform");
					validate();
					create(name, shaderID, value);
				}
				inline UpdateUniform<T>& create(String name, ResourceID shaderID, T value)
				{
					m_shaderID = shaderID;
					m_value = value;
					m_name = name;
					return *this;
				}
				inline uint8_t execute(void) override
				{
					if (isInvalid()) return tRenderCommandErrors::InvalidatedRenderCommand;
					auto& shader = ResourceManager::getShader(m_shaderID);
					if (shader.isInvalid()) return tRenderCommandErrors::InvalidShader;
					if constexpr(std::is_same_v<T, float>)
						shader.updateUniform_f(m_name, m_value);
					else if constexpr(std::is_same_v<T, int32_t>)
						shader.updateUniform_i(m_name, m_value);
					else if constexpr(std::is_same_v<T, bool>)
						shader.updateUniform_b(m_name, m_value);
					else if constexpr(std::is_same_v<T, Vec2>)
						shader.updateUniform_vec2f(m_name, m_value);
					else if constexpr(std::is_same_v<T, Vec3>)
						shader.updateUniform_vec3f(m_name, m_value);
					else if constexpr(std::is_same_v<T, Vec4>)
						shader.updateUniform_vec4f(m_name, m_value);
					else if constexpr(std::is_same_v<T, glm::mat4>)
						shader.updateUniform_mat4f(m_name, m_value);
					else if constexpr(std::is_same_v<T, std::vector<int32_t>>)
						shader.updateUniform_arri(m_name, m_value.size(), &m_value[0]);
					else if constexpr(std::is_same_v<T, std::vector<float>>)
						shader.updateUniform_arrf(m_name, m_value.size(), &m_value[0]);
					else if constexpr(std::is_same_v<T, Color>)
						shader.updateUniform_vec4f(m_name, m_value);
					else
						return tRenderCommandErrors::UnknownUniformType;
					return tRenderCommandErrors::NoError;
				}

			private:
				T m_value;
				String m_name;
				ResourceID m_shaderID { ResourceManager::InvalidResource };
		};
	}
}

#endif