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
#include "../gui/Window.hpp"
#include "../resources/UbuntuMonoRegularTTF.hpp"

#define COLOR_CAST(ostd_color) std::bit_cast<SDL_FColor>(ostd_color.getNormalizedColor())
#define QUAD_INDICES_ARR { 0, 1, 2, 2, 3, 0 }

namespace ogfx
{
	// ===================================================== SIGNAL HANDLER =====================================================
	BasicRenderer2D::SignalHandler::SignalHandler(BasicRenderer2D& parent) : m_parent(parent)
	{
		enableSignals();
		connectSignal(ostd::BuiltinSignals::BeforeSDLShutdown);
		setTypeName("ostd::GraphicsWindowOutputHandler::SignalHandler");
		validate();
	}

	void BasicRenderer2D::SignalHandler::handleSignal(ostd::Signal& signal)
	{
		if (signal.ID == ostd::BuiltinSignals::BeforeSDLShutdown)
		{
			m_parent.closeFont();
		}
	}
	// ===================================================== SIGNAL HANDLER =====================================================





	// ===================================================== UTILS =====================================================
	BasicRenderer2D::~BasicRenderer2D(void)
	{
		closeFont();
	}

	int32_t BasicRenderer2D::init(WindowCore& window)
	{
		m_window = &window;
		if (m_initialized) return set_error_state(tErrors::NoError);
		init_arrays();
		m_initialized = true;
		return set_error_state(tErrors::NoError);
	}

	void BasicRenderer2D::flushBatch(void)
	{
		if (!m_initialized || m_vertexCount == 0)
    		return;
		SDL_RenderGeometry(m_window->getSDLRenderer(), m_texture, m_vertices.data(), m_vertexCount, m_indices.data(), m_indexCount);
		m_drawCallCount++;
		m_texture = nullptr;
		m_vertexCount = 0;
		m_indexCount = 0;
	}

	void BasicRenderer2D::endFrame(void)
	{
		flushBatch();
		m_drawCallCount = 0;
	}

	void BasicRenderer2D::pushClippingRect(const ostd::Rectangle& rect, bool additive)
	{
		if (!m_initialized) return;

	    ostd::Rectangle finalRect = rect;

	    if (additive && !m_clipStack.empty())
	        finalRect = m_clipStack.back().getIntersection(rect, false);

		if (!m_clipStack.empty() && m_clipStack.back() == finalRect)
			return;
		flushBatch();

	    m_clipStack.push_back(finalRect);

	    SDL_Rect r;
	    r.x = (int)std::round(finalRect.x);
	    r.y = (int)std::round(finalRect.y);
	    r.w = (int)std::round(finalRect.w);
	    r.h = (int)std::round(finalRect.h);

		SDL_SetRenderClipRect(m_window->getSDLRenderer(), &r);
	}

	void BasicRenderer2D::popClippingRect(void)
	{
		if (!m_initialized) return;
	    if (m_clipStack.empty()) return;

	    m_clipStack.pop_back();

		flushBatch();
	    if (m_clipStack.empty())
	    {
	        SDL_SetRenderClipRect(m_window->getSDLRenderer(), nullptr);
	        return;
	    }

	    const auto& rect = m_clipStack.back();

	    SDL_Rect r;
	    r.x = (int)std::round(rect.x);
	    r.y = (int)std::round(rect.y);
	    r.w = (int)std::round(rect.w);
	    r.h = (int)std::round(rect.h);

	    SDL_SetRenderClipRect(m_window->getSDLRenderer(), &r);
	}

	int32_t BasicRenderer2D::loadDefaultFont(int32_t fontSize)
	{
		return openFont(ubuntu_mono_regular_ttf_data, ubuntu_mono_regular_ttf_size, fontSize);
	}

	void BasicRenderer2D::closeFont(void)
	{
		if (!m_initialized || !m_fontOpen) return;
		if (m_font != nullptr)
			TTF_CloseFont(m_font);
		m_font = nullptr;
		m_fontOpen = false;
		m_fontSize = DefaultFontSize;
		m_fontFromMemory = false;
		set_error_state(tErrors::NoError);
	}

	int32_t BasicRenderer2D::openFont(const ostd::String& fontPath, int32_t fontSize)
	{
		if (!m_initialized) return set_error_state(tErrors::Uninitialized);
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

	int32_t BasicRenderer2D::openFont(const ostd::UByte resource_data[], uint32_t data_size, int32_t fontSize)
	{
		if (!m_initialized) return set_error_state(tErrors::Uninitialized);
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

	int32_t BasicRenderer2D::setFontSize(int32_t fontSize)
	{
		if (!m_initialized)
			return set_error_state(tErrors::Uninitialized);
		if (!m_fontOpen)
			return set_error_state(tErrors::NoFont);
		if (m_font == nullptr)
			return set_error_state(tErrors::NullFont);
		if (fontSize == m_fontSize || fontSize <= 0) return set_error_state(tErrors::NoError);
		TTF_SetFontSize(m_font, fontSize);
		return set_error_state(tErrors::NoError);
	}

	ostd::IPoint BasicRenderer2D::getStringDimensions(const ostd::String& message, int32_t fontSize)
	{
		if (!m_initialized)
		{
			set_error_state(tErrors::Uninitialized);
			return { 0, 0 };
		}
		if (!m_fontOpen)
		{
			set_error_state(tErrors::NoFont);
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
		SDL_Texture* texture = SDL_CreateTextureFromSurface(m_window->getSDLRenderer(), surf);
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
	// ===================================================== UTILS =====================================================





	// ===================================================== SPECIALIZED =====================================================
    void BasicRenderer2D::drawImage(const ogfx::Image& image, const ostd::Vec2& position, const ostd::Vec2& size, const ostd::Rectangle& srcRect)
    {
   		if (!m_initialized || !image.isLoaded())
            return;

        SDL_Texture* tex = image.getSDLTexture();
        if (!tex)
            return;

        ostd::Vec2 texSize = image.getSize();

        // 1. Resolve source rectangle
        float sx, sy, sw, sh;
        if (srcRect.w == 0 && srcRect.h == 0)
        {
            // Use entire texture
            sx = 0.0f;
            sy = 0.0f;
            sw = texSize.x;
            sh = texSize.y;
        }
        else
        {
            sx = srcRect.x;
            sy = srcRect.y;
            sw = srcRect.w;
            sh = srcRect.h;
        }

        // 2. Resolve destination rectangle
        float dx = position.x;
        float dy = position.y;
        float dw = (size.x == 0) ? sw : size.x;
        float dh = (size.y == 0) ? sh : size.y;
        float x1 = dx;
        float y1 = dy;
        float x2 = dx + dw;
        float y2 = dy + dh;

        // 3. Build quad vertices
        ostd::Vec2 verts[4] = {
            { x1, y1 },
            { x2, y1 },
            { x2, y2 },
            { x1, y2 }
        };

        // 4. Build UVs (normalized)
        ostd::Vec2 uvs[4] = {
            { sx / texSize.x,         sy / texSize.y },
            { (sx + sw) / texSize.x,  sy / texSize.y },
            { (sx + sw) / texSize.x, (sy + sh) / texSize.y },
            { sx / texSize.x,        (sy + sh) / texSize.y }
        };

        // 5. Push quad
        uint32_t inds[6] = QUAD_INDICES_ARR;
        push_polygon(verts, uvs, 4, inds, 6, ostd::Colors::White, tex);
    }

    void BasicRenderer2D::drawAnimation(const Animation& anim, const ostd::Vec2& position, const ostd::Vec2& size)
    {
    	if (!m_initialized) return;
     	if (!anim.hasImage()) return;
      	const Image& img = anim.getSpriteSheet();
       	if (!img.isLoaded() || !img.isValid()) return;
        drawImage(img, position, size, anim.getFrameRect());
    }

	void BasicRenderer2D::drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, int32_t fontSize)
	{
		if (!m_initialized) return;
	}

	void BasicRenderer2D::drawCenteredString(const ostd::String& str, const ostd::Vec2& center, const ostd::Color& color, int32_t fontSize)
	{
		if (!m_initialized) return;
	}
	// ===================================================== SPECIALIZED =====================================================





	// ===================================================== PRIMITIVES =====================================================
	void BasicRenderer2D::drawLine(const ostd::FLine& line, const ostd::Color& color, int32_t thickness, bool rounded)
	{
		if (!m_initialized || thickness <= 0) return;

		using namespace ostd;

		Vec2 p1 = line.p1.asVec2();
		Vec2 p2 = line.p2.asVec2();

		Vec2 dir = (p2 - p1).normalize();
		Vec2 perp = { -dir.y, dir.x }; // 90° rotation
		float half = thickness * 0.5f;
		Vec2 off = perp * half;

		std::array<ostd::Vec2, 4> verts = {{
    		p1 - off,
			p1 + off,
			p2 + off,
			p2 - off
		}};
		std::array<uint32_t, 6> inds = QUAD_INDICES_ARR;

		push_polygon(verts.data(), nullptr, 4, inds.data(), 6, color, nullptr);

	    if (!rounded || thickness < 4)
	        return;

		int segments = std::max(6, int(thickness * 0.75f));
	    generate_half_circle(p1, -dir, half, segments, color);
	    generate_half_circle(p2, dir, half, segments, color);
	}

	void BasicRenderer2D::drawRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness)
	{
		if (!m_initialized || thickness <= 0)
        	return;

	    float half = thickness * 0.5f;

	    // Inset rectangle so the outline stays inside the original bounds
	    float x1 = rect.x + half;
	    float y1 = rect.y + half;
	    float x2 = rect.x + rect.w - half;
	    float y2 = rect.y + rect.h - half;

	    // Top
	    drawLine({ {x1, y1}, {x2, y1} }, color, thickness, false);

	    // Right
	    drawLine({ {x2, y1 - half}, {x2, y2 + half} }, color, thickness, false);

	    // Bottom
	    drawLine({ {x2, y2}, {x1, y2} }, color, thickness, false);

	    // Left
	    drawLine({ {x1, y2 + half}, {x1, y1 - half} }, color, thickness, false);
	}

	void BasicRenderer2D::drawRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, float radius, int32_t thickness)
	{
		if (!m_initialized || thickness <= 0)
	        return;

	    // Clamp radius so it never exceeds half the smallest dimension
	    radius = std::max(0.0f, std::min(radius, std::min(rect.w, rect.h) * 0.5f));

	    float half = thickness * 0.5f;

	    // Inset rectangle so the border stays INSIDE the given bounds
	    float x1 = rect.x + half;
	    float y1 = rect.y + half;
	    float x2 = rect.x + rect.w - half;
	    float y2 = rect.y + rect.h - half;

	    // Corner centers
	    ostd::Vec2 TL { x1 + radius, y1 + radius };
	    ostd::Vec2 TR { x2 - radius, y1 + radius };
	    ostd::Vec2 BR { x2 - radius, y2 - radius };
	    ostd::Vec2 BL { x1 + radius, y2 - radius };

	    // Straight edges (shortened to meet the arcs cleanly)
	    drawLine({ {x1 + radius, y1}, {x2 - radius, y1} }, color, thickness, false); // top
	    drawLine({ {x2, y1 + radius}, {x2, y2 - radius} }, color, thickness, false); // right
	    drawLine({ {x2 - radius, y2}, {x1 + radius, y2} }, color, thickness, false); // bottom
	    drawLine({ {x1, y2 - radius}, {x1, y1 + radius} }, color, thickness, false); // left

	    // Number of segments for smooth arcs
	    int segments = std::max(6, int(radius * 0.75f));

	    // Quarter‑ellipse strokes (each is 90°)
	    generate_ellipse_stroke(TL, radius, radius, thickness, M_PI, M_PI + M_PI * 0.5f, color, segments); // TL
	    generate_ellipse_stroke(TR, radius, radius, thickness, 1.5f * M_PI, 2.0f * M_PI, color, segments); // TR
	    generate_ellipse_stroke(BR, radius, radius, thickness, 0.0f, M_PI * 0.5f, color, segments); // BR
	    generate_ellipse_stroke(BL, radius, radius, thickness, M_PI * 0.5f, M_PI, color, segments); // BL
	}

	void BasicRenderer2D::drawCircle(const ostd::Vec2& center, float radius, const ostd::Color& color, int32_t thickness)
	{
		if (!m_initialized || thickness <= 0)
	        return;
	    int segments = std::max(12, int(radius * 0.75f));
	    generate_ellipse_stroke(center, radius, radius, thickness, 0.0f, 2.0f * M_PI, color, segments);
	}

	void BasicRenderer2D::drawCircle(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness)
	{
	    if (!m_initialized || thickness <= 0)
	        return;

	    ostd::Vec2 center {
	        rect.x + rect.w * 0.5f,
	        rect.y + rect.h * 0.5f
	    };

	    float radius = std::min(rect.w, rect.h) * 0.5f;
	    drawCircle(center, radius, color, thickness);
	}

	void BasicRenderer2D::drawEllipse(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness)
	{
	  if (!m_initialized || thickness <= 0)
	      return;

		ostd::Vec2 center = { rect.x + rect.w*0.5f, rect.y + rect.h*0.5f };
	    float rx = rect.w * 0.5f;
	    float ry = rect.h * 0.5f;

	    int segments = std::max(12, int(std::max(rx, ry) * 0.75f));
	    generate_ellipse_stroke(center, rx, ry, thickness, 0.0f, 2.0f * M_PI, color, segments);
	}

	void BasicRenderer2D::fillRect(const ostd::Rectangle& rect, const ostd::Color& color)
	{
		if (!m_initialized)
	        return;

	    float x1 = rect.x;
	    float y1 = rect.y;
	    float x2 = rect.x + rect.w;
	    float y2 = rect.y + rect.h;

	    ostd::Vec2 verts[4] = {
	        { x1, y1 },
	        { x2, y1 },
	        { x2, y2 },
	        { x1, y2 }
	    };
	    uint32_t inds[6] = QUAD_INDICES_ARR;
	    push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
	}

	void BasicRenderer2D::fillRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, float radius)
	{
		if (!m_initialized)
	        return;

	    // Clamp radius
	    radius = std::max(0.0f, std::min(radius, std::min(rect.w, rect.h) * 0.5f));

	    float x1 = rect.x;
	    float y1 = rect.y;
	    float x2 = rect.x + rect.w;
	    float y2 = rect.y + rect.h;

	    float cx1 = x1 + radius;
	    float cy1 = y1 + radius;
	    float cx2 = x2 - radius;
	    float cy2 = y2 - radius;

	    // Number of segments for smooth corners
	    int segments = std::max(6, int(radius * 0.75f));

	    //
	    // 1. Fill the center rectangle
	    //
	    {
	        ostd::Vec2 verts[4] = {
	            { cx1, cy1 },
	            { cx2, cy1 },
	            { cx2, cy2 },
	            { cx1, cy2 }
	        };
	        uint32_t inds[6] = { 0,1,2, 2,3,0 };
	        push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
	    }

	    //
	    // 2. Fill the four side rectangles
	    //

	    // Top
	    if (radius > 0)
	    {
	        ostd::Vec2 verts[4] = {
	            { x1 + radius, y1 },
	            { x2 - radius, y1 },
	            { x2 - radius, y1 + radius },
	            { x1 + radius, y1 + radius }
	        };
	        uint32_t inds[6] = { 0,1,2, 2,3,0 };
	        push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
	    }

	    // Bottom
	    {
	        ostd::Vec2 verts[4] = {
	            { x1 + radius, y2 - radius },
	            { x2 - radius, y2 - radius },
	            { x2 - radius, y2 },
	            { x1 + radius, y2 }
	        };
	        uint32_t inds[6] = { 0,1,2, 2,3,0 };
	        push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
	    }

	    // Left
	    {
	        ostd::Vec2 verts[4] = {
	            { x1, y1 + radius },
	            { x1 + radius, y1 + radius },
	            { x1 + radius, y2 - radius },
	            { x1, y2 - radius }
	        };
	        uint32_t inds[6] = { 0,1,2, 2,3,0 };
	        push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
	    }

	    // Right
	    {
	        ostd::Vec2 verts[4] = {
	            { x2 - radius, y1 + radius },
	            { x2, y1 + radius },
	            { x2, y2 - radius },
	            { x2 - radius, y2 - radius }
	        };
	        uint32_t inds[6] = { 0,1,2, 2,3,0 };
	        push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
	    }

	    //
	    // 3. Fill the four rounded corners (quarter ellipses)
	    //

	    generate_filled_ellipse_stroke({cx1, cy1}, radius, radius, M_PI, color, segments);            // TL
	    generate_filled_ellipse_stroke({cx2, cy1}, radius, radius, 1.5f * M_PI, color, segments);     // TR
	    generate_filled_ellipse_stroke({cx2, cy2}, radius, radius, 0.0f, color, segments);            // BR
	    generate_filled_ellipse_stroke({cx1, cy2}, radius, radius, 0.5f * M_PI, color, segments);     // BL
	}

	void BasicRenderer2D::fillCircle(const ostd::Vec2& center, float radius, const ostd::Color& color)
	{
		if (!m_initialized)
	        return;

	    int segments = std::max(12, int(radius * 0.75f));
	    generate_filled_ellipse(center, radius, radius, color, segments);
	}

	void BasicRenderer2D::fillCircle(const ostd::Rectangle& rect, const ostd::Color& color)
	{
		if (!m_initialized)
		    return;

		ostd::Vec2 center {
		    rect.x + rect.w * 0.5f,
		    rect.y + rect.h * 0.5f
		};

		float radius = std::min(rect.w, rect.h) * 0.5f;
		fillCircle(center, radius, color);
	}

	void BasicRenderer2D::fillEllipse(const ostd::Rectangle& rect, const ostd::Color& color)
	{
		if (!m_initialized)
	        return;

	    ostd::Vec2 center {
	        rect.x + rect.w * 0.5f,
	        rect.y + rect.h * 0.5f
	    };

	    float radiusX = rect.w * 0.5f;
	    float radiusY = rect.h * 0.5f;

	    int segments = std::max(12, int(std::max(radiusX, radiusY) * 0.75f));
	    generate_filled_ellipse(center, radiusX, radiusY, color, segments);
	}

	void BasicRenderer2D::outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
	{
		if (!m_initialized) return;
		ostd::Rectangle offset = { 1, 1, -2, -2 };
		fillRect(rect + offset, fillColor);
		drawRect(rect , outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, float radius, int32_t outlineThickness)
	{
		if (!m_initialized) return;
		ostd::Rectangle offset = { 1, 1, -2, -2 };
		fillRoundRect(rect + offset, fillColor, radius);
		drawRoundRect(rect, outlineColor, radius, outlineThickness);
	}

	void BasicRenderer2D::outlinedCircle(const ostd::Vec2& center, float radius, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
	{
		if (!m_initialized) return;
		fillCircle(center, radius - 1, fillColor);
		drawCircle(center, radius, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedCircle(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
	{
		if (!m_initialized)
		    return;

		ostd::Vec2 center {
		    rect.x + rect.w * 0.5f,
		    rect.y + rect.h * 0.5f
		};

		float radius = std::min(rect.w, rect.h) * 0.5f;
		outlinedCircle(center, radius, fillColor, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedEllipse(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
	{
		if (!m_initialized) return;
		ostd::Rectangle offset = { 1, 1, -2, -2 };
		fillEllipse(rect + offset, fillColor);
		drawEllipse(rect, outlineColor, outlineThickness);
	}
	// ===================================================== PRIMITIVES =====================================================





	// ===================================================== PRIVATE HELPERS =====================================================
	void BasicRenderer2D::init_arrays(void)
	{
		for (auto& v : m_vertices)
		{
			v.color = COLOR_CAST(ostd::Colors::Transparent);
			v.position = { 0, 0 };
			v.tex_coord = { 0, 0 };
		}
		for (auto& i : m_indices)
			i = 0;
	}

	void BasicRenderer2D::generate_half_circle(const ostd::Vec2& center, const ostd::Vec2& dir, float radius, int segments, const ostd::Color& color)
	{
	    // Ensure we have room
	    if (m_vertexCount + segments + 2 >= MaxVertices ||  m_indexCount + segments * 3 >= MaxIndices)
	        flushBatch();

	    SDL_FColor col = COLOR_CAST(color);

	    // Index of the center vertex
	    int centerIndex = m_vertexCount;

	    // Push center vertex
	    m_vertices[m_vertexCount++] = {
	        { center.x, center.y },
	        col,
	        { 0.0f, 0.0f },
	    };

	    // Compute the base angle from the direction vector
	    float baseAngle = std::atan2(dir.y, dir.x);

	    // Half circle spans 180 degrees
	    float startAngle = baseAngle - M_PI * 0.5f;
	    float endAngle   = baseAngle + M_PI * 0.5f;

	    // Generate arc vertices
	    for (int i = 0; i <= segments; i++)
	    {
	        float t = float(i) / float(segments);
	        float angle = startAngle + t * (endAngle - startAngle);

	        float x = center.x + std::cos(angle) * radius;
	        float y = center.y + std::sin(angle) * radius;

	        m_vertices[m_vertexCount++] = {
	            { x, y },
	            col,
	            { 0.0f, 0.0f }
	        };
	    }

	    // Generate triangle fan indices
	    for (int i = 0; i < segments; i++)
	    {
	        m_indices[m_indexCount++] = centerIndex;
	        m_indices[m_indexCount++] = centerIndex + 1 + i;
	        m_indices[m_indexCount++] = centerIndex + 2 + i;
	    }
	}

	void BasicRenderer2D::generate_quarter_circle(const ostd::Vec2& center, float radius, float thickness, float startAngle, const ostd::Color& color, int segments)
	{
		float half = thickness * 0.5f;

		float outerR = radius + half;
		float innerR = radius - half;

		if (innerR < 0.0f)
		    innerR = 0.0f;

		// Ensure capacity
		if (m_vertexCount + (segments + 1) * 2 >= MaxVertices || m_indexCount + segments * 6 >= MaxIndices)
		    flushBatch();

		SDL_FColor col = COLOR_CAST(color);

		int base = m_vertexCount;

		float endAngle = startAngle + M_PI * 0.5f;

		// Generate vertices: outer arc + inner arc
		for (int i = 0; i <= segments; i++)
		{
		    float t = float(i) / float(segments);
		    float angle = startAngle + t * (endAngle - startAngle);

		    float cs = std::cos(angle);
		    float sn = std::sin(angle);

		    // Outer arc
		    m_vertices[m_vertexCount++] = {
		        { center.x + cs * outerR, center.y + sn * outerR },
		        col,
		        { 0, 0 }
		    };

		    // Inner arc
		    m_vertices[m_vertexCount++] = {
		        { center.x + cs * innerR, center.y + sn * innerR },
		        col,
		        { 0, 0 }
		    };
		}

		// Generate indices (triangle strip converted to triangles)
		for (int i = 0; i < segments; i++)
		{
		    int o0 = base + i * 2;
		    int i0 = o0 + 1;
		    int o1 = o0 + 2;
		    int i1 = o0 + 3;

		    // Triangle 1
		    m_indices[m_indexCount++] = o0;
		    m_indices[m_indexCount++] = o1;
		    m_indices[m_indexCount++] = i0;

		    // Triangle 2
		    m_indices[m_indexCount++] = o1;
		    m_indices[m_indexCount++] = i1;
		    m_indices[m_indexCount++] = i0;
		}
	}

	void BasicRenderer2D::generate_circle_stroke(const ostd::Vec2& center, float radius, float thickness, const ostd::Color& color, int segments)
	{
	    float half = thickness * 0.5f;

	    float outerR = radius + half;
	    float innerR = radius - half;
	    if (innerR < 0.0f)
	        innerR = 0.0f;

	    // Ensure capacity
	    if (m_vertexCount + (segments + 1) * 2 >= MaxVertices || m_indexCount + segments * 6 >= MaxIndices)
	        flushBatch();

	    SDL_FColor col = COLOR_CAST(color);

	    int base = m_vertexCount;

	    // Generate vertices: outer arc + inner arc
	    for (int i = 0; i <= segments; i++)
	    {
	        float t = float(i) / float(segments);
	        float angle = t * (2.0f * M_PI);

	        float cs = std::cos(angle);
	        float sn = std::sin(angle);

	        // Outer arc
	        m_vertices[m_vertexCount++] = {
	            { center.x + cs * outerR, center.y + sn * outerR },
	            col,
	            { 0, 0 }
	        };

	        // Inner arc
	        m_vertices[m_vertexCount++] = {
	            { center.x + cs * innerR, center.y + sn * innerR },
	            col,
	            { 0, 0 }
	        };
	    }

	    // Generate indices (triangle strip → triangles)
	    for (int i = 0; i < segments; i++)
	    {
	        int o0 = base + i * 2;
	        int i0 = o0 + 1;
	        int o1 = o0 + 2;
	        int i1 = o0 + 3;

	        // Triangle 1
	        m_indices[m_indexCount++] = o0;
	        m_indices[m_indexCount++] = o1;
	        m_indices[m_indexCount++] = i0;

	        // Triangle 2
	        m_indices[m_indexCount++] = o1;
	        m_indices[m_indexCount++] = i1;
	        m_indices[m_indexCount++] = i0;
	    }
	}

	void BasicRenderer2D::generate_ellipse_stroke(const ostd::Vec2& center, float radiusX, float radiusY, float thickness, float startAngle, float endAngle, const ostd::Color& color, int segments)
	{
	    float half = thickness * 0.5f;

	    float outerRX = radiusX + half;
	    float outerRY = radiusY + half;
	    float innerRX = std::max(0.0f, radiusX - half);
	    float innerRY = std::max(0.0f, radiusY - half);

	    // Ensure capacity
	    if (m_vertexCount + (segments + 1) * 2 >= MaxVertices ||
	        m_indexCount + segments * 6 >= MaxIndices)
	    {
	        flushBatch();
	    }

	    SDL_FColor col = COLOR_CAST(color);

	    int base = m_vertexCount;

	    float angleRange = endAngle - startAngle;

	    // Generate vertices: outer arc + inner arc
	    for (int i = 0; i <= segments; i++)
	    {
	        float t = float(i) / float(segments);
	        float angle = startAngle + t * angleRange;

	        float cs = std::cos(angle);
	        float sn = std::sin(angle);

	        // Outer arc
	        m_vertices[m_vertexCount++] = {
	            { center.x + cs * outerRX, center.y + sn * outerRY },
	            col,
	            { 0, 0 }
	        };

	        // Inner arc
	        m_vertices[m_vertexCount++] = {
	            { center.x + cs * innerRX, center.y + sn * innerRY },
	            col,
	            { 0, 0 }
	        };
	    }

	    // Generate indices (triangle strip → triangles)
	    for (int i = 0; i < segments; i++)
	    {
	        int o0 = base + i * 2;
	        int i0 = o0 + 1;
	        int o1 = o0 + 2;
	        int i1 = o0 + 3;

	        // Triangle 1
	        m_indices[m_indexCount++] = o0;
	        m_indices[m_indexCount++] = o1;
	        m_indices[m_indexCount++] = i0;

	        // Triangle 2
	        m_indices[m_indexCount++] = o1;
	        m_indices[m_indexCount++] = i1;
	        m_indices[m_indexCount++] = i0;
	    }
	}

	void BasicRenderer2D::generate_filled_ellipse_stroke(const ostd::Vec2& center, float radiusX, float radiusY, float startAngle, const ostd::Color& color, int segments)
	{
	    // Ensure capacity
	    if (m_vertexCount + segments + 2 >= MaxVertices || m_indexCount + segments * 3 >= MaxIndices)
	        flushBatch();

	    SDL_FColor col = COLOR_CAST(color);

	    int base = m_vertexCount;

	    // Center vertex
	    m_vertices[m_vertexCount++] = {
	        { center.x, center.y },
	        col,
	        { 0, 0 }
	    };

	    float endAngle = startAngle + M_PI * 0.5f;

	    for (int i = 0; i <= segments; i++)
	    {
	        float t = float(i) / float(segments);
	        float angle = startAngle + t * (endAngle - startAngle);

	        float x = center.x + std::cos(angle) * radiusX;
	        float y = center.y + std::sin(angle) * radiusY;

	        m_vertices[m_vertexCount++] = {
	            { x, y },
	            col,
	            { 0, 0 }
	        };
	    }

	    // Triangle fan indices
	    for (int i = 0; i < segments; i++)
	    {
	        m_indices[m_indexCount++] = base;
	        m_indices[m_indexCount++] = base + 1 + i;
	        m_indices[m_indexCount++] = base + 2 + i;
	    }
	}

	void BasicRenderer2D::generate_filled_ellipse(const ostd::Vec2& center, float radiusX, float radiusY, const ostd::Color& color, int segments)
	{
	    // Ensure capacity
	    if (m_vertexCount + segments + 2 >= MaxVertices || m_indexCount + segments * 3 >= MaxIndices)
	        flushBatch();

	    SDL_FColor col = COLOR_CAST(color);

	    int base = m_vertexCount;

	    // Center vertex
	    m_vertices[m_vertexCount++] = {
	        { center.x, center.y },
	        col,
	        { 0, 0 }
	    };

	    // Outer ring vertices
	    for (int i = 0; i <= segments; i++)
	    {
	        float t = float(i) / float(segments);
	        float angle = t * (2.0f * M_PI);

	        float x = center.x + std::cos(angle) * radiusX;
	        float y = center.y + std::sin(angle) * radiusY;

	        m_vertices[m_vertexCount++] = {
	            { x, y },
	            col,
	            { 0, 0 }
	        };
	    }

	    // Triangle fan indices
	    for (int i = 0; i < segments; i++)
	    {
	        m_indices[m_indexCount++] = base;
	        m_indices[m_indexCount++] = base + 1 + i;
	        m_indices[m_indexCount++] = base + 2 + i;
	    }
	}

	void BasicRenderer2D::push_polygon(const ostd::Vec2* verts, const ostd::Vec2* texCoords, uint32_t vertCount, const uint32_t* inds, uint32_t indexCount, const ostd::Color& color, SDL_Texture* texture)
	{
	    if (!m_initialized || vertCount <= 0 || indexCount <= 0)
	        return;

		if (vertCount > MaxVertices || indexCount > MaxIndices)
		{
			OX_ERROR("Maximum number of vertices exceeded in single draw call.");
			return;
		}
		if (texture != m_texture)
	        flushBatch();
	    m_texture = texture;
	    if (m_vertexCount + vertCount >= MaxVertices || m_indexCount + indexCount >= MaxIndices)
	        flushBatch();

	    SDL_FColor col = COLOR_CAST(color);

	    int base = m_vertexCount;
		bool hasTexCoords = texCoords != nullptr;
	    for (int i = 0; i < vertCount; i++)
	    {
			ostd::Vec2 tc { 0.0f, 0.0f };
			if (hasTexCoords)
				tc = texCoords[i];
	        m_vertices[m_vertexCount++] = {
	            { verts[i].x, verts[i].y },
	            col,
	            { tc.x, tc.y },
	        };
	    }
	    for (int i = 0; i < indexCount; i++)
	        m_indices[m_indexCount++] = base + inds[i];
	}

	void BasicRenderer2D::print_ttf_error(const ostd::String& funcName)
	{
		m_out.fg(ostd::ConsoleColors::Red).p(funcName).p("(...) failed: ").p(ostd::String(SDL_GetError())).reset().nl();
	}

	void BasicRenderer2D::renderText(const ostd::String& message, int32_t x, int32_t y, const ostd::Color& color, int32_t fontSize)
	{
		if (!m_initialized)
		{
			set_error_state(tErrors::Uninitialized);
			return;
		}
		if (!m_fontOpen)
		{
			set_error_state(tErrors::NoFont);
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
		auto renderer = m_window->getSDLRenderer();
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
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
		SDL_RenderTexture(renderer, texture, NULL, &dst);
		SDL_DestroyTexture(texture);
	}

	void BasicRenderer2D::renderCenteredText(const ostd::String& message, int32_t center_x, int32_t center_y, const ostd::Color& color, int32_t fontSize)
	{
		if (!m_initialized)
		{
			set_error_state(tErrors::Uninitialized);
			return;
		}
		if (!m_fontOpen)
		{
			set_error_state(tErrors::NoFont);
			return;
		}
		if (m_font == nullptr)
		{
			set_error_state(tErrors::NullFont);
			return;
		}
		setFontSize(fontSize);
		auto renderer = m_window->getSDLRenderer();
		SDL_Surface* surf = TTF_RenderText_Blended(m_font, message.c_str(), message.len(), { color.r, color.g, color.b, color.a });
		if (surf == nullptr)
		{
			set_error_state(tErrors::TTFRenderTextBlendedFail);
			print_ttf_error("TTF_RenderText_Blended");
			return;
		}
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
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
		SDL_RenderTexture(renderer, texture, NULL, &dst);
		SDL_DestroyTexture(texture);
	}
	// ===================================================== PRIVATE HELPERS =====================================================
}
