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

#include "FontUtils.hpp"
#include "../resources/UbuntuMonoRegularTTF.hpp"

namespace ogfx
{
	TTFRenderer::SignalHandler::SignalHandler(TTFRenderer& parent) : m_parent(parent)
	{
		enableSignals();
		connectSignal(ostd::tBuiltinSignals::BeforeSDLShutdown);
		setTypeName("ostd::GraphicsWindowOutputHandler::SignalHandler");
		validate();
	}

	void TTFRenderer::SignalHandler::handleSignal(ostd::tSignal& signal)
	{
		if (signal.ID == ostd::tBuiltinSignals::BeforeSDLShutdown)
		{
			m_parent.closeFont();
		}
	}



	TTFRenderer::~TTFRenderer(void)
	{
		closeFont();
	}

	void TTFRenderer::renderText(const ostd::String& message, int32_t x, int32_t y, const ostd::Color& color, int32_t fontSize)
	{
		if (!TTFRenderer::m_initialized)
		{
			set_error_state(tErrors::Uninitialized);
			return;
		}
		if (!m_fontOpen)
		{
			set_error_state(tErrors::NoFont);
			return;
		}
		if (m_renderer == nullptr)
		{
			set_error_state(tErrors::NullRenderer);
			return;
		}
		if (m_font == nullptr)
		{
			set_error_state(tErrors::NullFont);
			return;
		}
		setFontSize(fontSize);
		SDL_Surface* surf = TTF_RenderText_Blended(m_font, message.c_str(), message.len(), { color.r, color.g, color.b, color.a });
		if (surf == nullptr)
		{
			set_error_state(tErrors::TTFRenderTextBlendedFail);
			print_ttf_error("TTF_RenderText_Blended");
			return;
		}
		SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surf);
		if (texture == nullptr)
		{
			set_error_state(tErrors::TTFCreateTextureFromSurfaceFail);
			print_ttf_error("SDL_CreateTextureFromSurface");
			return;
		}
		SDL_DestroySurface(surf);

		float w = 0, h = 0;
		SDL_GetTextureSize(texture, &w, &h);

		SDL_FRect dst;
		dst.x = x;
		dst.y = y;
		dst.w = w;
		dst.h = h;
		SDL_RenderTexture(m_renderer, texture, NULL, &dst);
		SDL_DestroyTexture(texture);
	}

	void TTFRenderer::renderCenteredText(const ostd::String& message, int32_t center_x, int32_t center_y, const ostd::Color& color, int32_t fontSize)
	{
		if (!TTFRenderer::m_initialized)
		{
			set_error_state(tErrors::Uninitialized);
			return;
		}
		if (!m_fontOpen)
		{
			set_error_state(tErrors::NoFont);
			return;
		}
		if (m_renderer == nullptr)
		{
			set_error_state(tErrors::NullRenderer);
			return;
		}
		if (m_font == nullptr)
		{
			set_error_state(tErrors::NullFont);
			return;
		}
		setFontSize(fontSize);
		SDL_Surface* surf = TTF_RenderText_Blended(m_font, message.c_str(), message.len(), { color.r, color.g, color.b, color.a });
		if (surf == nullptr)
		{
			set_error_state(tErrors::TTFRenderTextBlendedFail);
			print_ttf_error("TTF_RenderText_Blended");
			return;
		}
		SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surf);
		if (texture == nullptr)
		{
			set_error_state(tErrors::TTFCreateTextureFromSurfaceFail);
			print_ttf_error("SDL_CreateTextureFromSurface");
			return;
		}
		SDL_DestroySurface(surf);

		float w = 0, h = 0;
		SDL_GetTextureSize(texture, &w, &h);

		SDL_FRect dst;
		dst.x = center_x - (w / 2);
		dst.y = center_y - (h / 2);
		dst.w = w;
		dst.h = h;
		SDL_RenderTexture(m_renderer, texture, NULL, &dst);
		SDL_DestroyTexture(texture);
	}

	ostd::IPoint TTFRenderer::getStringDimensions(const ostd::String& message, int32_t fontSize)
	{
		if (!TTFRenderer::m_initialized)
		{
			set_error_state(tErrors::Uninitialized);
			return { 0, 0 };
		}
		if (!m_fontOpen)
		{
			set_error_state(tErrors::NoFont);
			return { 0, 0 };
		}
		if (m_renderer == nullptr)
		{
			set_error_state(tErrors::NullRenderer);
			return { 0, 0 };
		}
		if (m_font == nullptr)
		{
			set_error_state(tErrors::NullFont);
			return { 0, 0 };
		}
		setFontSize(fontSize);
		SDL_Surface* surf = TTF_RenderText_Blended(m_font, message.c_str(), message.len(), { 0, 0, 0, 255 });
		if (surf == nullptr)
		{
			set_error_state(tErrors::TTFRenderTextBlendedFail);
			print_ttf_error("TTF_RenderText_Blended");
			return { 0, 0 };
		}
		SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surf);
		if (texture == nullptr)
		{
			set_error_state(tErrors::TTFCreateTextureFromSurfaceFail);
			print_ttf_error("SDL_CreateTextureFromSurface");
			return { 0, 0 };
		}
		SDL_DestroySurface(surf);

		float w = 0, h = 0;
		SDL_GetTextureSize(texture, &w, &h);
		SDL_DestroyTexture(texture);
		return { static_cast<int32_t>(w), static_cast<int32_t>(h) };
	}

	int32_t TTFRenderer::init(SDL_Renderer* renderer)
	{
		if (TTFRenderer::m_initialized) return set_error_state(tErrors::NoError);
		if (renderer == nullptr) return set_error_state(tErrors::NullRenderer);
		m_renderer = renderer;
		TTFRenderer::m_initialized = true;
		return set_error_state(tErrors::NoError);
	}

	int32_t TTFRenderer::loadDefaultFont(int32_t fontSize)
	{
		return openFont(ubuntu_mono_regular_ttf_data, ubuntu_mono_regular_ttf_size, fontSize);
	}

	void TTFRenderer::closeFont(void)
	{
		if (!TTFRenderer::m_initialized || !m_fontOpen) return;
		if (m_font != nullptr)
			TTF_CloseFont(m_font);
		m_font = nullptr;
		m_fontOpen = false;
		m_fontSize = DefaultFontSize;
		m_fontFromMemory = false;
		set_error_state(tErrors::NoError);
	}

	int32_t TTFRenderer::openFont(const ostd::String& fontPath, int32_t fontSize)
	{
		if (!TTFRenderer::m_initialized) return set_error_state(tErrors::Uninitialized);
		if (m_fontOpen) closeFont();
		if (fontSize <= 0)
			fontSize = m_fontSize;
		else
			m_fontSize = fontSize;
		m_font = TTF_OpenFont(fontPath.c_str(), fontSize);
		if (m_font == nullptr)
		{
			print_ttf_error("TTF_OpenFont");
			return set_error_state(tErrors::FailedToOpenFontFile);
		}
		m_fontOpen = true;
		m_fontFromMemory = false;
		return set_error_state(tErrors::NoError);
	}

	int32_t TTFRenderer::openFont(const ostd::UByte resource_data[], uint32_t data_size, int32_t fontSize)
	{
		if (!TTFRenderer::m_initialized) return set_error_state(tErrors::Uninitialized);
		if (data_size < 100) // Arbitrary 100 bytes
			return set_error_state(tErrors::FailedToOpenFontFile);
		if (m_fontOpen) closeFont();
		if (fontSize <= 0)
			fontSize = m_fontSize;
		else
			m_fontSize = fontSize;
		SDL_IOStream* rw = SDL_IOFromConstMem(resource_data, data_size);
		m_font = TTF_OpenFontIO(rw, true, fontSize);
		if (m_font == nullptr)
		{
			print_ttf_error("TTF_OpenFont");
			return set_error_state(tErrors::FailedToOpenFontFile);
		}
		m_fontOpen = true;
		m_fontFromMemory = true;
		return set_error_state(tErrors::NoError);
	}

	int32_t TTFRenderer::setFontSize(int32_t fontSize)
	{
		if (!TTFRenderer::m_initialized)
			return set_error_state(tErrors::Uninitialized);
		if (!m_fontOpen)
			return set_error_state(tErrors::NoFont);
		if (m_font == nullptr)
			return set_error_state(tErrors::NullFont);
		if (fontSize == m_fontSize || fontSize <= 0) return set_error_state(tErrors::NoError);
		TTF_SetFontSize(m_font, fontSize);
		return set_error_state(tErrors::NoError);
	}

	void TTFRenderer::print_ttf_error(const ostd::String& funcName)
	{
		m_out.fg(ostd::ConsoleColors::Red).p(funcName).p("(...) failed: ").p(ostd::String(SDL_GetError())).reset().nl();
	}
}
