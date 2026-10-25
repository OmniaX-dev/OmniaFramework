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
		m_streaming = other.m_streaming;
		m_smoothFiltering = other.m_smoothFiltering;
		m_contentsLost = other.m_contentsLost;
		// The renderer tracks images by address: hand the registration over.
		if (m_renderer)
		{
			m_renderer->unregister_image(other);
			m_renderer->register_image(*this);
		}
		other.m_renderer = nullptr;
		other.m_sdl_texture = nullptr;
		other.m_loaded = false;
		other.m_streaming = false;
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
		m_streaming = false;
		m_contentsLost = false;
		m_sdl_texture = nullptr;
		m_width = 0;
		m_height = 0;
		m_filePath = "";
		invalidate();
	}

	Image& Image::create(BasicRenderer2D& gfx, i32 width, i32 height)
	{
		destroy();
		if (!gfx.isInitialized() || width <= 0 || height <= 0)
			return *this;  //TODO: Error
		m_renderer = &gfx;
		m_width = cast<f32>(width);
		m_height = cast<f32>(height);
		m_streaming = true;
		gfx.register_image(*this);
		setTypeName("ogfx::Image");
		validate();
		create_streaming_texture();
		return *this;
	}

	bool Image::create_streaming_texture(void)
	{
		m_sdl_texture = SDL_CreateTexture(m_renderer->getSDLRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, cast<i32>(m_width), cast<i32>(m_height));
		if (!m_sdl_texture)
		{
			OX_ERROR("Failed to create a %dx%d Image: %s", cast<i32>(m_width), cast<i32>(m_height), SDL_GetError());
			m_loaded = false;
			return false;
		}
		SDL_SetTextureBlendMode(m_sdl_texture, SDL_BLENDMODE_BLEND);
		SDL_SetTextureScaleMode(m_sdl_texture, m_smoothFiltering ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
		m_loaded = true;
		m_contentsLost = true;
		return true;
	}

	bool Image::updatePixels(const u32* pixels, const IRect& area)
	{
		if (!m_streaming || !m_sdl_texture || !pixels)
			return false;
		const i32 width = cast<i32>(m_width), height = cast<i32>(m_height);
		i32 x0 = area.position.x, y0 = area.position.y;
		i32 x1 = x0 + area.size.x, y1 = y0 + area.size.y;
		const bool full = area.size.x <= 0 || area.size.y <= 0;
		if (full)
		{
			x0 = 0; y0 = 0; x1 = width; y1 = height;
		}
		x0 = std::max(x0, 0); y0 = std::max(y0, 0);
		x1 = std::min(x1, width); y1 = std::min(y1, height);
		if (x0 >= x1 || y0 >= y1)
			return true;
		const SDL_Rect rect { x0, y0, x1 - x0, y1 - y0 };
		if (!SDL_UpdateTexture(m_sdl_texture, &rect, pixels + cast<size_t>(y0) * width + x0, width * cast<i32>(sizeof(u32))))
		{
			OX_ERROR("Image::updatePixels failed: %s", SDL_GetError());
			return false;
		}
		if (full || (x0 == 0 && y0 == 0 && x1 == width && y1 == height))
			m_contentsLost = false;
		return true;
	}

	void Image::setSmoothFiltering(bool smooth)
	{
		m_smoothFiltering = smooth;
		if (m_sdl_texture)
			SDL_SetTextureScaleMode(m_sdl_texture, smooth ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
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
		if (m_renderer && m_streaming)
		{
			__release_texture();
			create_streaming_texture();
			return;
		}
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
		SDL_SetTextureScaleMode(m_sdl_texture, m_smoothFiltering ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
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
