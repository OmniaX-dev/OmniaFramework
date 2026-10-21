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

#include "RenderTarget.hpp"
#include "BasicRenderer.hpp"
#include <ostd/io/Logger.hpp>

namespace ogfx
{
	// ===================================================== RENDER TARGET =====================================================
	RenderTarget::~RenderTarget(void)
	{
		destroy();
	}

	bool RenderTarget::create(BasicRenderer2D& renderer, const Vec2& size)
	{
		destroy();
		m_renderer = &renderer;
		m_size = { std::max(0.0f, size.x), std::max(0.0f, size.y) };
		m_contentsLost = true;
		renderer.register_target(*this);
		validate();
		// Before the renderer is initialized this just records the size: the texture is created
		// on first use.
		if (renderer.isInitialized())
			return ensure_texture();
		return true;
	}

	void RenderTarget::destroy(void)
	{
		if (m_renderer)
		{
			m_renderer->unregister_target(*this);
			release_texture();
		}
		m_renderer = nullptr;
		m_texture = nullptr;
		m_size = { 0, 0 };
		m_pixelSize = { 0, 0 };
		m_scale = 1.0f;
		m_contentsLost = false;
		invalidate();
	}

	bool RenderTarget::resize(const Vec2& size)
	{
		if (!m_renderer)
			return false;
		const Vec2 newSize { std::max(0.0f, size.x), std::max(0.0f, size.y) };
		if (newSize.x == m_size.x && newSize.y == m_size.y)
			return true;
		m_size = newSize;
		if (!m_renderer->isInitialized())
			return true;
		return ensure_texture();
	}

	bool RenderTarget::ensure_texture(void)
	{
		if (!m_renderer || !m_renderer->isInitialized())
			return false;
		if (m_size.x <= 0 || m_size.y <= 0)
			return false;
		const f32 scale = m_renderer->getPixelScale();
		const IPoint pixelSize { std::max(1, cast<i32>(std::ceil(m_size.x * scale))), std::max(1, cast<i32>(std::ceil(m_size.y * scale))) };
		if (m_texture && pixelSize.x == m_pixelSize.x && pixelSize.y == m_pixelSize.y && scale == m_scale)
			return true;

		release_texture();
		SDL_Texture* texture = SDL_CreateTexture(m_renderer->getSDLRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, pixelSize.x, pixelSize.y);
		if (!texture)
		{
			OX_ERROR("RenderTarget: failed to create a %dx%d texture: %s", pixelSize.x, pixelSize.y, SDL_GetError());
			return false;
		}
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
		m_texture = texture;
		m_pixelSize = pixelSize;
		m_scale = scale;
		// A new texture has undefined contents.
		m_renderer->clear_texture(m_texture);
		m_contentsLost = true;
		return true;
	}

	void RenderTarget::release_texture(void)
	{
		// Once the renderer has released its device resources the SDL renderer (and with it
		// every texture) is gone: there is nothing left to destroy.
		if (m_texture && m_renderer && m_renderer->isInitialized())
			SDL_DestroyTexture(m_texture);
		m_texture = nullptr;
	}

	void RenderTarget::notify_contents_lost(void)
	{
		m_contentsLost = true;
		if (m_contentsLostCallback)
			m_contentsLostCallback(*this);
	}

	void RenderTarget::detach_from_renderer(void)
	{
		release_texture();
		m_renderer = nullptr;
		m_texture = nullptr;
		invalidate();
	}
	// ===================================================== RENDER TARGET =====================================================





	// ===================================================== SCOPED RENDER TARGET =====================================================
	ScopedRenderTarget::ScopedRenderTarget(BasicRenderer2D& renderer, RenderTarget& target, bool clear, const Color& clearColor) : m_renderer(renderer)
	{
		m_active = m_renderer.beginRenderTarget(target, clear, clearColor);
	}

	ScopedRenderTarget::~ScopedRenderTarget(void)
	{
		if (m_active)
			m_renderer.endRenderTarget();
	}
	// ===================================================== SCOPED RENDER TARGET =====================================================
}
