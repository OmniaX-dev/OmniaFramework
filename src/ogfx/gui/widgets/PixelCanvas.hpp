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
#include <ogfx/resources/Image.hpp>
#include <ogfx/render/RenderTarget.hpp>
#include <ogfx/render/PostEffects.hpp>
#include <array>
#include <chrono>
#include <functional>
#include <memory>

namespace ogfx
{
	class PixelFont;

	namespace gui
	{
		// ============================================================================
		// PixelCanvas — a fixed-resolution pixel buffer, shown as a widget.
		//
		//   PixelCanvas canvas { window, 320, 200 };          // the canvas resolution
		//   canvas.setUpdateCallback([](PixelCanvas& c, f64 dt) {
		//       c.clear(Colors::Black);
		//       c.fillCircle({ 160, 100 }, 40, Colors::Orange);
		//       c.drawString("Hello", 4, 4, Colors::White);   // needs setFont()
		//   });
		//
		// Resolution: everything is drawn in canvas pixels (0..width, 0..height), whatever
		// size the widget has in the layout. The canvas is scaled to the widget's content
		// area, nearest-neighbour by default, keeping its aspect ratio (letterboxed) unless
		// that is disabled; integer scaling keeps every canvas pixel the same size.
		//
		// Frame rate: the canvas runs at its own fixed rate (30 fps by default), independent
		// from the GUI's: onCanvasUpdate() and the update callback run at that rate, and only
		// then are the pixels that changed uploaded to the GPU. The GUI keeps drawing the last
		// uploaded frame in between. Drawing functions can be called at any time.
		//
		// Pixels are 32-bit 0xAARRGGBB, straight alpha. Colors with alpha < 255 are blended
		// over what is already there; opaque ones overwrite (faster). Raw access:
		// getPixels() (width * height pixels, row by row).
		//
		// Effects: an optional CRT monitor look (ogfx::CRTShader) and bloom (ogfx::BloomEffect),
		// applied when the canvas is drawn; both need shader support (areEffectsSupported()),
		// and are skipped without it.
		// ============================================================================
		class PixelCanvas : public Widget
		{
			public: class ColorPalette256
			{
				public:
					// Starts as the default palette (the 256 xterm colors, index 0 transparent).
					ColorPalette256(void);
					// Index 0 is always transparent: setting it is ignored.
					void setColor(u8 index, const Color& color);
					Color getColor(u8 index) const;
					// Back to the default colors.
					void reset(void);
					// The color as a 0xAARRGGBB pixel.
					inline u32 operator[](u8 index) const { return m_colors[index]; }
					// Incremented by every change (lets users of the palette notice changes).
					inline u32 getVersion(void) const { return m_version; }

				private:
					std::array<u32, 256> m_colors;
					u32 m_version { 0 };
			};
			public: using UpdateCallback = std::function<void(PixelCanvas& canvas, f64 deltaTime)>;

			public:
				inline PixelCanvas(Window& window, i32 width = DefaultWidth, i32 height = DefaultHeight) : Widget({ 0, 0, 0, 0 }, window) { create(width, height); }
				virtual ~PixelCanvas(void);
				PixelCanvas& create(i32 width, i32 height);
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;

				// ================================== SETTINGS =================================
				// Resizes the pixel buffer (cleared to transparent).
				void setResolution(i32 width, i32 height);
				inline IPoint getResolution(void) const { return { m_width, m_height }; }
				inline i32 getCanvasWidth(void) const { return m_width; }
				inline i32 getCanvasHeight(void) const { return m_height; }
				void setFrameRate(f32 fps);
				inline f32 getFrameRate(void) const { return m_frameRate; }
				// Called at the canvas frame rate, after onCanvasUpdate().
				inline void setUpdateCallback(UpdateCallback callback) { m_updateCallback = std::move(callback); }
				// Font used by drawString() (and by TextConsoleCanvas). Not owned: it must outlive
				// its use here.
				void setFont(const PixelFont& font);
				inline const PixelFont* getFont(void) const { return m_font; }
				// Bilinear instead of nearest-neighbour scaling (with the CRT effect on, the shader
				// does its own sharp filtering and this has no effect).
				void setSmoothScaling(bool smooth);
				inline bool isSmoothScaling(void) const { return m_smoothScaling; }
				OSTD_BOOL_PARAM_GETSET_E(MaintainAspectRatio, m_maintainAspectRatio);
				OSTD_BOOL_PARAM_GETSET_E(IntegerScaling, m_integerScaling);
				// Fills the content area around the canvas (and behind transparent pixels).
				OSTD_PARAM_GETSET(Color, LetterboxColor, m_letterboxColor);
				// Where the canvas is drawn (window coordinates, logical units).
				Rectangle getCanvasDisplayBounds(void) const;
				// Window position (e.g. the mouse) -> canvas pixel. False when outside the canvas.
				bool mapToCanvas(const Vec2& windowPosition, IPoint& outPixel) const;
				// Canvas frames since creation, and the measured canvas frame rate.
				inline u64 getCanvasFrameCount(void) const { return m_frameCount; }
				inline f32 getMeasuredFrameRate(void) const { return m_measuredFrameRate; }
				// ==========================================================================



				// ================================== EFFECTS =================================
				bool areEffectsSupported(void) const;
				void enableCRTEffect(bool enable = true);
				inline bool isCRTEffectEnabled(void) const { return m_crtEnabled; }
				void setCRTParams(const CRTShader::Params& params);
				CRTShader::Params getCRTParams(void) const;
				void enableBloomEffect(bool enable = true);
				inline bool isBloomEffectEnabled(void) const { return m_bloomEnabled; }
				void setBloomParams(const BloomEffect::Params& params);
				BloomEffect::Params getBloomParams(void) const;
				// ==========================================================================



				// ================================== RAW PIXELS =================================
				// The pixel buffer (width * height 0xAARRGGBB pixels, row by row), for reading and
				// writing. With markDirty the whole buffer is uploaded on the next canvas frame;
				// pass false and call markDirty(area) to upload only what changed.
				// nullptr while drawing is disabled (TextConsoleCanvas in text mode).
				u32* getPixels(bool markDirty = true);
				const u32* getPixels(void) const;
				// Marks an area (all of the canvas when empty) to be uploaded on the next frame.
				void markDirty(const IRect& area = { 0, 0, 0, 0 });
				static constexpr u32 toPixel(u8 r, u8 g, u8 b, u8 a = 255) { return (cast<u32>(a) << 24) | (cast<u32>(r) << 16) | (cast<u32>(g) << 8) | cast<u32>(b); }
				static u32 toPixel(const Color& color);
				static Color toColor(u32 pixel);
				// ==========================================================================



				// ================================== DRAWING =================================
				// Coordinates are canvas pixels. Everything is clipped to the canvas. Thickness grows
				// inwards for rectangles, ellipses and circles, and is centered on lines.
				void clear(const Color& color = Colors::Transparent);
				void setPixel(i32 x, i32 y, const Color& color);
				Color getPixel(i32 x, i32 y) const;
				// Copies a w x h block of 0xAARRGGBB pixels (blended unless `blend` is false).
				void blit(const u32* pixels, i32 width, i32 height, i32 x, i32 y, bool blend = true);

				void drawLine(const IPoint& p1, const IPoint& p2, const Color& color, i32 thickness = 1, bool rounded = false);
				void drawRect(const IRect& rect, const Color& color, i32 thickness = 1);
				void drawRoundRect(const IRect& rect, const Color& color, i32 radius, i32 thickness = 1);
				void drawCircle(const IPoint& center, i32 radius, const Color& color, i32 thickness = 1);
				void drawEllipse(const IRect& rect, const Color& color, i32 thickness = 1);
				void drawTriangle(const IPoint& a, const IPoint& b, const IPoint& c, const Color& color, i32 thickness = 1);

				void fillRect(const IRect& rect, const Color& color);
				void fillRoundRect(const IRect& rect, const Color& color, i32 radius);
				void fillCircle(const IPoint& center, i32 radius, const Color& color);
				void fillEllipse(const IRect& rect, const Color& color);
				void fillTriangle(const IPoint& a, const IPoint& b, const IPoint& c, const Color& color);

				void outlinedRect(const IRect& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);
				void outlinedRoundRect(const IRect& rect, const Color& fillColor, const Color& outlineColor, i32 radius, i32 outlineThickness = 1);
				void outlinedCircle(const IPoint& center, i32 radius, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);
				void outlinedEllipse(const IRect& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);
				void outlinedTriangle(const IPoint& a, const IPoint& b, const IPoint& c, const Color& fillColor, const Color& outlineColor, i32 outlineThickness = 1);

				// Text with the canvas font (setFont()), `scale` times the font's cell size. '\n'
				// starts a new line at x. Characters the font doesn't have are left blank. A
				// transparent background leaves the pixels behind the glyphs untouched.
				void drawString(const String& text, i32 x, i32 y, const Color& color, const Color& background = Colors::Transparent, i32 scale = 1);
				void drawCharacter(u8 character, i32 x, i32 y, const Color& color, const Color& background = Colors::Transparent, i32 scale = 1);
				IPoint getStringSize(const String& text, i32 scale = 1) const;
				// ==========================================================================

			protected:
				// Called at the canvas frame rate (before the update callback).
				inline virtual void onCanvasUpdate(f64 deltaTime) {  }
				// Internal hook, after the updates and before the upload (TextConsoleCanvas renders here).
				inline virtual void render_canvas_frame(void) {  }

				// ===== Helpers for subclasses =====
				inline bool can_draw(void) const { return !m_drawingDisabled && !m_pixels.empty(); }
				inline void expand_dirty(i32 x0, i32 y0, i32 x1, i32 y1)
				{
					m_dirtyX0 = std::min(m_dirtyX0, x0); m_dirtyY0 = std::min(m_dirtyY0, y0);
					m_dirtyX1 = std::max(m_dirtyX1, x1); m_dirtyY1 = std::max(m_dirtyY1, y1);
				}
				inline void mark_all_dirty(void) { expand_dirty(0, 0, m_width, m_height); }
				// Fills [x0, x1) of row y (clipped), blending when the color isn't opaque.
				void fill_span(i32 y, i32 x0, i32 x1, u32 color);

			private:
				void tick(f64 deltaTime);
				void reset_dirty(void);
				void upload(void);
				bool ensure_image(BasicRenderer2D& gfx);
				Rectangle compute_display_bounds(void) const;
				// Pixel coverage of a rectangle with elliptical corners at the row whose center is
				// cy: the span [x0, x1) of pixels whose centers are inside. False if none.
				bool rounded_span(f32 x, f32 y, f32 w, f32 h, f32 rx, f32 ry, f32 cy, i32& x0, i32& x1) const;
				void fill_rounded(f32 x, f32 y, f32 w, f32 h, f32 rx, f32 ry, u32 color);
				void ring_rounded(f32 x, f32 y, f32 w, f32 h, f32 rx, f32 ry, f32 thickness, u32 color);
				void fill_polygon(const Vec2* points, i32 count, u32 color);
				void thin_line(i32 x1, i32 y1, i32 x2, i32 y2, u32 color);
				void thick_line(const Vec2& p1, const Vec2& p2, f32 thickness, bool rounded, u32 color);
				void put_pixel(i32 x, i32 y, u32 color);
				void draw_glyph(u8 character, i32 x, i32 y, i32 cellW, i32 cellH, u32 color, u32 background);

			public:
				inline static constexpr i32 DefaultWidth { 640 };
				inline static constexpr i32 DefaultHeight { 480 };
				inline static constexpr f32 DefaultFrameRate { 30.0f };
				// A GUI frame this close to the canvas interval still runs the canvas frame.
				inline static constexpr f64 FrameTolerance { 0.002 };
				static const ColorPalette256 DefaultPalette;

			protected:
				stdvec<u32> m_pixels;
				i32 m_width { 0 };
				i32 m_height { 0 };
				bool m_drawingDisabled { false };

			private:
				using Clock = std::chrono::steady_clock;

				i32 m_dirtyX0 { 0 };
				i32 m_dirtyY0 { 0 };
				i32 m_dirtyX1 { 0 };
				i32 m_dirtyY1 { 0 };

				Image m_image;
				const PixelFont* m_font { nullptr };
				UpdateCallback m_updateCallback { nullptr };
				f32 m_frameRate { DefaultFrameRate };
				f64 m_frameInterval { 1.0 / DefaultFrameRate };
				Clock::time_point m_lastTick;
				Clock::time_point m_nextTick;
				Clock::time_point m_startTime;
				bool m_ticked { false };
				u64 m_frameCount { 0 };
				f32 m_measuredFrameRate { 0.0f };
				f64 m_rateWindow { 0.0 };
				u32 m_rateFrames { 0 };

				bool m_smoothScaling { false };
				bool m_maintainAspectRatio { true };
				bool m_integerScaling { false };
				Color m_letterboxColor { 0, 0, 0, 255 };

				bool m_crtEnabled { false };
				bool m_bloomEnabled { false };
				std::unique_ptr<CRTShader> m_crt;
				std::unique_ptr<BloomEffect> m_bloom;
				CRTShader::Params m_crtParams;
				BloomEffect::Params m_bloomParams;
				RenderTarget m_effectTarget;
		};
	}
}
