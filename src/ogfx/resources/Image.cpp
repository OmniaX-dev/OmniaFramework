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

#include "Image.hpp"
#include "../../io/Logger.hpp"
#include "../render/BasicRenderer.hpp"
#include "../gui/Window.hpp"

namespace ogfx
{
	void Image::destroy(void)
	{
		if (isValid() && isLoaded())
			SDL_DestroyTexture(m_sdl_texture);
		m_loaded = false;
		m_sdl_texture = nullptr;
		m_width = 0;
		m_height = 0;
		m_filePath = "";
		invalidate();
	}

	Image& Image::loadFromFile(const String& filePath, BasicRenderer2D& gfx)
	{
		if (isLoaded() && isValid())
			return *this;
		if (!gfx.isInitialized())
			return *this;  //TODO: Error
		m_sdl_texture = IMG_LoadTexture(gfx.getWindow().getSDLRenderer(), filePath.c_str());
		if (!m_sdl_texture)
		{
			OX_ERROR("Failed to load Image: %s", SDL_GetError());
			return *this;
		}
		SDL_GetTextureSize(m_sdl_texture, &m_width, &m_height);
		m_filePath = filePath;
		m_loaded = true;
		setTypeName("ogfx::Image");
		validate();
		return *this;
	}
}
