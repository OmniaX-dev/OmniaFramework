/*
    OmniaFramework - A collection of useful functionality
    Copyright (C) 2025  OmniaX-Dev

    This file is part of OmniaFramework.

    OmniaFramework is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OmniaFramework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OmniaFramework.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <ostd/data/BaseObject.hpp>
#include <ogfx/utils/SDLInclude.hpp>
#include <ostd/math/Geometry.hpp>

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
            inline ostd::Vec2 getSize(void) const { return { m_width, m_height }; }
            inline bool isLoaded(void) const { return m_loaded; }
            inline SDL_Texture* getSDLTexture(void) { return m_sdl_texture; }

        private:
            SDL_Texture* m_sdl_texture { nullptr };
            float m_width { 0 };
            float m_height { 0 };
            bool m_loaded { false };
    };
}
