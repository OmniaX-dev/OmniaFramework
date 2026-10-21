/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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
	Image::Image(Image&& other) noexcept
	{
		invalidate();
		*this = std::move(other);
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		if (this == &other)
			return *this;
		destroy();
		ostd::Object::operator=(other);
		m_renderer = other.m_renderer;
		m_sdl_texture = other.m_sdl_texture;
		m_filePath = other.m_filePath;
		m_width = other.m_width;
		m_height = other.m_height;
		m_loaded = other.m_loaded;
		// The renderer tracks images by address: hand the registration over.
		if (m_renderer)
		{
			m_renderer->unregister_image(other);
			m_renderer->register_image(*this);
		}
		other.m_renderer = nullptr;
		other.m_sdl_texture = nullptr;
		other.m_loaded = false;
		other.m_filePath = "";
		other.m_width = 0;
		other.m_height = 0;
		other.invalidate();
		return *this;
	}

	void Image::destroy(void)
	{
		__release_texture();
		if (m_renderer)
			m_renderer->unregister_image(*this);
		m_renderer = nullptr;
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
		m_renderer = &gfx;
		gfx.register_image(*this);
		setTypeName("ogfx::Image");
		validate();
		return *this;
	}

	void Image::__reload(void)
	{
		if (!m_renderer || m_filePath == "")
			return;
		__release_texture();
		m_sdl_texture = IMG_LoadTexture(m_renderer->getSDLRenderer(), m_filePath.c_str());
		if (!m_sdl_texture)
		{
			OX_ERROR("Failed to reload Image after device reset: %s", SDL_GetError());
			m_loaded = false;
			return;
		}
		SDL_GetTextureSize(m_sdl_texture, &m_width, &m_height);
		m_loaded = true;
	}

	void Image::__release_texture(void)
	{
		// Without a live renderer the texture was already destroyed along with the SDL renderer.
		if (m_sdl_texture && m_renderer && m_renderer->isInitialized())
			SDL_DestroyTexture(m_sdl_texture);
		m_sdl_texture = nullptr;
		m_loaded = false;
	}

	void Image::__detach_from_renderer(void)
	{
		__release_texture();
		m_renderer = nullptr;
	}
}
