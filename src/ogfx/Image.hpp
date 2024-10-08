#pragma once

#include <ostd/BaseObject.hpp>
#include <ogfx/SDLInclude.hpp>
#include <ostd/Geometry.hpp>

namespace ogfx
{
    class BasicRenderer2D;
    class Image : public ostd::BaseObject
    {
        public:
            inline Image(void) { invalidate(); }
            inline Image(const ostd::String& filePath, BasicRenderer2D& gfx) { loadFromFile(filePath, gfx); }
            inline ~Image(void) { destroy(); }
            void destroy(void);
            Image& loadFromFile(const ostd::String& filePath, BasicRenderer2D& gfx);
            inline ostd::IPoint getSize(void) const { return { m_width, m_height }; } 
            inline bool isLoaded(void) const { return m_loaded; }     
            inline SDL_Texture* getSDLTexture(void) const { return m_sdl_texture; }

        private:
            SDL_Texture* m_sdl_texture { nullptr };
            int32_t m_width { 0 };
            int32_t m_height { 0 };
            bool m_loaded { false };
    };
}
