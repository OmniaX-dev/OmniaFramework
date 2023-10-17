#include "Shader.hpp"
#include <Utils.hpp>
#include <glad/glad.h>
#include <Logger.hpp>
#include <Defines.hpp>
#include "Errors.hpp"

//TODO: Implement Errors

namespace ogfx
{
	using namespace ostd;

    Shader::~Shader(void)
    {
        glDeleteProgram(getOpenGLID());
    }

    Shader& Shader::createFromName(String shaderName, bool singleFile, String shaderFolder, bool auto_register_uniforms)
    {
        StringEditor se = shaderFolder;
        se.trim();
        bool use_custom_shader_folder = se.str() != "";
        String tmp_shader_folder = "";
        if (use_custom_shader_folder) 
        {
            tmp_shader_folder = Shader::getShaderFolder();
            Shader::setShaderFolder(se.str());
        }
        uint64_t shader_id;
        if (!singleFile) shader_id = Shader::gl_loadShader(shaderName);
        else shader_id = Shader::gl_loadShaderFromSingleFile(shaderName);
        if (use_custom_shader_folder)
            Shader::setShaderFolder(tmp_shader_folder);
        setID(shader_id);
        if (shader_id == 0) return *this;
        setTypeName("ox::Shader");
        validate();
        bind();
        Shader::s_auto_register_uniforms = auto_register_uniforms;
        __auto_register_uniforms();
        unbind();
        return *this;
    }

    Shader& Shader::createFromSources(String vertSource, String fragSource, bool auto_register_uniforms)
    {
        uint64_t shader_id = Shader::gl_createShaderProgram(vertSource, fragSource);
        setID(shader_id);
        if (shader_id == 0) return *this;
        setTypeName("ox::Shader");
        validate();
        bind();
        Shader::s_auto_register_uniforms = auto_register_uniforms;
        __auto_register_uniforms();
        unbind();
        return *this;
    }

	Shader& Shader::createFromSeparateFiles(String vertFilePath, String fragFilePath, bool auto_register_uniforms)
	{
		std::vector<String> vertLines;
        std::vector<String> fragLines;
        if (!Utils::readFile(vertFilePath, vertLines))
		{
            ErrorHandler::pushError(Shader::ERR_SHADER_SOURCE_READ_FILED);
            String error_str = ErrorHandler::getLastErrorString();
            OX_ERROR("%s.\nShader path: %s (vertex)", error_str.c_str(), vertFilePath.c_str());
			return *this;
		}
        if (!Utils::readFile(fragFilePath, fragLines))
		{
            ErrorHandler::pushError(Shader::ERR_SHADER_SOURCE_READ_FILED);
            String error_str = ErrorHandler::getLastErrorString();
            OX_ERROR("%s.\nShader path: %s (fragment)", error_str.c_str(), fragFilePath.c_str());
			return *this;
		};
        String vertSource = Shader::linesToString(vertLines);
        String fragSource = Shader::linesToString(fragLines);
        createFromSources(vertSource, fragSource);
        bind();
        Shader::s_auto_register_uniforms = auto_register_uniforms;
        __auto_register_uniforms();
        unbind();
        return *this;
	}

    void Shader::bind(void) const { glUseProgram(getOpenGLID()); }
    
    void Shader::unbind(void) const { glUseProgram(0); }

    int32_t Shader::registerUniform(String uniform_name)
    {
        int32_t uni_loc = glGetUniformLocation(getOpenGLID(), uniform_name.c_str());
        if (uni_loc == -1)
        {
            ErrorHandler::pushError(Shader::ERR_FAILED_TO_REGISTER_UNIFORM);
			String error_str = ErrorHandler::getLastErrorString();
            OX_ERROR("%s:\nUniform name: <%s>", error_str.c_str(), uniform_name.c_str());
        }
        m_uniforms[uniform_name] = uni_loc;
        return OX_NO_ERROR;
    }

    int32_t Shader::gl_getUniformLocation(String uniform_name)
    {
        return glGetUniformLocation(getOpenGLID(), uniform_name.c_str());
    }
    
    int32_t Shader::getRegisteredUniform(String uniform_name)
    {
        if (m_uniforms.find(uniform_name) == m_uniforms.end()) return -1;
        return m_uniforms.at(uniform_name);
    }
    
    
    void Shader::updateUniform_f(String uniform_name, float value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform1f(m_uniforms.at(uniform_name), value);
        Shader::s_uniform_updates++;
    }

    void Shader::updateUniform_i(String uniform_name, int32_t value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform1i(m_uniforms.at(uniform_name), value);
        Shader::s_uniform_updates++;
    }
    
    void Shader::updateUniform_b(String uniform_name, bool value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform1i(m_uniforms.at(uniform_name), (int)value);
        Shader::s_uniform_updates++;
    }
    
    void Shader::updateUniform_vec2f(String uniform_name, const Vec2& value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform2f(m_uniforms.at(uniform_name), value.x, value.y);
        Shader::s_uniform_updates++;
    }
    
    void Shader::updateUniform_vec3f(String uniform_name, const Vec3& value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform3f(m_uniforms.at(uniform_name), value.x, value.y, value.z);
        Shader::s_uniform_updates++;
    }
    
    void Shader::updateUniform_vec4f(String uniform_name, float x, float y, float z, float w)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform4f(m_uniforms.at(uniform_name), x, y, z, w);
        Shader::s_uniform_updates++;
    }
    
    void Shader::updateUniform_vec4f(String uniform_name, const Color& value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        const auto fcol = value.getNormalizedColor();
        glUniform4f(m_uniforms.at(uniform_name), fcol.r, fcol.g, fcol.b, fcol.a);
        Shader::s_uniform_updates++;
    }

    void Shader::updateUniform_mat4f(String uniform_name, const glm::mat4& value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniformMatrix4fv(m_uniforms.at(uniform_name), 1, GL_FALSE, &value[0][0]);
        Shader::s_uniform_updates++;
    }
    
    void Shader::updateUniform_arri(String uniform_name, int32_t array_size, const int32_t* value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform1iv(m_uniforms.at(uniform_name), array_size, value);
        Shader::s_uniform_updates++;
    }
    
    void Shader::updateUniform_arrf(String uniform_name, int32_t array_size, const float* value)
    {
        if (!__can_update_uniform(uniform_name)) return;
        glUniform1fv(m_uniforms.at(uniform_name), array_size, value);
        Shader::s_uniform_updates++;
    }
    




    bool Shader::readShaderSource(String shaderName, String& vertSource, String& fragSource)
    {
        std::vector<String> vertLines;
        std::vector<String> fragLines;
        if (!Utils::readFile(Shader::s_shader_folder + "/" + shaderName + "." + Shader::s_vertex_shader_extension, vertLines)) return false;
        if (!Utils::readFile(Shader::s_shader_folder + "/" + shaderName + "." + Shader::s_fragment_shader_extension, fragLines)) return false;
        vertSource = Shader::linesToString(vertLines);
        fragSource = Shader::linesToString(fragLines);
        return true;
    }
    
    uint32_t Shader::gl_compileShader(uint32_t gl_shader_type, const String& source)
    {
        if (Shader::s_auto_register_uniforms)
        {
            auto tokens = StringEditor(source).tokenize(";", true);
            for (auto line : tokens)
            {
				bool skip_next_uniform = false;
                StringEditor se = line;
				if (se.startsWith("//@[skip_auto_register]"))
				{
					skip_next_uniform = true;
				}
				else if(se.startsWith(("uniform ")))
                {
					if (skip_next_uniform) continue;
                    se = se.substr(8);
                    se.trim();
                    if (se.len() == 0) continue;
                    int32_t index = se.indexOf(" ");
                    if (index <= 0) continue;
                    se = se.substr(index + 1);
                    se.trim();
                    index = se.indexOf("[");
                    if (index > 0)
                    {
                        se = se.substr(0, index);
                        se.trim();
                    }
                    Shader::m_temp_uniform_names.push_back(se.str());
                }
            }
        }
        uint32_t id = glCreateShader(gl_shader_type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        int32_t result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int32_t length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length * sizeof(char));
            glGetShaderInfoLog(id, length, &length, message);
            String shader_type_str = (gl_shader_type == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex");
            ErrorHandler::pushError(Shader::ERR_SHADER_COMPILE_FAILED);
            String error_str = ErrorHandler::getLastErrorString();
            OX_ERROR("%s: <%s shader>.\nGLSL Error:\n%s", error_str.c_str(), shader_type_str.c_str(), message);
            glDeleteShader(id);
            return 0;
        }
        return id;
    }

    uint32_t Shader::gl_createShaderProgram(const String& vertexShader, const String& fragmentShader)
    {
        uint32_t program = glCreateProgram();
        uint32_t vs = Shader::gl_compileShader(GL_VERTEX_SHADER, vertexShader);
		if (vs == 0) return 0;
        uint32_t fs = Shader::gl_compileShader(GL_FRAGMENT_SHADER, fragmentShader);
		if (fs == 0) return 0;
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glValidateProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    }

    uint32_t Shader::gl_loadShader(String shaderName)
    {
        String vertSource;
        String fragSource;
        if (!Shader::readShaderSource(shaderName, vertSource, fragSource))
        {
            ErrorHandler::pushError(Shader::ERR_SHADER_SOURCE_READ_FILED);
            String error_str = ErrorHandler::getLastErrorString();
            OX_ERROR("%s.\nShader name: %s", error_str.c_str(), shaderName.c_str());
            return 0;
        }
        return Shader::gl_createShaderProgram(vertSource, fragSource);
    }

    uint32_t Shader::gl_loadShaderFromSingleFile(String shaderName)
    {
        constexpr uint8_t VERTEX = 0;
        constexpr uint8_t FRAGMENT = 1;
        constexpr uint8_t NONE = 255;
        uint8_t mode = NONE;
        StringEditor sources[2];
        std::vector<String> lines;
        if (!Utils::readFile(Shader::s_shader_folder + "/" + shaderName + "." + Shader::s_single_shader_extension, lines))
        {
            ErrorHandler::pushError(Shader::ERR_SHADER_SOURCE_READ_FILED);
            String error_str = ErrorHandler::getLastErrorString();
            OX_ERROR("%s.\nShader name: %s (single file)", error_str.c_str(), shaderName.c_str());;
            return 0;
        }
        StringEditor sel;
        for (auto& line : lines)
        {
            sel = line;
            sel.trim().toLower();
            if (sel.startsWith("#version "))
            {
                sources[VERTEX].add(line);
                sources[FRAGMENT].add(line);
                continue;
            }
            else if (sel.startsWith("@shader_type"))
            {
                sel = sel.substr(12);
                sel.trim();
                if (sel.str() == "[vertex]")
                    mode = VERTEX;
                else if (sel.str() == "[fragment]")
                    mode = FRAGMENT;
                else
                    mode = NONE;
                continue;
            }
            else if (mode != NONE)
            {
                sources[mode].add(line).add("\n");
				continue;
            }
        }
        return Shader::gl_createShaderProgram(sources[VERTEX].str(), sources[FRAGMENT].str());
    }

    void Shader::__auto_register_uniforms(void)
    {
        if (Shader::s_auto_register_uniforms)
        {
            for (auto& unif : Shader::m_temp_uniform_names)
                registerUniform(unif);
        }
        Shader::s_auto_register_uniforms = true;
        std::vector<String>().swap(Shader::m_temp_uniform_names);
    }

    bool Shader::__can_update_uniform(const String& uniform_name)
    {
        if (m_uniforms.find(uniform_name) == m_uniforms.end())
        {
            ErrorHandler::pushError(Shader::ERR_FAILED_TO_UPDATE_UNIFORM);
            String error_str = ErrorHandler::getLastErrorString();
            OX_ERROR("%s:\nUniform name: <%s>", error_str.c_str(), uniform_name.c_str());
            return false;
        }
        return true;
    }

    String Shader::linesToString(const std::vector<String>& lines)
    {
        String full;
        for (auto& line : lines)
            full += line + "\n";
        return full;
    }
}
