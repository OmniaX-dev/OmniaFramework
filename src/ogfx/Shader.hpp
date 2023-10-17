#ifndef __SHADER_HPP__
#define __SHADER_HPP__

#include <ostd/Types.hpp>
#include <ostd/BaseObject.hpp>
#include <unordered_map>
#include <vector>
#include <ostd/Geometry.hpp>
#include <ostd/Color.hpp>
#include <vendor/glm/glm.hpp>
#include <ostd/Defines.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove this from header
    class Shader : public ostd::BaseObject
	{
		public:
			inline Shader(void) { invalidate(); }
			inline Shader(String shaderName, bool singleFile = false, String shaderFolder = "", bool auto_register_uniforms = true) { createFromName(shaderName, singleFile, shaderFolder, auto_register_uniforms); }
			~Shader(void);
			Shader& createFromName(String shaderName, bool singleFile = false, String shaderFolder = "", bool auto_register_uniforms = true);
			Shader& createFromSources(String vertSource, String fragSource, bool auto_register_uniforms = true);
			Shader& createFromSeparateFiles(String vertFilePath, String fragFilePath, bool auto_register_uniforms = true);
			void bind(void) const;
			void unbind(void) const;
			inline uint32_t getOpenGLID(void) const { return static_cast<uint32_t>(getID()); }

			//TODO: Register struct uniforms automatically
			int32_t registerUniform(String uniform_name);
			int32_t gl_getUniformLocation(String uniform_name);
			int32_t getRegisteredUniform(String uniform_name);
			void updateUniform_f(String uniform_name, float value);
			void updateUniform_i(String uniform_name, int32_t value);
			void updateUniform_b(String uniform_name, bool value);
			void updateUniform_vec2f(String uniform_name, const Vec2& value);
			void updateUniform_vec3f(String uniform_name, const Vec3& value);
			void updateUniform_vec4f(String uniform_name, float x, float y, float z, float w);
			inline void updateUniform_vec4f(String uniform_name, const Vec4& value) { updateUniform_vec4f(uniform_name, value.x, value.y, value.z, value.w); }
			void updateUniform_vec4f(String uniform_name, const Color& value);
			void updateUniform_mat4f(String uniform_name, const glm::mat4& value);
			void updateUniform_arri(String uniform_name, int32_t array_size, const int32_t* value);
			void updateUniform_arrf(String uniform_name, int32_t array_size, const float* value);

			inline static void setShaderFolder(String folder_path) { Shader::s_shader_folder = folder_path; }
			inline static String getShaderFolder(void) { return Shader::s_shader_folder; }
			inline static void setFragmentShaderExtension(String frag_ext) { Shader::s_fragment_shader_extension = frag_ext; }
			inline static String getFragmentShaderExtension(void) { return Shader::s_fragment_shader_extension; }
			inline static void setVertexShaderExtension(String vert_ext) { Shader::s_vertex_shader_extension = vert_ext; }
			inline static String getVertexShaderExtension(void) { return Shader::s_vertex_shader_extension; }
			inline static void setSingleShaderExtension(String single_ext) { Shader::s_single_shader_extension = single_ext; }
			inline static String getSingleShaderExtension(void) { return Shader::s_single_shader_extension; }
			static bool readShaderSource(String shaderName, String& vertSource, String& fragSource);
			
			static uint32_t gl_compileShader(uint32_t gl_shader_type, const String& source);
			static uint32_t gl_createShaderProgram(const String& vertexShader, const String& fragmentShader);
			static uint32_t gl_loadShader(String shaderName);
			static uint32_t gl_loadShaderFromSingleFile(String shaderName);

			inline static uint32_t getUniformUpdateCount(void) { return s_uniform_updates; }
			inline static void resetUniformUpdateCount(void) { s_uniform_updates = 0; }

		private:
			void __auto_register_uniforms(void);
			bool __can_update_uniform(const String& uniform_name);
			static String linesToString(const std::vector<String>& lines);

		private:
			std::unordered_map<String, int32_t> m_uniforms;
			inline static std::vector<String> m_temp_uniform_names;
			inline static bool s_auto_register_uniforms { true };
			inline static uint32_t s_uniform_updates { 0 };

		private:
			inline static String s_shader_folder { "shaders" };
			inline static String s_vertex_shader_extension { "vs" };
			inline static String s_fragment_shader_extension { "fs" };
			inline static String s_single_shader_extension { "shad" };

		public:
			inline static constexpr int32_t ERR_SHADER_COMPILE_FAILED = OX_SHADER_ERR_MASK + 0x0001;
			inline static constexpr int32_t ERR_SHADER_SOURCE_READ_FILED = OX_SHADER_ERR_MASK + 0x0002;
			inline static constexpr int32_t ERR_FAILED_TO_REGISTER_UNIFORM = OX_SHADER_ERR_MASK + 0x0003;
			inline static constexpr int32_t ERR_FAILED_TO_UPDATE_UNIFORM = OX_SHADER_ERR_MASK + 0x0004;
	};
}

#endif
