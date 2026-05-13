/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev
	(license header same as the project)
*/

#include "ColorPicker.hpp"
#include "../../render/BasicRenderer.hpp"
#include <cmath>
#include <algorithm>

namespace ogfx
{
	namespace gui
	{
		// ===================================================================
		// Lifecycle
		// ===================================================================
		ColorPicker& ColorPicker::create(void)
		{
			setPadding({ 4, 4, 4, 4 });
			setTypeName("ogfx::gui::ColorPicker");
			setStylesheetCategoryName("colorPicker");
			enableBackground();
			enableBorder();
			disableFocus();
			setContentOffset({ 0, 0 });
			setSize({ 320, 180 });
			validate();
			return *this;
		}

		void ColorPicker::applyTheme(const ostd::Stylesheet& theme)
		{
			setStripWidth(getThemeValue<f32>(theme, "stripWidth", m_stripWidth));
			setRegionSpacing(getThemeValue<f32>(theme, "regionSpacing", m_regionSpacing));
			setCursorRadius(getThemeValue<f32>(theme, "cursorRadius", m_cursorRadius));
			setInfoColumnWidth(getThemeValue<f32>(theme, "infoColumnWidth", m_infoColumnW));
			setCheckerSize(getThemeValue<f32>(theme, "checkerSize", m_checkerSize));
			setGradientSteps(getThemeValue<i32>(theme, "gradientSteps", m_gradientSteps));
			enableAlphaChannel(getThemeValue<bool>(theme, "showAlpha", m_showAlpha));
			enableColorPreview(getThemeValue<bool>(theme, "showPreview", m_showPreview));
			m_showPreview = false; // Overridden to disable functionality without removing the code
		}

		void ColorPicker::onDraw(BasicRenderer2D& gfx)
		{
			const Rectangle sv = sv_bounds();
			const Rectangle hue = hue_bounds();
			draw_sv_square(gfx, sv);
			draw_hue_strip(gfx, hue);
			draw_sv_cursor(gfx, sv);
			draw_strip_cursor(gfx, hue, m_hue);

			if (m_showAlpha)
			{
				const Rectangle al = alpha_bounds();
				draw_alpha_strip(gfx, al);
				draw_strip_cursor(gfx, al, 1.0f - m_alpha);
			}

			if (m_showPreview)
			{
				const Rectangle sw = swatch_bounds();
				draw_checkerboard(gfx, sw);
				gfx.fillRect(sw, getColor());
				gfx.drawRect(sw, getBorderColor(), 1);
				// The TextEdit child draws itself via the widget tree.
			}
		}

		// ===================================================================
		// Public API
		// ===================================================================
		Color ColorPicker::getColor(void) const
		{
			f32 r, g, b;
			Color::HSVtoRGB(m_hue, m_sat, m_val, r, g, b);
			return Color(
				static_cast<u8>(std::round(r * 255.0f)),
				static_cast<u8>(std::round(g * 255.0f)),
				static_cast<u8>(std::round(b * 255.0f)),
				static_cast<u8>(std::round(m_alpha * 255.0f))
			);
		}

		void ColorPicker::setColor(const Color& c)
		{
			const f32 nr = c.r.value / 255.0f;
			const f32 ng = c.g.value / 255.0f;
			const f32 nb = c.b.value / 255.0f;
			f32 h, s, v;
			Color::RGBtoHSV(nr, ng, nb, h, s, v);

			// Hue preservation: if the incoming color is grayscale, keep the
			// current hue. Otherwise the hue cursor would snap to 0 every
			// time someone passes in black/gray/white.
			if (s > 0.0001f)
				m_hue = h;
			m_sat = s;
			m_val = v;
			m_alpha = c.a.value / 255.0f;

			fire_changed();
		}

		void ColorPicker::setHSV(f32 h, f32 s, f32 v)
		{
			m_hue = std::clamp(h, 0.0f, 1.0f);
			m_sat = std::clamp(s, 0.0f, 1.0f);
			m_val = std::clamp(v, 0.0f, 1.0f);
			fire_changed();
		}

		void ColorPicker::setAlpha(f32 a)
		{
			m_alpha = std::clamp(a, 0.0f, 1.0f);
			fire_changed();
		}

		// ===================================================================
		// Geometry
		// ===================================================================
		// The widget's content area is split:
		//   [ SV square ][ gap ][ hue ][ gap ][ alpha? ][ gap ][ info column ]
		//
		// SV square takes whatever's left over after the right-hand stuff.
		// All bounds are in GLOBAL coordinates so they can be compared
		// directly with mouse event positions.
		Rectangle ColorPicker::sv_bounds(void) const
		{
			const Rectangle cb = getGlobalContentBounds();
			f32 rightConsumed = m_stripWidth + m_regionSpacing; // hue
			if (m_showAlpha)   rightConsumed += m_stripWidth + m_regionSpacing;
			if (m_showPreview) rightConsumed += m_infoColumnW + m_regionSpacing;
			const f32 sqSize = std::max(20.0f, cb.w - rightConsumed);
			return { cb.x, cb.y, sqSize, cb.h };
		}

		Rectangle ColorPicker::hue_bounds(void) const
		{
			const Rectangle cb = getGlobalContentBounds();
			const Rectangle sv = sv_bounds();
			return { sv.x + sv.w + m_regionSpacing, cb.y, m_stripWidth, cb.h };
		}

		Rectangle ColorPicker::alpha_bounds(void) const
		{
			if (!m_showAlpha) return { 0, 0, 0, 0 };
			const Rectangle cb = getGlobalContentBounds();
			const Rectangle hb = hue_bounds();
			return { hb.x + hb.w + m_regionSpacing, cb.y, m_stripWidth, cb.h };
		}

		Rectangle ColorPicker::swatch_bounds(void) const
		{
			if (!m_showPreview) return { 0, 0, 0, 0 };
			const Rectangle cb = getGlobalContentBounds();
			const Rectangle anchor = m_showAlpha ? alpha_bounds() : hue_bounds();
			const f32 x = anchor.x + anchor.w + m_regionSpacing;
			const f32 swatchH = 40.0f;
			return { x, cb.y, m_infoColumnW, swatchH };
		}

		// ===================================================================
		// Hit testing & mouse
		// ===================================================================
		ColorPicker::Region ColorPicker::hit_test(const Vec2& m) const
		{
			if (sv_bounds().contains(m, true))    return Region::SV;
			if (hue_bounds().contains(m, true))   return Region::Hue;
			if (m_showAlpha && alpha_bounds().contains(m, true)) return Region::Alpha;
			return Region::None;
		}

		void ColorPicker::update_from_mouse(Region region, const Vec2& m)
		{
			auto norm = [](f32 v, f32 lo, f32 hi) {
				return std::clamp((v - lo) / (hi - lo), 0.0f, 1.0f);
			};

			if (region == Region::SV)
			{
				const Rectangle b = sv_bounds();
				m_sat = norm(m.x, b.x, b.x + b.w);
				m_val = 1.0f - norm(m.y, b.y, b.y + b.h);
			}
			else if (region == Region::Hue)
			{
				const Rectangle b = hue_bounds();
				// Top is hue 0 (red), bottom wraps back near red.
				m_hue = norm(m.y, b.y, b.y + b.h);
				// Clamp to avoid h == 1.0 causing HSVtoRGB to wrap weirdly.
				if (m_hue >= 1.0f) m_hue = 0.9999f;
			}
			else if (region == Region::Alpha)
			{
				const Rectangle b = alpha_bounds();
				// Top = fully opaque, bottom = fully transparent.
				m_alpha = 1.0f - norm(m.y, b.y, b.y + b.h);
			}
			else
			{
				return;
			}
			fire_changed();
		}

		void ColorPicker::onMousePressed(const Event& event)
		{
			const Vec2 m { event.mouse->position_x, event.mouse->position_y };
			m_activeDrag = hit_test(m);
			if (m_activeDrag != Region::None)
			{
				update_from_mouse(m_activeDrag, m);
				event.handle();
			}
		}

		void ColorPicker::onMouseDragged(const Event& event)
		{
			if (m_activeDrag == Region::None) return;
			if (event.mouse->button != MouseEventData::eButton::Left) return;
			const Vec2 m { event.mouse->position_x, event.mouse->position_y };
			update_from_mouse(m_activeDrag, m);
			event.handle();
		}

		void ColorPicker::onMouseReleased(const Event& /*event*/)
		{
			m_activeDrag = Region::None;
		}

		// ===================================================================
		// Drawing
		// ===================================================================
		Color ColorPicker::current_pure_hue(void) const
		{
			f32 r, g, b;
			Color::HSVtoRGB(m_hue, 1.0f, 1.0f, r, g, b);
			return Color(
				static_cast<u8>(std::round(r * 255.0f)),
				static_cast<u8>(std::round(g * 255.0f)),
				static_cast<u8>(std::round(b * 255.0f)),
				255
			);
		}

		void ColorPicker::draw_sv_square(BasicRenderer2D& gfx, const Rectangle& r)
		{
			// Strategy: divide the square into N vertical stripes. For each
			// stripe i at saturation s_i = i/N, draw a vertical gradient from
			// HSV(hue, s_i, 1)  (top) down to BLACK (bottom). This is exactly
			// the canonical HSV "saturation X, value Y" 2D gradient — no
			// shaders needed.
			const i32 N = std::max(8, m_gradientSteps);
			const f32 stripeW = r.w / static_cast<f32>(N);

			for (i32 i = 0; i < N; ++i)
			{
				const f32 s = (i + 0.5f) / static_cast<f32>(N);
				f32 rr, gg, bb;
				Color::HSVtoRGB(m_hue, s, 1.0f, rr, gg, bb);
				const Color topColor(
					static_cast<u8>(std::round(rr * 255.0f)),
					static_cast<u8>(std::round(gg * 255.0f)),
					static_cast<u8>(std::round(bb * 255.0f)),
					255
				);

				ColorGradient grad({ topColor, Colors::Black }, { 1.0f });
				grad.setAngleDeg(ColorGradient::VerticalDeg);
				Rectangle stripe {
					r.x + i * stripeW,
					r.y,
					stripeW + 0.5f, // tiny overlap so no seams between stripes
					r.h
				};
				gfx.fillGradientRect(stripe, grad);
			}

			// Border on top to crisp it up.
			gfx.drawRect(r, getBorderColor(), 1);
		}

		void ColorPicker::draw_hue_strip(BasicRenderer2D& gfx, const Rectangle& r)
		{
			// Six bands across the hue wheel: red -> yellow -> green -> cyan
			// -> blue -> magenta -> red. ColorGradient handles them in one call.
			ColorGradient grad(
				{
					Color(255,   0,   0),
					Color(255, 255,   0),
					Color(  0, 255,   0),
					Color(  0, 255, 255),
					Color(  0,   0, 255),
					Color(255,   0, 255),
					Color(255,   0,   0)
				},
				{ 1, 1, 1, 1, 1, 1 } // equal weights -> normalized to 1/6 each
			);
			grad.setAngleDeg(ColorGradient::VerticalDeg);
			gfx.fillGradientRect(r, grad);
			gfx.drawRect(r, getBorderColor(), 1);
		}

		void ColorPicker::draw_alpha_strip(BasicRenderer2D& gfx, const Rectangle& r)
		{
			// Checkerboard background so transparency is visible.
			draw_checkerboard(gfx, r);

			// Color gradient: pure-color-opaque at top -> same-color-transparent at bottom.
			Color top = current_pure_hue();
			f32 rr, gg, bb;
			Color::HSVtoRGB(m_hue, m_sat, m_val, rr, gg, bb);
			top.r = static_cast<u8>(std::round(rr * 255.0f));
			top.g = static_cast<u8>(std::round(gg * 255.0f));
			top.b = static_cast<u8>(std::round(bb * 255.0f));
			top.a = 255;
			Color bottom = top;
			bottom.a = 0;

			ColorGradient grad({ top, bottom }, { 1.0f });
			grad.setAngleDeg(ColorGradient::VerticalDeg);
			gfx.fillGradientRect(r, grad);
			gfx.drawRect(r, getBorderColor(), 1);
		}

		void ColorPicker::draw_checkerboard(BasicRenderer2D& gfx, const Rectangle& r)
		{
			const Color light(200, 200, 200);
			const Color dark(140, 140, 140);
			gfx.fillRect(r, light);
			const f32 cs = m_checkerSize;
			const i32 cols = static_cast<i32>(std::ceil(r.w / cs));
			const i32 rows = static_cast<i32>(std::ceil(r.h / cs));
			for (i32 y = 0; y < rows; ++y)
			{
				for (i32 x = 0; x < cols; ++x)
				{
					if (((x + y) & 1) == 0) continue;
					const f32 cx = r.x + x * cs;
					const f32 cy = r.y + y * cs;
					const f32 cw = std::min(cs, r.x + r.w - cx);
					const f32 ch = std::min(cs, r.y + r.h - cy);
					if (cw <= 0 || ch <= 0) continue;
					gfx.fillRect({ cx, cy, cw, ch }, dark);
				}
			}
		}

		void ColorPicker::draw_sv_cursor(BasicRenderer2D& gfx, const Rectangle& r)
		{
			const Vec2 c { r.x + m_sat * r.w, r.y + (1.0f - m_val) * r.h };
			// Double ring (white outer, black inner) — visible on any background.
			gfx.drawCircle(c, m_cursorRadius,        Colors::White, 2);
			gfx.drawCircle(c, m_cursorRadius - 2.0f, Colors::Black, 1);
		}

		void ColorPicker::draw_strip_cursor(BasicRenderer2D& gfx, const Rectangle& r, f32 t)
		{
			const f32 y = r.y + t * r.h;
			// Two short horizontal bars flanking the strip, plus a thin line
			// across it.
			const f32 bar = 4.0f;
			gfx.fillRect({ r.x - bar, y - 2.0f, bar, 4.0f }, Colors::White);
			gfx.fillRect({ r.x + r.w, y - 2.0f, bar, 4.0f }, Colors::White);
			gfx.drawRect({ r.x - bar - 1, y - 3, r.w + (bar + 1) * 2, 6 },
				Colors::Black, 1);
		}

		void ColorPicker::fire_changed(void)
		{
			if (callback_onColorChanged)
				callback_onColorChanged(getColor());
		}
	}
}
