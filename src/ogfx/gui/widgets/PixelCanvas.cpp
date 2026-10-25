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

#include "PixelCanvas.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../../render/PixelFont.hpp"
#include "../Window.hpp"
#include <cmath>
#include <limits>

namespace ogfx
{
	namespace gui
	{
		namespace
		{
			// Source-over blend of straight-alpha 0xAARRGGBB pixels.
			inline u32 blend_pixel(u32 dst, u32 src)
			{
				const u32 sa = src >> 24;
				if (sa == 255) return src;
				if (sa == 0) return dst;
				const u32 da = dst >> 24;
				const u32 dw = da * (255 - sa);              // destination weight * 255
				const u32 oa255 = sa * 255 + dw;             // output alpha * 255
				const u32 half = oa255 / 2;
				const u32 r = (((src >> 16) & 0xFF) * sa * 255 + ((dst >> 16) & 0xFF) * dw + half) / oa255;
				const u32 g = (((src >> 8) & 0xFF) * sa * 255 + ((dst >> 8) & 0xFF) * dw + half) / oa255;
				const u32 b = ((src & 0xFF) * sa * 255 + (dst & 0xFF) * dw + half) / oa255;
				const u32 a = (oa255 + 127) / 255;
				return (a << 24) | (r << 16) | (g << 8) | b;
			}

			// The color with its alpha multiplied by coverage / 255.
			inline u32 with_coverage(u32 color, u32 coverage)
			{
				const u32 a = ((color >> 24) * coverage + 127) / 255;
				return (a << 24) | (color & 0x00FFFFFF);
			}

			constexpr u32 rgb(u32 r, u32 g, u32 b) { return 0xFF000000u | (r << 16) | (g << 8) | b; }
		}





		// ===================================================== COLOR PALETTE 256 =====================================================
		const PixelCanvas::ColorPalette256 PixelCanvas::DefaultPalette;

		PixelCanvas::ColorPalette256::ColorPalette256(void)
		{
			reset();
		}

		void PixelCanvas::ColorPalette256::reset(void)
		{
			// The xterm 256-color palette: 16 standard colors, a 6x6x6 cube, 24 grays.
			static constexpr u32 standard[16] = {
				rgb(0, 0, 0),       rgb(128, 0, 0),   rgb(0, 128, 0),   rgb(128, 128, 0),
				rgb(0, 0, 128),     rgb(128, 0, 128), rgb(0, 128, 128), rgb(192, 192, 192),
				rgb(128, 128, 128), rgb(255, 0, 0),   rgb(0, 255, 0),   rgb(255, 255, 0),
				rgb(0, 0, 255),     rgb(255, 0, 255), rgb(0, 255, 255), rgb(255, 255, 255)
			};
			static constexpr u32 levels[6] = { 0, 95, 135, 175, 215, 255 };
			for (u32 i = 0; i < 16; i++)
				m_colors[i] = standard[i];
			for (u32 i = 0; i < 216; i++)
				m_colors[16 + i] = rgb(levels[i / 36], levels[(i / 6) % 6], levels[i % 6]);
			for (u32 i = 0; i < 24; i++)
				m_colors[232 + i] = rgb(8 + i * 10, 8 + i * 10, 8 + i * 10);
			m_colors[0] = 0; // always transparent
			m_version++;
		}

		void PixelCanvas::ColorPalette256::setColor(u8 index, const Color& color)
		{
			if (index == 0)
				return;
			const u32 pixel = toPixel(color);
			if (m_colors[index] == pixel)
				return;
			m_colors[index] = pixel;
			m_version++;
		}

		Color PixelCanvas::ColorPalette256::getColor(u8 index) const
		{
			return toColor(m_colors[index]);
		}
		// ===================================================== COLOR PALETTE 256 =====================================================





		// ===================================================== MAIN =====================================================
		PixelCanvas::~PixelCanvas(void)
		{
		}

		PixelCanvas& PixelCanvas::create(i32 width, i32 height)
		{
			setPadding({ 0, 0, 0, 0 });
			setTypeName("ogfx::gui::PixelCanvas");
			disableChildren();
			disableBackground();
			disableBorder();
			setStylesheetCategoryName("pixelcanvas");
			m_startTime = Clock::now();
			setResolution(width, height);
			validate();
			return *this;
		}

		void PixelCanvas::onUpdate(void)
		{
			const auto now = Clock::now();
			if (!m_ticked)
			{
				m_ticked = true;
				m_lastTick = now;
				m_nextTick = now;
			}
			// Deadline based: the next frame is due one interval after the previous deadline (not
			// after the previous frame), so the canvas averages its frame rate whatever the GUI's
			// frame timing. A little tolerance lets a GUI frame that arrives just before the
			// deadline run it, instead of waiting a whole GUI frame more.
			if (now + std::chrono::duration_cast<Clock::duration>(std::chrono::duration<f64>(FrameTolerance)) < m_nextTick)
				return;
			const auto interval = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<f64>(m_frameInterval));
			m_nextTick += interval;
			if (m_nextTick < now)
				m_nextTick = now + interval; // fell behind (a stall): don't try to catch up
			const f64 elapsed = std::chrono::duration<f64>(now - m_lastTick).count();
			m_lastTick = now;
			tick(std::min(elapsed, 0.25));
		}

		void PixelCanvas::tick(f64 deltaTime)
		{
			onCanvasUpdate(deltaTime);
			if (m_updateCallback)
				m_updateCallback(*this, deltaTime);
			render_canvas_frame();
			upload();
			m_frameCount++;
			m_rateFrames++;
			m_rateWindow += deltaTime;
			if (m_rateWindow >= 0.5)
			{
				m_measuredFrameRate = cast<f32>(m_rateFrames / m_rateWindow);
				m_rateFrames = 0;
				m_rateWindow = 0.0;
			}
		}

		void PixelCanvas::upload(void)
		{
			if (!ensure_image(getWindow().getGFX()))
				return;
			if (m_image.contentsLost())
			{
				m_image.updatePixels(m_pixels.data());
			}
			else if (m_dirtyX0 < m_dirtyX1 && m_dirtyY0 < m_dirtyY1)
			{
				const i32 x0 = std::max(m_dirtyX0, 0), y0 = std::max(m_dirtyY0, 0);
				const i32 x1 = std::min(m_dirtyX1, m_width), y1 = std::min(m_dirtyY1, m_height);
				if (x0 < x1 && y0 < y1)
					m_image.updatePixels(m_pixels.data(), { x0, y0, x1 - x0, y1 - y0 });
			}
			reset_dirty();
		}

		bool PixelCanvas::ensure_image(BasicRenderer2D& gfx)
		{
			if (!gfx.isInitialized() || m_pixels.empty())
				return false;
			const Vec2 size = m_image.getSize();
			if (!m_image.isLoaded() || cast<i32>(size.x) != m_width || cast<i32>(size.y) != m_height)
			{
				m_image.create(gfx, m_width, m_height);
				if (!m_image.isLoaded())
					return false;
				m_image.setSmoothFiltering(m_smoothScaling);
			}
			return true;
		}

		void PixelCanvas::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			if (m_letterboxColor.a > 0)
				gfx.fillRect(getGlobalContentBounds(), m_letterboxColor);
			if (!ensure_image(gfx))
				return;
			// First frame after (re)creating the texture: don't wait for the next canvas frame.
			if (m_image.contentsLost())
				m_image.updatePixels(m_pixels.data());
			const Rectangle dest = compute_display_bounds();
			if (dest.w <= 0 || dest.h <= 0)
				return;

			const bool shaders = areEffectsSupported();
			const bool crt = m_crtEnabled && shaders && m_crt && m_crt->isLoaded();
			const bool bloom = m_bloomEnabled && shaders && m_bloom;
			const f32 time = cast<f32>(std::chrono::duration<f64>(Clock::now() - m_startTime).count());
			// The CRT shader does its own (sharp) filtering and needs a linearly filtered texture.
			const bool smooth = crt || m_smoothScaling;
			if (m_image.isSmoothFiltering() != smooth)
				m_image.setSmoothFiltering(smooth);
			auto draw_canvas = [&](const Vec2& position, const Vec2& size) {
				if (!crt)
				{
					gfx.drawImage(m_image, position, size);
					return;
				}
				m_crt->prepare({ cast<f32>(m_width), cast<f32>(m_height) }, size * gfx.getPixelScale(), time);
				gfx.beginShader(*m_crt);
				gfx.drawImage(m_image, position, size);
				gfx.endShader();
			};

			if (!bloom)
			{
				draw_canvas(dest.getPosition(), dest.getSize());
				return;
			}
			// Bloom needs the (CRT-processed) canvas in a render target.
			if (m_effectTarget.getRenderer() != &gfx)
				m_effectTarget.create(gfx, dest.getSize());
			else
				m_effectTarget.resize(dest.getSize());
			if (!gfx.beginRenderTarget(m_effectTarget))
			{
				draw_canvas(dest.getPosition(), dest.getSize());
				return;
			}
			draw_canvas({ 0, 0 }, dest.getSize());
			gfx.endRenderTarget();
			gfx.drawRenderTarget(m_effectTarget, dest.getPosition(), dest.getSize());
			m_bloom->draw(gfx, m_effectTarget, dest.getPosition(), dest.getSize());
		}
		// ===================================================== MAIN =====================================================





		// ===================================================== SETTINGS =====================================================
		void PixelCanvas::setResolution(i32 width, i32 height)
		{
			m_width = std::max(width, 1);
			m_height = std::max(height, 1);
			m_pixels.assign(cast<size_t>(m_width) * m_height, 0);
			reset_dirty();
			mark_all_dirty();
		}

		void PixelCanvas::setFrameRate(f32 fps)
		{
			m_frameRate = std::clamp(fps, 1.0f, 240.0f);
			m_frameInterval = 1.0 / m_frameRate;
		}

		void PixelCanvas::setFont(const PixelFont& font)
		{
			m_font = &font;
		}

		void PixelCanvas::setSmoothScaling(bool smooth)
		{
			m_smoothScaling = smooth;
			if (m_image.isLoaded())
				m_image.setSmoothFiltering(smooth);
		}

		Rectangle PixelCanvas::getCanvasDisplayBounds(void) const
		{
			return compute_display_bounds();
		}

		Rectangle PixelCanvas::compute_display_bounds(void) const
		{
			const Rectangle area = getGlobalContentBounds();
			if (area.w <= 0 || area.h <= 0 || m_width <= 0 || m_height <= 0)
				return { area.x, area.y, 0, 0 };
			// Computed in physical pixels, so integer scaling and the centering are pixel-exact.
			const f32 ps = getWindow().getGFX().getPixelScale();
			const f32 areaW = area.w * ps, areaH = area.h * ps;
			f32 sx = areaW / m_width, sy = areaH / m_height;
			if (m_maintainAspectRatio)
				sx = sy = std::min(sx, sy);
			if (m_integerScaling)
			{
				if (sx >= 1.0f) sx = std::floor(sx);
				if (sy >= 1.0f) sy = std::floor(sy);
			}
			const f32 w = std::round(m_width * sx), h = std::round(m_height * sy);
			const f32 x = std::round(area.x * ps + (areaW - w) / 2.0f), y = std::round(area.y * ps + (areaH - h) / 2.0f);
			return { x / ps, y / ps, w / ps, h / ps };
		}

		bool PixelCanvas::mapToCanvas(const Vec2& windowPosition, IPoint& outPixel) const
		{
			const Rectangle dest = compute_display_bounds();
			if (dest.w <= 0 || dest.h <= 0)
				return false;
			const f32 fx = (windowPosition.x - dest.x) / dest.w * m_width;
			const f32 fy = (windowPosition.y - dest.y) / dest.h * m_height;
			outPixel = { cast<i32>(std::floor(fx)), cast<i32>(std::floor(fy)) };
			return outPixel.x >= 0 && outPixel.y >= 0 && outPixel.x < m_width && outPixel.y < m_height;
		}

		void PixelCanvas::reset_dirty(void)
		{
			m_dirtyX0 = m_dirtyY0 = std::numeric_limits<i32>::max();
			m_dirtyX1 = m_dirtyY1 = std::numeric_limits<i32>::min();
		}
		// ===================================================== SETTINGS =====================================================





		// ===================================================== EFFECTS =====================================================
		bool PixelCanvas::areEffectsSupported(void) const
		{
			return getWindow().getGFX().areShadersSupported();
		}

		void PixelCanvas::enableCRTEffect(bool enable)
		{
			m_crtEnabled = enable;
			if (enable && !m_crt)
			{
				m_crt = std::make_unique<CRTShader>();
				m_crt->setParams(m_crtParams);
			}
		}

		void PixelCanvas::setCRTParams(const CRTShader::Params& params)
		{
			m_crtParams = params;
			if (m_crt)
				m_crt->setParams(params);
		}

		CRTShader::Params PixelCanvas::getCRTParams(void) const
		{
			return m_crtParams;
		}

		void PixelCanvas::enableBloomEffect(bool enable)
		{
			m_bloomEnabled = enable;
			if (enable && !m_bloom)
			{
				m_bloom = std::make_unique<BloomEffect>();
				m_bloom->setParams(m_bloomParams);
			}
		}

		void PixelCanvas::setBloomParams(const BloomEffect::Params& params)
		{
			m_bloomParams = params;
			if (m_bloom)
				m_bloom->setParams(params);
		}

		BloomEffect::Params PixelCanvas::getBloomParams(void) const
		{
			return m_bloomParams;
		}
		// ===================================================== EFFECTS =====================================================





		// ===================================================== RAW PIXELS =====================================================
		u32* PixelCanvas::getPixels(bool markDirty)
		{
			if (!can_draw())
				return nullptr;
			if (markDirty)
				mark_all_dirty();
			return m_pixels.data();
		}

		const u32* PixelCanvas::getPixels(void) const
		{
			return can_draw() ? m_pixels.data() : nullptr;
		}

		void PixelCanvas::markDirty(const IRect& area)
		{
			if (area.size.x <= 0 || area.size.y <= 0)
				mark_all_dirty();
			else
				expand_dirty(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);
		}

		u32 PixelCanvas::toPixel(const Color& color)
		{
			return toPixel(color.r, color.g, color.b, color.a);
		}

		Color PixelCanvas::toColor(u32 pixel)
		{
			return Color(cast<u8>((pixel >> 16) & 0xFF), cast<u8>((pixel >> 8) & 0xFF), cast<u8>(pixel & 0xFF), cast<u8>(pixel >> 24));
		}
		// ===================================================== RAW PIXELS =====================================================





		// ===================================================== PRIMITIVES (INTERNAL) =====================================================
		void PixelCanvas::fill_span(i32 y, i32 x0, i32 x1, u32 color)
		{
			if (y < 0 || y >= m_height)
				return;
			x0 = std::max(x0, 0);
			x1 = std::min(x1, m_width);
			if (x0 >= x1)
				return;
			u32* row = m_pixels.data() + cast<size_t>(y) * m_width;
			const u32 alpha = color >> 24;
			if (alpha == 255)
				std::fill(row + x0, row + x1, color);
			else if (alpha > 0)
				for (i32 x = x0; x < x1; x++)
					row[x] = blend_pixel(row[x], color);
			else
				return;
			expand_dirty(x0, y, x1, y + 1);
		}

		void PixelCanvas::put_pixel(i32 x, i32 y, u32 color)
		{
			if (x < 0 || y < 0 || x >= m_width || y >= m_height)
				return;
			u32& p = m_pixels[cast<size_t>(y) * m_width + x];
			p = blend_pixel(p, color);
			expand_dirty(x, y, x + 1, y + 1);
		}

		bool PixelCanvas::rounded_span(f32 x, f32 y, f32 w, f32 h, f32 rx, f32 ry, f32 cy, i32& x0, i32& x1) const
		{
			if (w <= 0 || h <= 0 || cy < y || cy >= y + h)
				return false;
			f32 inset = 0.0f;
			if (rx > 0.0f && ry > 0.0f)
			{
				f32 dy = 0.0f;
				if (cy < y + ry)
					dy = (y + ry - cy) / ry;
				else if (cy > y + h - ry)
					dy = (cy - (y + h - ry)) / ry;
				if (dy > 0.0f)
					inset = rx * (1.0f - std::sqrt(std::max(0.0f, 1.0f - dy * dy)));
			}
			// Pixels whose centers (px + 0.5) are in [x + inset, x + w - inset).
			x0 = cast<i32>(std::ceil(x + inset - 0.5f));
			x1 = cast<i32>(std::ceil(x + w - inset - 0.5f));
			return x0 < x1;
		}

		void PixelCanvas::fill_rounded(f32 x, f32 y, f32 w, f32 h, f32 rx, f32 ry, u32 color)
		{
			if ((color >> 24) == 0 || w <= 0 || h <= 0)
				return;
			rx = std::min(rx, w / 2.0f);
			ry = std::min(ry, h / 2.0f);
			const i32 yStart = std::max(cast<i32>(std::floor(y)), 0);
			const i32 yEnd = std::min(cast<i32>(std::ceil(y + h)), m_height);
			i32 x0, x1;
			for (i32 py = yStart; py < yEnd; py++)
			{
				if (rounded_span(x, y, w, h, rx, ry, py + 0.5f, x0, x1))
					fill_span(py, x0, x1, color);
			}
		}

		void PixelCanvas::ring_rounded(f32 x, f32 y, f32 w, f32 h, f32 rx, f32 ry, f32 thickness, u32 color)
		{
			if ((color >> 24) == 0 || w <= 0 || h <= 0)
				return;
			thickness = std::max(thickness, 1.0f);
			rx = std::min(rx, w / 2.0f);
			ry = std::min(ry, h / 2.0f);
			const f32 ix = x + thickness, iy = y + thickness, iw = w - thickness * 2.0f, ih = h - thickness * 2.0f;
			const f32 irx = std::max(rx - thickness, 0.0f), iry = std::max(ry - thickness, 0.0f);
			const i32 yStart = std::max(cast<i32>(std::floor(y)), 0);
			const i32 yEnd = std::min(cast<i32>(std::ceil(y + h)), m_height);
			i32 x0, x1, ix0, ix1;
			for (i32 py = yStart; py < yEnd; py++)
			{
				const f32 cy = py + 0.5f;
				if (!rounded_span(x, y, w, h, rx, ry, cy, x0, x1))
					continue;
				if (rounded_span(ix, iy, iw, ih, irx, iry, cy, ix0, ix1))
				{
					fill_span(py, x0, std::min(ix0, x1), color);
					fill_span(py, std::max(ix1, x0), x1, color);
				}
				else
				{
					fill_span(py, x0, x1, color);
				}
			}
		}

		void PixelCanvas::fill_polygon(const Vec2* points, i32 count, u32 color)
		{
			if ((color >> 24) == 0 || count < 3)
				return;
			f32 minY = points[0].y, maxY = points[0].y;
			for (i32 i = 1; i < count; i++)
			{
				minY = std::min(minY, points[i].y);
				maxY = std::max(maxY, points[i].y);
			}
			const i32 yStart = std::max(cast<i32>(std::floor(minY)), 0);
			const i32 yEnd = std::min(cast<i32>(std::ceil(maxY)), m_height);
			std::array<f32, 16> crossings;
			for (i32 py = yStart; py < yEnd; py++)
			{
				const f32 cy = py + 0.5f;
				i32 n = 0;
				for (i32 i = 0, j = count - 1; i < count && n < cast<i32>(crossings.size()); j = i++)
				{
					const Vec2& a = points[i];
					const Vec2& b = points[j];
					// Half-open in y, so a vertex shared by two edges is counted once.
					if ((a.y <= cy && b.y > cy) || (b.y <= cy && a.y > cy))
						crossings[n++] = a.x + (cy - a.y) / (b.y - a.y) * (b.x - a.x);
				}
				std::sort(crossings.begin(), crossings.begin() + n);
				for (i32 k = 0; k + 1 < n; k += 2)
					fill_span(py, cast<i32>(std::ceil(crossings[k] - 0.5f)), cast<i32>(std::ceil(crossings[k + 1] - 0.5f)), color);
			}
		}

		void PixelCanvas::thin_line(i32 x1, i32 y1, i32 x2, i32 y2, u32 color)
		{
			// Bresenham, both end points included.
			const bool opaque = (color >> 24) == 255;
			const i32 dx = std::abs(x2 - x1), dy = -std::abs(y2 - y1);
			const i32 sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
			i32 err = dx + dy;
			const i32 minX = std::min(x1, x2), maxX = std::max(x1, x2), minY = std::min(y1, y2), maxY = std::max(y1, y2);
			while (true)
			{
				if (x1 >= 0 && y1 >= 0 && x1 < m_width && y1 < m_height)
				{
					u32& p = m_pixels[cast<size_t>(y1) * m_width + x1];
					p = opaque ? color : blend_pixel(p, color);
				}
				if (x1 == x2 && y1 == y2)
					break;
				const i32 e2 = err * 2;
				if (e2 >= dy) { err += dy; x1 += sx; }
				if (e2 <= dx) { err += dx; y1 += sy; }
			}
			expand_dirty(std::max(minX, 0), std::max(minY, 0), std::min(maxX + 1, m_width), std::min(maxY + 1, m_height));
		}

		void PixelCanvas::thick_line(const Vec2& p1, const Vec2& p2, f32 thickness, bool rounded, u32 color)
		{
			const f32 half = thickness / 2.0f;
			const Vec2 d { p2.x - p1.x, p2.y - p1.y };
			const f32 len = std::sqrt(d.x * d.x + d.y * d.y);
			if (len < 1e-4f)
			{
				if (rounded)
					fill_rounded(p1.x - half, p1.y - half, thickness, thickness, half, half, color);
				else
					fill_rounded(p1.x - half, p1.y - half, thickness, thickness, 0, 0, color);
				return;
			}
			const Vec2 n { -d.y / len * half, d.x / len * half };
			// Square caps reach half the thickness past the end points (like a w x h rectangle).
			const Vec2 e = rounded ? Vec2 { 0, 0 } : Vec2 { d.x / len * half, d.y / len * half };
			const Vec2 quad[4] = {
				{ p1.x + n.x - e.x, p1.y + n.y - e.y }, { p2.x + n.x + e.x, p2.y + n.y + e.y },
				{ p2.x - n.x + e.x, p2.y - n.y + e.y }, { p1.x - n.x - e.x, p1.y - n.y - e.y }
			};
			fill_polygon(quad, 4, color);
			if (rounded)
			{
				fill_rounded(p1.x - half, p1.y - half, thickness, thickness, half, half, color);
				fill_rounded(p2.x - half, p2.y - half, thickness, thickness, half, half, color);
			}
		}
		// ===================================================== PRIMITIVES (INTERNAL) =====================================================





		// ===================================================== DRAWING =====================================================
		void PixelCanvas::clear(const Color& color)
		{
			if (!can_draw())
				return;
			std::fill(m_pixels.begin(), m_pixels.end(), toPixel(color));
			mark_all_dirty();
		}

		void PixelCanvas::setPixel(i32 x, i32 y, const Color& color)
		{
			if (!can_draw() || x < 0 || y < 0 || x >= m_width || y >= m_height)
				return;
			m_pixels[cast<size_t>(y) * m_width + x] = toPixel(color);
			expand_dirty(x, y, x + 1, y + 1);
		}

		Color PixelCanvas::getPixel(i32 x, i32 y) const
		{
			if (m_pixels.empty() || x < 0 || y < 0 || x >= m_width || y >= m_height)
				return Colors::Transparent;
			return toColor(m_pixels[cast<size_t>(y) * m_width + x]);
		}

		void PixelCanvas::blit(const u32* pixels, i32 width, i32 height, i32 x, i32 y, bool blend)
		{
			if (!can_draw() || !pixels || width <= 0 || height <= 0)
				return;
			const i32 x0 = std::max(x, 0), y0 = std::max(y, 0);
			const i32 x1 = std::min(x + width, m_width), y1 = std::min(y + height, m_height);
			if (x0 >= x1 || y0 >= y1)
				return;
			for (i32 py = y0; py < y1; py++)
			{
				const u32* src = pixels + cast<size_t>(py - y) * width + (x0 - x);
				u32* dst = m_pixels.data() + cast<size_t>(py) * m_width + x0;
				if (!blend)
					std::copy(src, src + (x1 - x0), dst);
				else
					for (i32 i = 0; i < x1 - x0; i++)
						dst[i] = blend_pixel(dst[i], src[i]);
			}
			expand_dirty(x0, y0, x1, y1);
		}

		void PixelCanvas::drawLine(const IPoint& p1, const IPoint& p2, const Color& color, i32 thickness, bool rounded)
		{
			if (!can_draw() || color.a == 0)
				return;
			if (thickness <= 1)
				thin_line(p1.x, p1.y, p2.x, p2.y, toPixel(color));
			else
				thick_line({ p1.x + 0.5f, p1.y + 0.5f }, { p2.x + 0.5f, p2.y + 0.5f }, cast<f32>(thickness), rounded, toPixel(color));
		}

		void PixelCanvas::drawRect(const IRect& rect, const Color& color, i32 thickness)
		{
			if (!can_draw())
				return;
			ring_rounded(rect.position.x, rect.position.y, rect.size.x, rect.size.y, 0, 0, thickness, toPixel(color));
		}

		void PixelCanvas::drawRoundRect(const IRect& rect, const Color& color, i32 radius, i32 thickness)
		{
			if (!can_draw())
				return;
			ring_rounded(rect.position.x, rect.position.y, rect.size.x, rect.size.y, radius, radius, thickness, toPixel(color));
		}

		void PixelCanvas::drawCircle(const IPoint& center, i32 radius, const Color& color, i32 thickness)
		{
			if (!can_draw() || radius < 0)
				return;
			// The pixel at `center` is the middle: the circle spans 2 * radius + 1 pixels.
			const f32 r = radius + 0.5f;
			ring_rounded(center.x - radius, center.y - radius, r * 2.0f, r * 2.0f, r, r, thickness, toPixel(color));
		}

		void PixelCanvas::drawEllipse(const IRect& rect, const Color& color, i32 thickness)
		{
			if (!can_draw())
				return;
			ring_rounded(rect.position.x, rect.position.y, rect.size.x, rect.size.y, rect.size.x / 2.0f, rect.size.y / 2.0f, thickness, toPixel(color));
		}

		void PixelCanvas::drawTriangle(const IPoint& a, const IPoint& b, const IPoint& c, const Color& color, i32 thickness)
		{
			if (!can_draw() || color.a == 0)
				return;
			const u32 pixel = toPixel(color);
			if (thickness <= 1)
			{
				thin_line(a.x, a.y, b.x, b.y, pixel);
				thin_line(b.x, b.y, c.x, c.y, pixel);
				thin_line(c.x, c.y, a.x, a.y, pixel);
				return;
			}
			const Vec2 pa { a.x + 0.5f, a.y + 0.5f }, pb { b.x + 0.5f, b.y + 0.5f }, pc { c.x + 0.5f, c.y + 0.5f };
			const f32 t = cast<f32>(thickness);
			thick_line(pa, pb, t, true, pixel);
			thick_line(pb, pc, t, true, pixel);
			thick_line(pc, pa, t, true, pixel);
		}

		void PixelCanvas::fillRect(const IRect& rect, const Color& color)
		{
			if (!can_draw())
				return;
			const u32 pixel = toPixel(color);
			const i32 y0 = std::max(rect.position.y, 0), y1 = std::min(rect.position.y + rect.size.y, m_height);
			for (i32 y = y0; y < y1; y++)
				fill_span(y, rect.position.x, rect.position.x + rect.size.x, pixel);
		}

		void PixelCanvas::fillRoundRect(const IRect& rect, const Color& color, i32 radius)
		{
			if (!can_draw())
				return;
			fill_rounded(rect.position.x, rect.position.y, rect.size.x, rect.size.y, radius, radius, toPixel(color));
		}

		void PixelCanvas::fillCircle(const IPoint& center, i32 radius, const Color& color)
		{
			if (!can_draw() || radius < 0)
				return;
			const f32 r = radius + 0.5f;
			fill_rounded(center.x - radius, center.y - radius, r * 2.0f, r * 2.0f, r, r, toPixel(color));
		}

		void PixelCanvas::fillEllipse(const IRect& rect, const Color& color)
		{
			if (!can_draw())
				return;
			fill_rounded(rect.position.x, rect.position.y, rect.size.x, rect.size.y, rect.size.x / 2.0f, rect.size.y / 2.0f, toPixel(color));
		}

		void PixelCanvas::fillTriangle(const IPoint& a, const IPoint& b, const IPoint& c, const Color& color)
		{
			if (!can_draw())
				return;
			const Vec2 points[3] = { { a.x + 0.5f, a.y + 0.5f }, { b.x + 0.5f, b.y + 0.5f }, { c.x + 0.5f, c.y + 0.5f } };
			fill_polygon(points, 3, toPixel(color));
		}

		void PixelCanvas::outlinedRect(const IRect& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
		{
			if (!can_draw())
				return;
			const i32 t = std::max(outlineThickness, 1);
			fillRect({ rect.position.x + t, rect.position.y + t, rect.size.x - t * 2, rect.size.y - t * 2 }, fillColor);
			drawRect(rect, outlineColor, t);
		}

		void PixelCanvas::outlinedRoundRect(const IRect& rect, const Color& fillColor, const Color& outlineColor, i32 radius, i32 outlineThickness)
		{
			if (!can_draw())
				return;
			const i32 t = std::max(outlineThickness, 1);
			fill_rounded(rect.position.x + t, rect.position.y + t, rect.size.x - t * 2, rect.size.y - t * 2, std::max(radius - t, 0), std::max(radius - t, 0), toPixel(fillColor));
			drawRoundRect(rect, outlineColor, radius, t);
		}

		void PixelCanvas::outlinedCircle(const IPoint& center, i32 radius, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
		{
			if (!can_draw() || radius < 0)
				return;
			const f32 t = cast<f32>(std::max(outlineThickness, 1));
			const f32 r = radius + 0.5f;
			const f32 x = center.x - radius, y = center.y - radius;
			fill_rounded(x + t, y + t, r * 2.0f - t * 2.0f, r * 2.0f - t * 2.0f, r - t, r - t, toPixel(fillColor));
			ring_rounded(x, y, r * 2.0f, r * 2.0f, r, r, t, toPixel(outlineColor));
		}

		void PixelCanvas::outlinedEllipse(const IRect& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
		{
			if (!can_draw())
				return;
			const f32 t = cast<f32>(std::max(outlineThickness, 1));
			const f32 rx = rect.size.x / 2.0f, ry = rect.size.y / 2.0f;
			fill_rounded(rect.position.x + t, rect.position.y + t, rect.size.x - t * 2.0f, rect.size.y - t * 2.0f, std::max(rx - t, 0.0f), std::max(ry - t, 0.0f), toPixel(fillColor));
			ring_rounded(rect.position.x, rect.position.y, rect.size.x, rect.size.y, rx, ry, t, toPixel(outlineColor));
		}

		void PixelCanvas::outlinedTriangle(const IPoint& a, const IPoint& b, const IPoint& c, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
		{
			if (!can_draw())
				return;
			fillTriangle(a, b, c, fillColor);
			drawTriangle(a, b, c, outlineColor, outlineThickness);
		}
		// ===================================================== DRAWING =====================================================





		// ===================================================== TEXT =====================================================
		void PixelCanvas::draw_glyph(u8 character, i32 x, i32 y, i32 cellW, i32 cellH, u32 color, u32 background)
		{
			const i32 x0 = std::max(x, 0), y0 = std::max(y, 0);
			const i32 x1 = std::min(x + cellW, m_width), y1 = std::min(y + cellH, m_height);
			if (x0 >= x1 || y0 >= y1)
				return;
			const u8* glyph = m_font->getGlyph(character, cellW, cellH);
			const bool opaqueBackground = (background >> 24) == 255;
			const bool hasBackground = (background >> 24) != 0;
			for (i32 py = y0; py < y1; py++)
			{
				u32* dst = m_pixels.data() + cast<size_t>(py) * m_width;
				const u8* coverage = glyph ? glyph + cast<size_t>(py - y) * cellW - x : nullptr;
				for (i32 px = x0; px < x1; px++)
				{
					u32 out = dst[px];
					if (hasBackground)
						out = opaqueBackground ? background : blend_pixel(out, background);
					const u32 cov = coverage ? coverage[px] : 0;
					if (cov != 0)
						out = blend_pixel(out, cov == 255 ? color : with_coverage(color, cov));
					dst[px] = out;
				}
			}
			expand_dirty(x0, y0, x1, y1);
		}

		void PixelCanvas::drawCharacter(u8 character, i32 x, i32 y, const Color& color, const Color& background, i32 scale)
		{
			if (!can_draw() || !m_font || !m_font->isLoaded())
				return;
			scale = std::max(scale, 1);
			draw_glyph(character, x, y, m_font->getCharWidth() * scale, m_font->getCharHeight() * scale, toPixel(color), toPixel(background));
		}

		void PixelCanvas::drawString(const String& text, i32 x, i32 y, const Color& color, const Color& background, i32 scale)
		{
			if (!can_draw() || !m_font || !m_font->isLoaded())
				return;
			scale = std::max(scale, 1);
			const i32 cellW = m_font->getCharWidth() * scale, cellH = m_font->getCharHeight() * scale;
			const u32 fg = toPixel(color), bg = toPixel(background);
			i32 penX = x, penY = y;
			for (const char ch : text.cpp_str())
			{
				if (ch == '\n')
				{
					penX = x;
					penY += cellH;
					continue;
				}
				draw_glyph(cast<u8>(ch), penX, penY, cellW, cellH, fg, bg);
				penX += cellW;
			}
		}

		IPoint PixelCanvas::getStringSize(const String& text, i32 scale) const
		{
			if (!m_font || !m_font->isLoaded())
				return { 0, 0 };
			scale = std::max(scale, 1);
			i32 columns = 0, maxColumns = 0, lines = 1;
			for (const char ch : text.cpp_str())
			{
				if (ch == '\n')
				{
					lines++;
					columns = 0;
					continue;
				}
				maxColumns = std::max(maxColumns, ++columns);
			}
			return { maxColumns * m_font->getCharWidth() * scale, lines * m_font->getCharHeight() * scale };
		}
		// ===================================================== TEXT =====================================================
	}
}
