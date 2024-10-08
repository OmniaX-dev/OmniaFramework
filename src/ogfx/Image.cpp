#include "Image.hpp"
#include <SDL2/SDL_render.h>
#include <ogfx/BasicRenderer.hpp>
#include <ogfx/WindowBase.hpp>

namespace ogfx
{
    void Image::destroy(void)
    {
        SDL_DestroyTexture(m_sdl_texture);
        m_loaded = false;
        m_sdl_texture = nullptr;
        m_width = 0;
        m_height = 0;
    }

    Image& Image::loadFromFile(const ostd::String& filePath, BasicRenderer2D& gfx)
    {
        if (!gfx.isInitialized())
            return *this;  //TODO: Error
        m_sdl_texture = IMG_LoadTexture(gfx.getWindow().getSDLRenderer(), filePath.c_str());
        SDL_QueryTexture(m_sdl_texture, nullptr, nullptr, &m_width, &m_height);
        m_loaded = true;
        setTypeName("ogfx::Image");
        validate();
        return *this;
    }
}
