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

#include <ogfx/render/Shader.hpp>
#include <ogfx/render/RenderTarget.hpp>
#include <ostd/math/Geometry.hpp>

namespace ogfx
{
	class BasicRenderer2D;

	// ============================================================================
	// CRTShader — the built-in CRT monitor shader (other/shaders/crt.frag.hlsl).
	//
	//   ogfx::CRTShader crt;
	//   crt.prepare(imageSize, drawnSizeInPixels, timeSeconds);
	//   gfx.beginShader(crt);
	//       gfx.drawImage(image, position, size);   // the whole image, stretched
	//   gfx.endShader();
	//
	// Meant for a low-resolution image drawn over a larger area (gui::PixelCanvas uses it
	// for its CRT effect): curved glass, scanlines between the image's rows, an aperture
	// grille, vignette, convergence error, flicker and noise. The texture coordinates must
	// cover the whole image, and the image should use smooth filtering (the shader keeps the
	// pixels crisp itself). Needs shader support (see Shader::isSupported()).
	// ============================================================================
	class CRTShader : public Shader
	{
		public: struct Params
		{
			f32 curvature { 0.028f };         // barrel distortion, 0 = flat
			f32 scanlines { 0.45f };          // 0..1, darkening between rows
			f32 mask { 0.2f };                // 0..1, RGB aperture grille
			f32 vignette { 0.4f };            // 0..1
			f32 chromaticAberration { 0.5f }; // in image pixels
			f32 flicker { 0.015f };           // 0..1
			f32 noise { 0.025f };             // 0..1
			f32 brightness { 1.15f };         // overall gain, compensates for the darkening
			f32 cornerRadius { 0.04f };       // rounded screen corners (fraction of the size)
		};

		public:
			CRTShader(void);
			inline void setParams(const Params& params) { m_params = params; }
			inline const Params& getParams(void) const { return m_params; }
			// Sets the uniforms for the next draw: the image's size in pixels, the size it is
			// drawn at in screen pixels (logical size * BasicRenderer2D::getPixelScale()) and
			// the time in seconds (animates flicker and noise).
			void prepare(const Vec2& imageSize, const Vec2& outputPixelSize, f32 time);

		private:
			Params m_params;
	};

	// ============================================================================
	// BloomEffect — glow around the bright parts of a render target.
	//
	//   gfx.drawRenderTarget(scene, position, size);   // the scene itself
	//   bloom.draw(gfx, scene, position, size);        // + its glow, added on top
	//
	// The bright parts (above `threshold`) are extracted into a smaller internal target,
	// blurred (BasicRenderer2D::blurRenderTarget()) and added over what is already drawn.
	// The internal targets are kept between frames and follow the scene's size. Without
	// shader support draw() does nothing.
	// ============================================================================
	class BloomEffect
	{
		public: struct Params
		{
			f32 threshold { 0.35f };   // 0..1, brightness where the glow starts
			f32 intensity { 0.9f };    // glow strength
			f32 radius { 8.0f };       // blur reach, in the scene's logical units
			f32 resolution { 0.5f };   // size of the glow targets relative to the scene (0.1..1)
		};

		public:
			BloomEffect(void);
			BloomEffect(const BloomEffect&) = delete;
			BloomEffect& operator=(const BloomEffect&) = delete;
			inline void setParams(const Params& params) { m_params = params; }
			inline const Params& getParams(void) const { return m_params; }
			// size {0,0} = the scene's logical size.
			void draw(BasicRenderer2D& gfx, const RenderTarget& scene, const Vec2& position, const Vec2& size = { 0, 0 });

		private:
			Params m_params;
			Shader m_extract;
			Shader m_composite;
			RenderTarget m_bright;
			RenderTarget m_glow;
	};
}
