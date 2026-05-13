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

#pragma once

#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/widgets/Text.hpp>

namespace ogfx
{
	namespace gui
	{
		// ===========================================================================
		// ColorPicker — inline HSV/alpha picker.
		//
		// Layout (left to right inside content bounds):
		//
		//   [  SV square  ][ hue strip ][ alpha strip ][ swatch + hex field ]
		//
		// The SV square's hue is driven by the hue strip. The strips are thin
		// vertical bars. The swatch shows the current color over a checkerboard
		// (so alpha is visible). The hex field is a child TextEdit that round-
		// trips with the picker state.
		//
		// State is stored as HSV+alpha (not RGB). This preserves hue and
		// saturation as the user drags value to zero or saturation to zero —
		// otherwise the SV/hue cursors would jump every time the color goes
		// grayscale.
		// ===========================================================================
		class ColorPicker : public Widget
		{
			public:
				inline ColorPicker(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				ColorPicker& create(void);

				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMousePressed(const Event& event) override;
				void onMouseDragged(const Event& event) override;
				void onMouseReleased(const Event& event) override;

				// Round-tripped public API. setColor preserves hue when the
				// incoming color is grayscale (s == 0).
				Color getColor(void) const;
				void setColor(const Color& c);

				// Component access for callers that want HSV directly.
				inline f32 getHue(void) const { return m_hue; }
				inline f32 getSaturation(void) const { return m_sat; }
				inline f32 getValue(void) const { return m_val; }
				inline f32 getAlpha(void) const { return m_alpha; }
				void setHSV(f32 h, f32 s, f32 v);
				void setAlpha(f32 a);

				inline void setColorChangedCallback(std::function<void(const Color&)> cb)
				{ callback_onColorChanged = std::move(cb); }

				// Show/hide the alpha strip. Default: true.
				OSTD_BOOL_PARAM_GETSET_E(AlphaChannel, m_showAlpha);
				// Show/hide the hex input field. Default: true.
				OSTD_BOOL_PARAM_GETSET_E(ColorPreview, m_showPreview);
				// SV cursor radius (the little ring on the square).
				OSTD_PARAM_GETSET(f32, CursorRadius, m_cursorRadius);
				// Width of each side strip (hue/alpha).
				OSTD_PARAM_GETSET(f32, StripWidth, m_stripWidth);
				// Gap between regions.
				OSTD_PARAM_GETSET(f32, RegionSpacing, m_regionSpacing);
				// Size of the right-hand swatch + hex field column.
				OSTD_PARAM_GETSET(f32, InfoColumnWidth, m_infoColumnW);
				// Checkerboard cell size used for alpha visualization.
				OSTD_PARAM_GETSET(f32, CheckerSize, m_checkerSize);
				// Number of stripes used to draw the SV/alpha gradients
				// (the renderer's fillGradientRect is 1D, so we hand-draw the
				// SV square as N horizontal stripes that each fade their own
				// vertical gradient). 32-48 looks smooth and is cheap.
				OSTD_PARAM_GETSET(i32, GradientSteps, m_gradientSteps);

			private:
				enum class Region { None, SV, Hue, Alpha };

				// Geometry helpers — all return GLOBAL coordinates so they
				// match what the mouse events give us.
				Rectangle sv_bounds(void) const;
				Rectangle hue_bounds(void) const;
				Rectangle alpha_bounds(void) const;
				Rectangle swatch_bounds(void) const;

				Region hit_test(const Vec2& globalMouse) const;
				void update_from_mouse(Region region, const Vec2& globalMouse);

				// Drawing helpers.
				void draw_sv_square(BasicRenderer2D& gfx, const Rectangle& r);
				void draw_hue_strip(BasicRenderer2D& gfx, const Rectangle& r);
				void draw_alpha_strip(BasicRenderer2D& gfx, const Rectangle& r);
				void draw_checkerboard(BasicRenderer2D& gfx, const Rectangle& r);
				void draw_sv_cursor(BasicRenderer2D& gfx, const Rectangle& r);
				void draw_strip_cursor(BasicRenderer2D& gfx, const Rectangle& r, f32 t);

				// HSV state propagation.
				void fire_changed(void);

				Color current_pure_hue(void) const; // HSV(h, 1, 1)

			private:
				f32 m_hue { 0.0f };   // [0, 1]
				f32 m_sat { 1.0f };   // [0, 1]
				f32 m_val { 1.0f };   // [0, 1]
				f32 m_alpha { 1.0f }; // [0, 1]

				Region m_activeDrag { Region::None };

				bool m_showAlpha { true };
				bool m_showPreview { true };
				f32 m_cursorRadius { 6.0f };
				f32 m_stripWidth { 18.0f };
				f32 m_regionSpacing { 8.0f };
				f32 m_infoColumnW { 96.0f };
				f32 m_checkerSize { 6.0f };
				i32 m_gradientSteps { 32 };

				std::function<void(const Color&)> callback_onColorChanged { nullptr };
		};
	}
}
