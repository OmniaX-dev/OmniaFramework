#include "Errors.hpp"
#include <Utils.hpp>
#include <ogfx/Shader.hpp>
#include <Defines.hpp>
#include <Logger.hpp>
#include <ogfx/GLBuffers.hpp>
#include <ogfx/Texture.hpp>
#include <ogfx/BitmapFont.hpp>

namespace ogfx
{
    using namespace ostd;

    String ErrorHandler::mapError(int32_t err_code)
    {
        StringEditor se;
        se.add(Utils::getHexStr(err_code, true, 4)).add(": ");
        switch (err_code)
        {
            case OX_NO_ERROR: return "";

            /** ox::Shader Errors **/
            case Shader::ERR_SHADER_COMPILE_FAILED: return se.add("A Shader failed to compile").str();
            case Shader::ERR_SHADER_SOURCE_READ_FILED: return se.add("Failed to read shader source code").str();
            case Shader::ERR_FAILED_TO_REGISTER_UNIFORM: return se.add("Failed to register uniform").str();
            case Shader::ERR_FAILED_TO_UPDATE_UNIFORM: return se.add("Failed to update uniform").str();

            /** ox::VertexArray Errors **/
            case VertexArray::ERR_NO_EBO_SET: return se.add("No ElementBuffer set in VertexArray.").str();

            /** ox::Texture Errors **/
            case Texture::ERR_IMAGE_LOAD_FAILED: return se.add("Failed to load Texture.").str();
            case Texture::ERR_NO_DATA_STORED: return se.add("Attempting to retrieve local Texture data of a non-stored instance.").str();

            /** ox::BitmapFont Errors **/
            case BitmapFont::ERR_INVALID_TEXTURE: return se.add("Invalid BitmapFont Texture.").str();
            case BitmapFont::ERR_INVALID_TEXTURE_SIZE: return se.add("Invalid Size for ox::BitmapFont Texture: width and height must both be multiples of 16.").str();

            /** Custom or undefined Errors **/
            default:
            {
                if (ErrorHandler::s_customMapErrorCallback != nullptr)
                    return (*ErrorHandler::s_customMapErrorCallback)(err_code);
            }
        }
        return "Unknown Error.";
    }

    void ErrorHandler::pushError(int32_t err_code)
    {
        if (err_code != OX_NO_ERROR)
            s_errorStack.push_back(err_code);
    }

    String ErrorHandler::getLastErrorString(void)
    {
        if (ErrorHandler::s_errorStack.size() == 0) return "";
        return ErrorHandler::mapError(ErrorHandler::s_errorStack[s_errorStack.size() - 1]);
    }



    void ErrorHandler::__gl_clearErrors(void)
    {
        while(glGetError() != GL_NO_ERROR);
    }

    bool ErrorHandler::__gl_printError(const char* func_call, const char* file_path, int32_t line_number)
    {
        bool res = false;
		StringEditor se;
        while (GLenum err_code = glGetError())
        {
            se = "[OpenGL]: (err_code = ";
            se.add(Utils::getHexStr(err_code, true, 4)).add(")");
            OX_ERROR("%s:\nFile: %s on Line %d\n\t%s;\n", se.c_str(), file_path, line_number, func_call);
            res = true;
        }
        return res;
    }
}
