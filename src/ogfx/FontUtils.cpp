#include "FontUtils.hpp"

namespace ogfx
{
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
		SDL_Surface* surf = TTF_RenderText_Blended(m_font, message.c_str(), { color.r, color.g, color.b, color.a });
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
		SDL_FreeSurface(surf);

		int32_t w = 0, h = 0;
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);

		SDL_Rect dst;
		dst.x = x;
		dst.y = y;
		dst.w = w;
		dst.h = h;
		SDL_RenderCopy(m_renderer, texture, NULL, &dst);
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
		SDL_Surface* surf = TTF_RenderText_Blended(m_font, message.c_str(), { color.r, color.g, color.b, color.a });
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
		SDL_FreeSurface(surf);

		int32_t w = 0, h = 0;
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);

		SDL_Rect dst;
		dst.x = center_x - (w / 2);
		dst.y = center_y - (h / 2);
		dst.w = w;
		dst.h = h;
		SDL_RenderCopy(m_renderer, texture, NULL, &dst);
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
		SDL_Surface* surf = TTF_RenderText_Blended(m_font, message.c_str(), { 0, 0, 0, 255 });
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
		SDL_FreeSurface(surf);

		int32_t w = 0, h = 0;
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);
		SDL_DestroyTexture(texture);
		return { w, h };
	}

	int32_t TTFRenderer::init(SDL_Renderer* renderer)
	{
		if (TTFRenderer::m_initialized) return set_error_state(tErrors::NoError);
		if (renderer == nullptr) return set_error_state(tErrors::NullRenderer);
		if (TTF_Init() != 0)
		{
			print_ttf_error("TTF_Init");
			SDL_Quit();
			return set_error_state(tErrors::FailedToLoad);
		}
		m_renderer = renderer;
		TTFRenderer::m_initialized = true;
		return set_error_state(tErrors::NoError);
	}

	void TTFRenderer::closeFont(void)
	{
		if (!TTFRenderer::m_initialized || !m_fontOpen) return;
		if (m_font != nullptr)
			TTF_CloseFont(m_font);
		m_font = nullptr;
		m_fontOpen = false;
		m_fontSize = DefaultFontSize;
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
		m_out.fg(ostd::ConsoleColors::Red).p(funcName).p("(...) failed: ").p(ostd::String(TTF_GetError())).reset().nl();
	}
}