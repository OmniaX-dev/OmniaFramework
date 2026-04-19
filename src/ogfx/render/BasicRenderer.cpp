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
#include "../gui/Window.hpp"

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

	i32 BasicRenderer2D::init(WindowCore& window)
	{
		m_window = &window;
		if (m_initialized) return set_error_state(tErrors::NoError);
		SDL_SetRenderDrawBlendMode(m_window->getSDLRenderer(), SDL_BLENDMODE_BLEND);
		init_arrays();
		m_initialized = true;
		loadDefaultFont(20);
		m_initialized = false;
		m_fontGlyphAtlas.init(*this);
		m_initialized = true;
		return set_error_state(tErrors::NoError);
	}

	SDL_Renderer* BasicRenderer2D::getSDLRenderer(void) const
	{
		return m_window != nullptr ? m_window->getSDLRenderer() : nullptr;
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

	void BasicRenderer2D::pushClippingRect(const Rectangle& rect, bool additive)
	{
		if (!m_initialized) return;

		Rectangle finalRect = rect;

		if (additive && !m_clipStack.empty())
			finalRect = m_clipStack.back().getIntersection(rect, false);

		if (!m_clipStack.empty() && m_clipStack.back() == finalRect)
		{
			m_clipStack.push_back(finalRect);
			return;
		}
		flushBatch();

		m_clipStack.push_back(finalRect);

		SDL_Rect r;
		r.x = (i32)std::round(finalRect.x);
		r.y = (i32)std::round(finalRect.y);
		r.w = (i32)std::round(finalRect.w);
		r.h = (i32)std::round(finalRect.h);

		SDL_SetRenderClipRect(m_window->getSDLRenderer(), &r);
	}

	void BasicRenderer2D::popClippingRect(void)
	{
		if (!m_initialized) return;
		if (m_clipStack.empty()) return;

		Rectangle popped = m_clipStack.back();
		m_clipStack.pop_back();

		if (m_clipStack.empty())
		{
			flushBatch();
			SDL_SetRenderClipRect(m_window->getSDLRenderer(), nullptr);
			return;
		}

		const auto& prev = m_clipStack.back();
		if (prev == popped)
			return;

		flushBatch();
		SDL_Rect r;
		r.x = (i32)std::round(prev.x);
		r.y = (i32)std::round(prev.y);
		r.w = (i32)std::round(prev.w);
		r.h = (i32)std::round(prev.h);

		SDL_SetRenderClipRect(m_window->getSDLRenderer(), &r);
	}

	i32 BasicRenderer2D::loadDefaultFont(i32 fontSize)
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

	i32 BasicRenderer2D::openFont(const String& fontPath, i32 fontSize)
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

	i32 BasicRenderer2D::openFont(const ostd::UByte resource_data[], u32 data_size, i32 fontSize)
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

	i32 BasicRenderer2D::setFontSize(i32 fontSize)
	{
		if (!m_initialized)
			return set_error_state(tErrors::Uninitialized);
		if (!m_fontOpen)
			return set_error_state(tErrors::NoFont);
		if (m_font == nullptr)
			return set_error_state(tErrors::NullFont);
		if (fontSize == m_fontSize || fontSize <= 0) return set_error_state(tErrors::NoError);
		TTF_SetFontSize(m_font, fontSize);
		m_fontSize = fontSize;
		return set_error_state(tErrors::NoError);
	}

	Vec2 BasicRenderer2D::getStringDimensions(const String& message, i32 fontSize, TTF_Font* font)
	{
		if (!isValid()) return { 0, 0 };
		if (fontSize <= 0) fontSize = m_fontSize;
		if (!font) font = m_font;
		i32 oldFontSize = getFontSize();
		setFontSize(fontSize);

		auto glyphs = m_fontGlyphAtlas.processString(message, font, fontSize);
		if (glyphs.empty()) return { 0, 0 };

		f32 totalWidth = 0;
		for (size_t i = 0; i < glyphs.size(); i++)
		{
			totalWidth += glyphs[i]->advance;
			if (i > 0)
			{
				i32 kern = 0;
				TTF_GetGlyphKerning(font, glyphs[i - 1]->codepoint, glyphs[i]->codepoint, &kern);
				totalWidth += kern;
			}
		}
		setFontSize(oldFontSize);
		return { totalWidth, glyphs[0]->size.y };
	}
	// ===================================================== UTILS =====================================================





	// ===================================================== SPECIALIZED =====================================================
	void BasicRenderer2D::drawImage(const ogfx::Image& image, const Vec2& position, const Vec2& size, const Rectangle& srcRect)
	{
		if (!m_initialized || !image.isLoaded())
			return;

		SDL_Texture* tex = image.getSDLTexture();
		if (!tex)
			return;

		Vec2 texSize = image.getSize();

		// 1. Resolve source rectangle
		f32 sx, sy, sw, sh;
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
		f32 dx = position.x;
		f32 dy = position.y;
		f32 dw = (size.x == 0) ? sw : size.x;
		f32 dh = (size.y == 0) ? sh : size.y;
		f32 x1 = dx;
		f32 y1 = dy;
		f32 x2 = dx + dw;
		f32 y2 = dy + dh;

		// 3. Build quad vertices
		Vec2 verts[4] = {
			{ x1, y1 },
			{ x2, y1 },
			{ x2, y2 },
			{ x1, y2 }
		};

		// 4. Build UVs (normalized)
		Vec2 uvs[4] = {
			{ sx / texSize.x,         sy / texSize.y },
			{ (sx + sw) / texSize.x,  sy / texSize.y },
			{ (sx + sw) / texSize.x, (sy + sh) / texSize.y },
			{ sx / texSize.x,        (sy + sh) / texSize.y }
		};

		// 5. Push quad
		u32 inds[6] = QUAD_INDICES_ARR;
		push_polygon(verts, uvs, 4, inds, 6, Colors::White, tex);
	}

	void BasicRenderer2D::drawAnimation(const Animation& anim, const Vec2& position, const Vec2& size)
	{
		if (!m_initialized) return;
		if (!anim.hasImage()) return;
		const Image& img = anim.getSpriteSheet();
		if (!img.isLoaded() || !img.isValid()) return;
		drawImage(img, position, size, anim.getFrameRect());
	}

	void BasicRenderer2D::fillGradientRect(const Rectangle& rect, ColorGradient& gradient)
	{
		auto& colors = gradient.getColors();
		auto& weights = gradient.getWeights();
		f32 angle_radians = gradient.getAngleRad();
		if (!m_initialized || colors.size() < 2) return;
		if (weights.size() != colors.size() - 1) return; // N colors = N-1 weights

		// Normalize weights
		f32 total = 0;
		for (auto w : weights) total += w;
		for (auto& w : weights) w /= total;

		// Compute cumulative positions along gradient (0 to 1)
		stdvec<f32> stops(colors.size());
		stops[0] = 0.0f;
		for (size_t i = 0; i < weights.size(); i++)
			stops[i + 1] = stops[i] + weights[i];

		// Center of rect for rotation
		Vec2 center = { rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f };
		f32 cs = std::cos(angle_radians);
		f32 sn = std::sin(angle_radians);

		auto rotate = [&](Vec2 p) -> Vec2 {
			p -= center;
			return { center.x + p.x * cs - p.y * sn,
					 center.y + p.x * sn + p.y * cs };
		};

		// For each color band, push a quad
		for (size_t i = 0; i < colors.size() - 1; i++)
		{
			f32 y0 = rect.y + rect.h * stops[i];
			f32 y1 = rect.y + rect.h * stops[i + 1];

			Vec2 tl = rotate({ rect.x,          y0 });
			Vec2 tr = rotate({ rect.x + rect.w,  y0 });
			Vec2 br = rotate({ rect.x + rect.w,  y1 });
			Vec2 bl = rotate({ rect.x,           y1 });

			if (m_vertexCount + 4 >= MaxVertices || m_indexCount + 6 >= MaxIndices)
				flushBatch();
			if (m_texture != nullptr)
				flushBatch();
			m_texture = nullptr;

			SDL_FColor c0 = COLOR_CAST(colors[i]);
			SDL_FColor c1 = COLOR_CAST(colors[i + 1]);

			i32 base = m_vertexCount;
			m_vertices[m_vertexCount++] = { { tl.x, tl.y }, c0, { 0, 0 } };
			m_vertices[m_vertexCount++] = { { tr.x, tr.y }, c0, { 0, 0 } };
			m_vertices[m_vertexCount++] = { { br.x, br.y }, c1, { 0, 0 } };
			m_vertices[m_vertexCount++] = { { bl.x, bl.y }, c1, { 0, 0 } };

			u32 inds[6] = QUAD_INDICES_ARR;
			for (i32 j = 0; j < 6; j++)
				m_indices[m_indexCount++] = base + inds[j];
		}
	}

	void BasicRenderer2D::drawString(const String& str, const Vec2& position, const Color& color, i32 fontSize, f32 scale)
	{
		if (!isValid()) return;
		if (fontSize <= 0)
			fontSize = m_fontSize;
		i32 oldFontSize = m_fontSize;
		setFontSize(fontSize);

		auto glyphs = m_fontGlyphAtlas.processString(str, m_font, fontSize);
		if (glyphs.empty()) return;

		f32 x = position.x;
		f32 y = position.y;


		for (size_t i = 0; i < glyphs.size(); i++)
		{
			auto& g = glyphs[i];

			if (i > 0)
			{
				int kern = 0;
				TTF_GetGlyphKerning(m_font, glyphs[i - 1]->codepoint, g->codepoint, &kern);
				x += (kern * scale);
			}

			Vec2 verts[4] = {
				{ x,                        y },
				{ x + g->size.x * scale,     y },
				{ x + g->size.x * scale,     y + g->size.y * scale },
				{ x,                        y + g->size.y * scale }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, g->uvs, 4, inds, 6, color, g->atlas);

			x += (g->advance * scale);
		}
		setFontSize(oldFontSize);
	}

	void BasicRenderer2D::drawCenteredString(const String& str, const Vec2& center, const Color& color, i32 fontSize, f32 scale)
	{
		auto dims = getStringDimensions(str, fontSize);
		drawString(str, { center.x - (dims.x * scale) * 0.5f, center.y - (dims.y * scale) * 0.5f }, color, fontSize, scale);
	}

	void BasicRenderer2D::drawCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize, f32 scale)
	{
		drawCenteredString(str, Vec2 { bounds.x + bounds.w * 0.5f, bounds.y + bounds.h * 0.5f }, color, fontSize, scale);
	}
	// ===================================================== SPECIALIZED =====================================================





	// ===================================================== PRIMITIVES =====================================================
	void BasicRenderer2D::drawLine(const FLine& line, const Color& color, i32 thickness, bool rounded)
	{
		if (!m_initialized || thickness <= 0) return;

		using namespace ostd;

		Vec2 p1 = line.p1.asVec2();
		Vec2 p2 = line.p2.asVec2();

		Vec2 dir = (p2 - p1).normalize();
		Vec2 perp = { -dir.y, dir.x }; // 90° rotation
		f32 half = thickness * 0.5f;
		Vec2 off = perp * half;

		std::array<Vec2, 4> verts = {{
			p1 - off,
			p1 + off,
			p2 + off,
			p2 - off
		}};
		std::array<u32, 6> inds = QUAD_INDICES_ARR;

		push_polygon(verts.data(), nullptr, 4, inds.data(), 6, color, nullptr);

		if (!rounded || thickness < 4)
			return;

		i32 segments = std::max(16, i32(thickness * 1.5f));
		generate_half_circle(p1, -dir, half, segments, color);
		generate_half_circle(p2, dir, half, segments, color);
	}

	void BasicRenderer2D::drawRect(const Rectangle& rect, const Color& color, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
			return;

		f32 half = thickness * 0.5f;

		// Inset rectangle so the outline stays inside the original bounds
		f32 x1 = rect.x + half;
		f32 y1 = rect.y + half;
		f32 x2 = rect.x + rect.w - half;
		f32 y2 = rect.y + rect.h - half;

		// Top
		drawLine({ {x1, y1}, {x2, y1} }, color, thickness, false);

		// Right
		drawLine({ {x2, y1 - half}, {x2, y2 + half} }, color, thickness, false);

		// Bottom
		drawLine({ {x2, y2}, {x1, y2} }, color, thickness, false);

		// Left
		drawLine({ {x1, y2 + half}, {x1, y1 - half} }, color, thickness, false);
	}

	void BasicRenderer2D::drawRect(const Vec2& center, const Vec2& size, const Color& color, i32 thickness)
	{
		drawRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, thickness);
	}

	void BasicRenderer2D::drawRoundRect(const Rectangle& rect, const Color& color, f32 radius, i32 thickness)
	{
		drawRoundRect(rect, color, { radius, radius, radius, radius }, thickness);
	}

	void BasicRenderer2D::drawRoundRect(const Vec2& center, const Vec2& size, const Color& color, f32 radius, i32 thickness)
	{
		drawRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, { radius, radius, radius, radius }, thickness);
	}

	void BasicRenderer2D::drawRoundRect(const Vec2& center, const Vec2& size, const Color& color, const Rectangle& radii, i32 thickness)
	{
		drawRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, radii, thickness);
	}

	void BasicRenderer2D::drawRoundRect(const Rectangle& rect, const Color& color, const Rectangle& radii, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
			return;

		// radii: x=TL, y=TR, w=BR, h=BL
		f32 maxR = std::min(rect.w, rect.h) * 0.5f;
		f32 rTL = std::max(0.0f, std::min(radii.x, maxR));
		f32 rTR = std::max(0.0f, std::min(radii.y, maxR));
		f32 rBR = std::max(0.0f, std::min(radii.w, maxR));
		f32 rBL = std::max(0.0f, std::min(radii.h, maxR));

		f32 half = thickness * 0.5f;

		f32 x1 = rect.x + half;
		f32 y1 = rect.y + half;
		f32 x2 = rect.x + rect.w - half;
		f32 y2 = rect.y + rect.h - half;

		// Straight edges
		drawLine({ {x1 + rTL, y1}, {x2 - rTR, y1} }, color, thickness, false); // top
		drawLine({ {x2, y1 + rTR}, {x2, y2 - rBR} }, color, thickness, false); // right
		drawLine({ {x2 - rBR, y2}, {x1 + rBL, y2} }, color, thickness, false); // bottom
		drawLine({ {x1, y2 - rBL}, {x1, y1 + rTL} }, color, thickness, false); // left

		// Corner arcs
		auto segments = [](f32 r) -> i32 { return std::max(16, i32(r * 1.5f)); };

		if (rTL > 0) generate_ellipse_stroke({ x1 + rTL, y1 + rTL }, rTL, rTL, thickness, M_PI, M_PI * 1.5f, color, segments(rTL));
		if (rTR > 0) generate_ellipse_stroke({ x2 - rTR, y1 + rTR }, rTR, rTR, thickness, M_PI * 1.5f, M_PI * 2.0f, color, segments(rTR));
		if (rBR > 0) generate_ellipse_stroke({ x2 - rBR, y2 - rBR }, rBR, rBR, thickness, 0.0f, M_PI * 0.5f, color, segments(rBR));
		if (rBL > 0) generate_ellipse_stroke({ x1 + rBL, y2 - rBL }, rBL, rBL, thickness, M_PI * 0.5f, M_PI, color, segments(rBL));
	}

	void BasicRenderer2D::drawCircle(const Vec2& center, f32 radius, const Color& color, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
			return;
		i32 segments = std::max(16, i32(radius * 1.5f));
		generate_ellipse_stroke(center, radius, radius, thickness, 0.0f, 2.0f * M_PI, color, segments);
	}

	void BasicRenderer2D::drawCircle(const Rectangle& rect, const Color& color, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radius = std::min(rect.w, rect.h) * 0.5f;
		drawCircle(center, radius, color, thickness);
	}

	void BasicRenderer2D::drawEllipse(const Rectangle& rect, const Color& color, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
		  return;

		Vec2 center = { rect.x + rect.w*0.5f, rect.y + rect.h*0.5f };
		f32 rx = rect.w * 0.5f;
		f32 ry = rect.h * 0.5f;
		i32 segments = std::max(16, i32(std::max(rx, ry) * 1.5f));
		generate_ellipse_stroke(center, rx, ry, thickness, 0.0f, 2.0f * M_PI, color, segments);
	}

	void BasicRenderer2D::drawTriangle(const Triangle& tri, const Color& color, i32 thickness)
	{
		if (!m_initialized)
			return;
		drawTriangle(tri.vA, tri.vB, tri.vC, color, thickness);
	}

	void BasicRenderer2D::drawTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& color, i32 thickness)
	{
		if (!m_initialized)
			return;
		drawLine({ A, B }, color, thickness, false);
		drawLine({ B, C }, color, thickness, false);
		drawLine({ C, A }, color, thickness, false);
	}




	void BasicRenderer2D::fillRect(const Rectangle& rect, const Color& color)
	{
		if (!m_initialized)
			return;

		f32 x1 = rect.x;
		f32 y1 = rect.y;
		f32 x2 = rect.x + rect.w;
		f32 y2 = rect.y + rect.h;

		Vec2 verts[4] = {
			{ x1, y1 },
			{ x2, y1 },
			{ x2, y2 },
			{ x1, y2 }
		};
		u32 inds[6] = QUAD_INDICES_ARR;
		push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
	}

	void BasicRenderer2D::fillRect(const Vec2& center, const Vec2& size, const Color& color)
	{
		fillRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color);
	}

	void BasicRenderer2D::fillRoundRect(const Rectangle& rect, const Color& color, f32 radius)
	{
		fillRoundRect(rect, color, { radius, radius, radius, radius });
	}

	void BasicRenderer2D::fillRoundRect(const Vec2& center, const Vec2& size, const Color& color, f32 radius)
	{
		fillRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, { radius, radius, radius, radius });
	}

	void BasicRenderer2D::fillRoundRect(const Vec2& center, const Vec2& size, const Color& color, const Rectangle& radii)
	{
		fillRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, radii);
	}

	void BasicRenderer2D::fillRoundRect(const Rectangle& rect, const Color& color, const Rectangle& radii)
	{
		if (!m_initialized)
			return;

		f32 maxR = std::min(rect.w, rect.h) * 0.5f;
		f32 rTL = std::max(0.0f, std::min(radii.x, maxR));
		f32 rTR = std::max(0.0f, std::min(radii.y, maxR));
		f32 rBR = std::max(0.0f, std::min(radii.w, maxR));
		f32 rBL = std::max(0.0f, std::min(radii.h, maxR));

		f32 x1 = rect.x;
		f32 y1 = rect.y;
		f32 x2 = rect.x + rect.w;
		f32 y2 = rect.y + rect.h;

		// The largest corner radius on each side determines the inset for that side
		f32 topInset    = std::max(rTL, rTR);
		f32 bottomInset = std::max(rBL, rBR);
		f32 leftInset   = std::max(rTL, rBL);
		f32 rightInset  = std::max(rTR, rBR);

		// Center rectangle
		{
			Vec2 verts[4] = {
				{ x1 + leftInset, y1 + topInset },
				{ x2 - rightInset, y1 + topInset },
				{ x2 - rightInset, y2 - bottomInset },
				{ x1 + leftInset, y2 - bottomInset }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Top strip
		{
			Vec2 verts[4] = {
				{ x1 + rTL, y1 },
				{ x2 - rTR, y1 },
				{ x2 - rTR, y1 + topInset },
				{ x1 + rTL, y1 + topInset }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Bottom strip
		{
			Vec2 verts[4] = {
				{ x1 + rBL, y2 - bottomInset },
				{ x2 - rBR, y2 - bottomInset },
				{ x2 - rBR, y2 },
				{ x1 + rBL, y2 }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Left strip
		{
			Vec2 verts[4] = {
				{ x1, y1 + rTL },
				{ x1 + leftInset, y1 + rTL },
				{ x1 + leftInset, y2 - rBL },
				{ x1, y2 - rBL }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Right strip
		{
			Vec2 verts[4] = {
				{ x2 - rightInset, y1 + rTR },
				{ x2, y1 + rTR },
				{ x2, y2 - rBR },
				{ x2 - rightInset, y2 - rBR }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Fill gaps between corners and strips when radii differ on the same side
		// Top-left gap (if rTL < topInset, there's a rectangular gap below the TL corner)
		if (rTL < topInset)
		{
			Vec2 verts[4] = {
				{ x1, y1 + rTL },
				{ x1 + leftInset, y1 + rTL },
				{ x1 + leftInset, y1 + topInset },
				{ x1, y1 + topInset }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Top-right gap
		if (rTR < topInset)
		{
			Vec2 verts[4] = {
				{ x2 - rightInset, y1 + rTR },
				{ x2, y1 + rTR },
				{ x2, y1 + topInset },
				{ x2 - rightInset, y1 + topInset }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Bottom-left gap
		if (rBL < bottomInset)
		{
			Vec2 verts[4] = {
				{ x1, y2 - bottomInset },
				{ x1 + leftInset, y2 - bottomInset },
				{ x1 + leftInset, y2 - rBL },
				{ x1, y2 - rBL }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Bottom-right gap
		if (rBR < bottomInset)
		{
			Vec2 verts[4] = {
				{ x2 - rightInset, y2 - bottomInset },
				{ x2, y2 - bottomInset },
				{ x2, y2 - rBR },
				{ x2 - rightInset, y2 - rBR }
			};
			u32 inds[6] = QUAD_INDICES_ARR;
			push_polygon(verts, nullptr, 4, inds, 6, color, nullptr);
		}

		// Left-side gap (if rTL != rBL, leftInset is the max, smaller one needs a gap)
		if (rTL < leftInset && rTL < topInset)
		{
			// Already covered by TL gap above
		}
		if (rBL < leftInset && rBL < bottomInset)
		{
			// Already covered by BL gap above
		}

		// Corner fills
		auto segments = [](f32 r) -> i32 { return std::max(16, i32(r * 1.5f)); };

		if (rTL > 0) generate_filled_ellipse_stroke({ x1 + rTL, y1 + rTL }, rTL, rTL, M_PI, color, segments(rTL));
		if (rTR > 0) generate_filled_ellipse_stroke({ x2 - rTR, y1 + rTR }, rTR, rTR, M_PI * 1.5f, color, segments(rTR));
		if (rBR > 0) generate_filled_ellipse_stroke({ x2 - rBR, y2 - rBR }, rBR, rBR, 0.0f, color, segments(rBR));
		if (rBL > 0) generate_filled_ellipse_stroke({ x1 + rBL, y2 - rBL }, rBL, rBL, M_PI * 0.5f, color, segments(rBL));
	}

	void BasicRenderer2D::fillCircle(const Vec2& center, f32 radius, const Color& color)
	{
		if (!m_initialized)
			return;

		i32 segments = std::max(16, i32(radius * 1.5f));
		generate_filled_ellipse(center, radius, radius, color, segments);
	}

	void BasicRenderer2D::fillCircle(const Rectangle& rect, const Color& color)
	{
		if (!m_initialized)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radius = std::min(rect.w, rect.h) * 0.5f;
		fillCircle(center, radius, color);
	}

	void BasicRenderer2D::fillEllipse(const Rectangle& rect, const Color& color)
	{
		if (!m_initialized)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radiusX = rect.w * 0.5f;
		f32 radiusY = rect.h * 0.5f;

		i32 segments = std::max(16, i32(std::max(radiusX, radiusY) * 1.5f));
		generate_filled_ellipse(center, radiusX, radiusY, color, segments);
	}

	void BasicRenderer2D::fillTriangle(const Triangle& tri, const Color& color)
	{
		if (!m_initialized)
			return;
		fillTriangle(tri.vA, tri.vB, tri.vC, color);
	}

	void BasicRenderer2D::fillTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& color)
	{
		if (!m_initialized)
			return;
		Vec2 verts[3] = { A, B, C };
		Vec2 texCoords[3] = { {0, 0}, {0, 0}, {0, 0} };
		u32 inds[3] = { 0, 1, 2 };
		push_polygon(verts, texCoords, 3, inds, 3, color, nullptr);
	}




	void BasicRenderer2D::outlinedRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		Rectangle offset = { 1, 1, -2, -2 };
		fillRect(rect + offset, fillColor);
		drawRect(rect, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		outlinedRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, fillColor, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, f32 radius, i32 outlineThickness)
	{
		outlinedRoundRect(rect, fillColor, outlineColor, { radius, radius, radius, radius }, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, f32 radius, i32 outlineThickness)
	{
		outlinedRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, fillColor, outlineColor, { radius, radius, radius, radius }, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, const Rectangle& radii, i32 outlineThickness)
	{
		outlinedRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, fillColor, outlineColor, radii, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, const Rectangle& radii, i32 outlineThickness)
	{
		if (!m_initialized) return;
		Rectangle offset = { 1, 1, -2, -2 };
		fillRoundRect(rect + offset, fillColor, radii);
		drawRoundRect(rect, outlineColor, radii, outlineThickness);
	}

	void BasicRenderer2D::outlinedCircle(const Vec2& center, f32 radius, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		fillCircle(center, radius - 1, fillColor);
		drawCircle(center, radius, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedCircle(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radius = std::min(rect.w, rect.h) * 0.5f;
		outlinedCircle(center, radius, fillColor, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedEllipse(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		Rectangle offset = { 1, 1, -2, -2 };
		fillEllipse(rect + offset, fillColor);
		drawEllipse(rect, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedTriangle(const Triangle& tri, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		outlinedTriangle(tri.vA, tri.vB, tri.vC, fillColor, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		drawTriangle(A, B, C, outlineColor, outlineThickness);
		// Inset the fill triangle so it doesn't bleed past the outline
		f32 inset = cast<f32>(outlineThickness);
		Vec2 centroid = { (A.x + B.x + C.x) / 3.0f, (A.y + B.y + C.y) / 3.0f };

		auto shrink = [&](const Vec2& v) -> Vec2 {
			Vec2 dir = { centroid.x - v.x, centroid.y - v.y };
			f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len < 0.001f) return v;
			dir.x /= len;
			dir.y /= len;
			return { v.x + dir.x * inset, v.y + dir.y * inset };
		};

		fillTriangle(shrink(A), shrink(B), shrink(C), fillColor);
	}
	// ===================================================== PRIMITIVES =====================================================





	// ===================================================== PRIVATE HELPERS =====================================================
	void BasicRenderer2D::init_arrays(void)
	{
		for (auto& v : m_vertices)
		{
			v.color = COLOR_CAST(Colors::Transparent);
			v.position = { 0, 0 };
			v.tex_coord = { 0, 0 };
		}
		for (auto& i : m_indices)
			i = 0;
	}

	void BasicRenderer2D::generate_half_circle(const Vec2& center, const Vec2& dir, f32 radius, i32 segments, const Color& color)
	{
		// Ensure we have room
		if (m_vertexCount + segments + 2 >= MaxVertices ||  m_indexCount + segments * 3 >= MaxIndices)
			flushBatch();

		SDL_FColor col = COLOR_CAST(color);

		// Index of the center vertex
		i32 centerIndex = m_vertexCount;

		// Push center vertex
		m_vertices[m_vertexCount++] = {
			{ center.x, center.y },
			col,
			{ 0.0f, 0.0f },
		};

		// Compute the base angle from the direction vector
		f32 baseAngle = std::atan2(dir.y, dir.x);

		// Half circle spans 180 degrees
		f32 startAngle = baseAngle - M_PI * 0.5f;
		f32 endAngle   = baseAngle + M_PI * 0.5f;

		// Generate arc vertices
		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = f32(i) / f32(segments);
			f32 angle = startAngle + t * (endAngle - startAngle);

			f32 x = center.x + std::cos(angle) * radius;
			f32 y = center.y + std::sin(angle) * radius;

			m_vertices[m_vertexCount++] = {
				{ x, y },
				col,
				{ 0.0f, 0.0f }
			};
		}

		// Generate triangle fan indices
		for (i32 i = 0; i < segments; i++)
		{
			m_indices[m_indexCount++] = centerIndex;
			m_indices[m_indexCount++] = centerIndex + 1 + i;
			m_indices[m_indexCount++] = centerIndex + 2 + i;
		}
	}

	void BasicRenderer2D::generate_quarter_circle(const Vec2& center, f32 radius, f32 thickness, f32 startAngle, const Color& color, i32 segments)
	{
		f32 half = thickness * 0.5f;

		f32 outerR = radius + half;
		f32 innerR = radius - half;

		if (innerR < 0.0f)
			innerR = 0.0f;

		// Ensure capacity
		if (m_vertexCount + (segments + 1) * 2 >= MaxVertices || m_indexCount + segments * 6 >= MaxIndices)
			flushBatch();

		SDL_FColor col = COLOR_CAST(color);

		i32 base = m_vertexCount;

		f32 endAngle = startAngle + M_PI * 0.5f;

		// Generate vertices: outer arc + inner arc
		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = f32(i) / f32(segments);
			f32 angle = startAngle + t * (endAngle - startAngle);

			f32 cs = std::cos(angle);
			f32 sn = std::sin(angle);

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
		for (i32 i = 0; i < segments; i++)
		{
			i32 o0 = base + i * 2;
			i32 i0 = o0 + 1;
			i32 o1 = o0 + 2;
			i32 i1 = o0 + 3;

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

	void BasicRenderer2D::generate_circle_stroke(const Vec2& center, f32 radius, f32 thickness, const Color& color, i32 segments)
	{
		f32 half = thickness * 0.5f;

		f32 outerR = radius + half;
		f32 innerR = radius - half;
		if (innerR < 0.0f)
			innerR = 0.0f;

		// Ensure capacity
		if (m_vertexCount + (segments + 1) * 2 >= MaxVertices || m_indexCount + segments * 6 >= MaxIndices)
			flushBatch();

		SDL_FColor col = COLOR_CAST(color);

		i32 base = m_vertexCount;

		// Generate vertices: outer arc + inner arc
		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = f32(i) / f32(segments);
			f32 angle = t * (2.0f * M_PI);

			f32 cs = std::cos(angle);
			f32 sn = std::sin(angle);

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
		for (i32 i = 0; i < segments; i++)
		{
			i32 o0 = base + i * 2;
			i32 i0 = o0 + 1;
			i32 o1 = o0 + 2;
			i32 i1 = o0 + 3;

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

	void BasicRenderer2D::generate_ellipse_stroke(const Vec2& center, f32 radiusX, f32 radiusY, f32 thickness, f32 startAngle, f32 endAngle, const Color& color, i32 segments)
	{
		f32 half = thickness * 0.5f;

		f32 outerRX = radiusX + half;
		f32 outerRY = radiusY + half;
		f32 innerRX = std::max(0.0f, radiusX - half);
		f32 innerRY = std::max(0.0f, radiusY - half);

		// Ensure capacity
		if (m_vertexCount + (segments + 1) * 2 >= MaxVertices ||
			m_indexCount + segments * 6 >= MaxIndices)
		{
			flushBatch();
		}

		SDL_FColor col = COLOR_CAST(color);

		i32 base = m_vertexCount;

		f32 angleRange = endAngle - startAngle;

		// Generate vertices: outer arc + inner arc
		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = f32(i) / f32(segments);
			f32 angle = startAngle + t * angleRange;

			f32 cs = std::cos(angle);
			f32 sn = std::sin(angle);

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
		for (i32 i = 0; i < segments; i++)
		{
			i32 o0 = base + i * 2;
			i32 i0 = o0 + 1;
			i32 o1 = o0 + 2;
			i32 i1 = o0 + 3;

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

	void BasicRenderer2D::generate_filled_ellipse_stroke(const Vec2& center, f32 radiusX, f32 radiusY, f32 startAngle, const Color& color, i32 segments)
	{
		// Ensure capacity
		if (m_vertexCount + segments + 2 >= MaxVertices || m_indexCount + segments * 3 >= MaxIndices)
			flushBatch();

		SDL_FColor col = COLOR_CAST(color);

		i32 base = m_vertexCount;

		// Center vertex
		m_vertices[m_vertexCount++] = {
			{ center.x, center.y },
			col,
			{ 0, 0 }
		};

		f32 endAngle = startAngle + M_PI * 0.5f;

		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = f32(i) / f32(segments);
			f32 angle = startAngle + t * (endAngle - startAngle);

			f32 x = center.x + std::cos(angle) * radiusX;
			f32 y = center.y + std::sin(angle) * radiusY;

			m_vertices[m_vertexCount++] = {
				{ x, y },
				col,
				{ 0, 0 }
			};
		}

		// Triangle fan indices
		for (i32 i = 0; i < segments; i++)
		{
			m_indices[m_indexCount++] = base;
			m_indices[m_indexCount++] = base + 1 + i;
			m_indices[m_indexCount++] = base + 2 + i;
		}
	}

	void BasicRenderer2D::generate_filled_ellipse(const Vec2& center, f32 radiusX, f32 radiusY, const Color& color, i32 segments)
	{
		// Ensure capacity
		if (m_vertexCount + segments + 2 >= MaxVertices || m_indexCount + segments * 3 >= MaxIndices)
			flushBatch();

		SDL_FColor col = COLOR_CAST(color);

		i32 base = m_vertexCount;

		// Center vertex
		m_vertices[m_vertexCount++] = {
			{ center.x, center.y },
			col,
			{ 0, 0 }
		};

		// Outer ring vertices
		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = f32(i) / f32(segments);
			f32 angle = t * (2.0f * M_PI);

			f32 x = center.x + std::cos(angle) * radiusX;
			f32 y = center.y + std::sin(angle) * radiusY;

			m_vertices[m_vertexCount++] = {
				{ x, y },
				col,
				{ 0, 0 }
			};
		}

		// Triangle fan indices
		for (i32 i = 0; i < segments; i++)
		{
			m_indices[m_indexCount++] = base;
			m_indices[m_indexCount++] = base + 1 + i;
			m_indices[m_indexCount++] = base + 2 + i;
		}
	}

	void BasicRenderer2D::push_polygon(const Vec2* verts, const Vec2* texCoords, u32 vertCount, const u32* inds, u32 indexCount, const Color& color, SDL_Texture* texture)
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

		i32 base = m_vertexCount;
		bool hasTexCoords = texCoords != nullptr;
		for (i32 i = 0; i < vertCount; i++)
		{
			Vec2 tc { 0.0f, 0.0f };
			if (hasTexCoords)
				tc = texCoords[i];
			m_vertices[m_vertexCount++] = {
				{ verts[i].x, verts[i].y },
				col,
				{ tc.x, tc.y },
			};
		}
		for (i32 i = 0; i < indexCount; i++)
			m_indices[m_indexCount++] = base + inds[i];
	}

	void BasicRenderer2D::print_ttf_error(const String& funcName)
	{
		m_out.fg(ostd::ConsoleColors::Red).p(funcName).p("(...) failed: ").p(String(SDL_GetError())).reset().nl();
	}
	// ===================================================== PRIVATE HELPERS =====================================================
}
