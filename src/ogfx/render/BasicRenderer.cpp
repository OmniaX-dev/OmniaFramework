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

#include "BasicRenderer.hpp"
#include "../gui/WindowBase.hpp"

namespace ogfx
{
	void BasicRenderer2D::init(WindowBase& window)
	{
		m_window = &window;
		m_ttfr.init(window.getSDLRenderer());
		m_initialized = true;
	}

	void BasicRenderer2D::setFont(const ostd::String& fontFilePath)
	{
		if (!m_initialized) return;
		m_ttfr.openFont(fontFilePath, m_ttfr.getFontSize());
	}

	void BasicRenderer2D::setFontSize(int32_t fontSize)
	{
		if (!m_initialized) return;
		m_ttfr.setFontSize(fontSize);
	}

    void BasicRenderer2D::drawImage(const ogfx::Image& image, const ostd::Vec2& position, const ostd::Rectangle& rect)
    {
        if (!m_initialized) return;
        if (!image.isLoaded()) return;
        SDL_Rect texr;
        texr.x = position.x;
        texr.y = position.y;
        texr.w = image.getSize().x;
        texr.h = image.getSize().y;
        SDL_Rect srcRect;
        srcRect.x = rect.x;
        srcRect.y = rect.y;
        srcRect.w = rect.w;
        srcRect.h = rect.w;
        if (srcRect.x == 0 && srcRect.y == 0 && srcRect.w == 0 && srcRect.h == 0)
        	SDL_RenderCopy(m_window->getSDLRenderer(), image.getSDLTexture(), nullptr, &texr);
        else
        {
       		texr.w = srcRect.w;
        	texr.h = srcRect.h;
       		SDL_RenderCopy(m_window->getSDLRenderer(), image.getSDLTexture(), &srcRect, &texr);
        }
    }

    void BasicRenderer2D::drawAnimation(const Animation& anim, const ostd::Vec2& position)
    {
    	if (!m_initialized) return;
     	if (!anim.hasImage()) return;
      	const auto& img = anim.getSpriteSheet();
       	if (!img.isLoaded() || !img.isValid()) return;
        drawImage(img, position, anim.getFrameRect());
    }

	void BasicRenderer2D::drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, int32_t fontSize)
	{
		if (!m_initialized) return;
		if (!m_ttfr.isValid()) return;
		m_ttfr.renderText(str, (int32_t)std::round(position.x), (int32_t)std::round(position.y), color, fontSize);
	}

	void BasicRenderer2D::drawCenteredString(const ostd::String& str, const ostd::Vec2& center, const ostd::Color& color, int32_t fontSize)
	{
		if (!m_initialized) return;
		if (!m_ttfr.isValid()) return;
		m_ttfr.renderCenteredText(str, (int32_t)std::round(center.x), (int32_t)std::round(center.y), color, fontSize);
	}

	void BasicRenderer2D::drawLine(const ostd::FLine& line, const ostd::Color& color, int32_t thickness)
	{
		if (!m_initialized) return;
		thickLineRGBA(m_window->getSDLRenderer(), (int32_t)std::round(line.p1.x), (int32_t)std::round(line.p1.y), (int32_t)std::round(line.p2.x), (int32_t)std::round(line.p2.y), thickness, color.r, color.g, color.b, color.a);
	}

	void BasicRenderer2D::drawRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness)
	{
		if (!m_initialized) return;
		float x1 = rect.x;
		float y1 = rect.y;
		float x2 = rect.x + rect.w;
		float y2 = rect.y + rect.h;
		if (thickness == 1)
			rectangleRGBA(m_window->getSDLRenderer(), (int32_t)std::round(x1), (int32_t)std::round(y1), (int32_t)std::round(x2), (int32_t)std::round(y2), color.r, color.g, color.b, color.a);
		else if (thickness > 1)
		{
			for (int32_t i = 0; i < thickness; i++)
				rectangleRGBA(m_window->getSDLRenderer(), (int32_t)std::round(x1 + i), (int32_t)std::round(y1 + i), (int32_t)std::round(x2 - i), (int32_t)std::round(y2 - i), color.r, color.g, color.b, color.a);
		}
	}

	void BasicRenderer2D::drawRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t radius, int32_t thickness)
	{
		if (!m_initialized) return;
		float x1 = rect.x;
		float y1 = rect.y;
		float x2 = rect.x + rect.w;
		float y2 = rect.y + rect.h;
		if (thickness == 1)
			roundedRectangleRGBA(m_window->getSDLRenderer(), (int32_t)std::round(x1), (int32_t)std::round(y1), (int32_t)std::round(x2), (int32_t)std::round(y2), radius, color.r, color.g, color.b, color.a);
		else if (thickness > 1)
		{
			for (int32_t i = 0; i < thickness; i++)
				roundedRectangleRGBA(m_window->getSDLRenderer(), (int32_t)std::round(x1 + i), (int32_t)std::round(y1 + i), (int32_t)std::round(x2 - i), (int32_t)std::round(y2 - i), radius, color.r, color.g, color.b, color.a);
		}
	}

	void BasicRenderer2D::drawCircle(const ostd::Vec2& center, int32_t radius, const ostd::Color& color, int32_t thickness)
	{
		if (!m_initialized) return;
		if (thickness == 1)
			circleRGBA(m_window->getSDLRenderer(), (int32_t)std::round(center.x), (int32_t)std::round(center.y), radius, color.r, color.g, color.b, color.a);
		else if (thickness > 1)
		{
			for (int32_t i = 0; i < thickness; i++)
				circleRGBA(m_window->getSDLRenderer(), (int32_t)std::round(center.x), (int32_t)std::round(center.y), radius - i, color.r, color.g, color.b, color.a);
		}
	}

	void BasicRenderer2D::fillRect(const ostd::Rectangle& rect, const ostd::Color& color)
	{
		if (!m_initialized) return;
		float x1 = rect.x;
		float y1 = rect.y;
		float x2 = rect.x + rect.w;
		float y2 = rect.y + rect.h;
		boxRGBA(m_window->getSDLRenderer(), (int32_t)std::round(x1), (int32_t)std::round(y1), (int32_t)std::round(x2), (int32_t)std::round(y2), color.r, color.g, color.b, color.a);
	}

	void BasicRenderer2D::fillRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t radius)
	{
		if (!m_initialized) return;
		float x1 = rect.x;
		float y1 = rect.y;
		float x2 = rect.x + rect.w;
		float y2 = rect.y + rect.h;
		roundedBoxRGBA(m_window->getSDLRenderer(), (int32_t)std::round(x1), (int32_t)std::round(y1), (int32_t)std::round(x2), (int32_t)std::round(y2), radius, color.r, color.g, color.b, color.a);
	}

	void BasicRenderer2D::fillCircle(const ostd::Vec2& center, int32_t radius, const ostd::Color& color)
	{
		if (!m_initialized) return;
		filledCircleRGBA(m_window->getSDLRenderer(), (int32_t)std::round(center.x), (int32_t)std::round(center.y), radius, color.r, color.g, color.b, color.a);
	}

	void BasicRenderer2D::outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
	{
		if (!m_initialized) return;
		fillRect(rect, fillColor);
		drawRect(rect, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t radius, int32_t outlineThickness)
	{
		if (!m_initialized) return;
		fillRoundRect(rect, fillColor, radius);
		drawRoundRect(rect, outlineColor, radius, outlineThickness);
	}

	void BasicRenderer2D::outlinedCircle(const ostd::Vec2& center, int32_t radius, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
	{
		if (!m_initialized) return;
		fillCircle(center, radius, fillColor);
		drawCircle(center, radius, outlineColor, outlineThickness);
	}

}
