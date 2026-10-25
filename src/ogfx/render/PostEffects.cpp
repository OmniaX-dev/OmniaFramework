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

#include "PostEffects.hpp"
#include "BasicRenderer.hpp"
#include "../resources/EmbeddedShaders.hpp"

namespace ogfx
{
	namespace
	{
		// Must match the cbuffers in other/shaders/crt.frag.hlsl and bloom.frag.hlsl (std140).
		struct CRTUniforms
		{
			f32 sourceSize[2];
			f32 outputSize[2];
			f32 curvature;
			f32 scanlines;
			f32 mask;
			f32 vignette;
			f32 chromatic;
			f32 flicker;
			f32 noise;
			f32 brightness;
			f32 time;
			f32 cornerRadius;
			f32 padding[2];
		};
		static_assert(sizeof(CRTUniforms) == 64);

		struct BloomUniforms
		{
			f32 threshold;
			f32 intensity;
			f32 mode;
			f32 padding;
		};
		static_assert(sizeof(BloomUniforms) == 16);
	}





	// ===================================================== CRT SHADER =====================================================
	CRTShader::CRTShader(void)
	{
		setTypeName("ogfx::CRTShader");
		loadFromMemory(OGFX_EMBEDDED_SHADER_BYTECODE(crt_frag), OGFX_EMBEDDED_SHADER_LAYOUT(crt_frag));
	}

	void CRTShader::prepare(const Vec2& imageSize, const Vec2& outputPixelSize, f32 time)
	{
		CRTUniforms u { };
		u.sourceSize[0] = std::max(imageSize.x, 1.0f);
		u.sourceSize[1] = std::max(imageSize.y, 1.0f);
		u.outputSize[0] = std::max(outputPixelSize.x, 1.0f);
		u.outputSize[1] = std::max(outputPixelSize.y, 1.0f);
		u.curvature = m_params.curvature;
		u.scanlines = std::clamp(m_params.scanlines, 0.0f, 1.0f);
		u.mask = std::clamp(m_params.mask, 0.0f, 1.0f);
		u.vignette = std::clamp(m_params.vignette, 0.0f, 1.0f);
		u.chromatic = m_params.chromaticAberration;
		u.flicker = std::clamp(m_params.flicker, 0.0f, 1.0f);
		u.noise = std::clamp(m_params.noise, 0.0f, 1.0f);
		u.brightness = m_params.brightness;
		// Wrapped, so the float keeps its precision in long sessions (the shader only needs
		// it for periodic effects).
		u.time = std::fmod(time, 1000.0f);
		u.cornerRadius = std::clamp(m_params.cornerRadius, 0.0f, 0.5f);
		setUniforms(u);
	}
	// ===================================================== CRT SHADER =====================================================





	// ===================================================== BLOOM EFFECT =====================================================
	BloomEffect::BloomEffect(void)
	{
		// One Shader per mode, so the two passes don't update (and flush) uniforms between them.
		m_extract.loadFromMemory(OGFX_EMBEDDED_SHADER_BYTECODE(bloom_frag), OGFX_EMBEDDED_SHADER_LAYOUT(bloom_frag));
		m_composite.loadFromMemory(OGFX_EMBEDDED_SHADER_BYTECODE(bloom_frag), OGFX_EMBEDDED_SHADER_LAYOUT(bloom_frag));
	}

	void BloomEffect::draw(BasicRenderer2D& gfx, const RenderTarget& scene, const Vec2& position, const Vec2& size)
	{
		if (!gfx.areShadersSupported() || !m_extract.isLoaded() || !m_composite.isLoaded())
			return;
		const Vec2 sceneSize = scene.getSize();
		if (sceneSize.x <= 0 || sceneSize.y <= 0 || !scene.hasTexture())
			return;
		const f32 resolution = std::clamp(m_params.resolution, 0.1f, 1.0f);
		const Vec2 glowSize { std::max(1.0f, std::round(sceneSize.x * resolution)), std::max(1.0f, std::round(sceneSize.y * resolution)) };
		if (m_bright.getRenderer() != &gfx)
		{
			m_bright.create(gfx, glowSize);
			m_glow.create(gfx, glowSize);
		}
		else
		{
			m_bright.resize(glowSize);
		}

		// 1. Bright parts, at the glow resolution.
		m_extract.setUniforms(BloomUniforms { m_params.threshold, 1.0f, 0.0f, 0.0f });
		if (!gfx.beginRenderTarget(m_bright))
			return;
		gfx.beginShader(m_extract);
		gfx.drawRenderTarget(scene, { 0, 0 }, glowSize);
		gfx.endShader();
		gfx.endRenderTarget();

		// 2. Blurred (the radius is relative to the scene, the glow target is smaller).
		if (!gfx.blurRenderTarget(m_bright, m_glow, std::max(m_params.radius, 0.0f) * resolution))
			return;

		// 3. Added over the scene.
		m_composite.setUniforms(BloomUniforms { m_params.threshold, std::max(m_params.intensity, 0.0f), 1.0f, 0.0f });
		gfx.beginShader(m_composite);
		gfx.drawRenderTarget(m_glow, position, (size.x > 0 && size.y > 0) ? size : sceneSize);
		gfx.endShader();
	}
	// ===================================================== BLOOM EFFECT =====================================================
}
