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

#include <ostd/data/Object.hpp>
#include <ostd/string/String.hpp>
#include <type_traits>

struct SDL_GPURenderState;

namespace ogfx
{
	class BasicRenderer2D;

	// Precompiled fragment shader bytecode, one blob per GPU backend. Any of them may be
	// missing; the one the GPU device supports is picked at runtime (SPIR-V for Vulkan,
	// DXIL for D3D12, MSL for Metal). The data must stay alive while the Shader is loaded
	// (loadFromMemory() does not copy it; loadFromFiles() keeps its own copy).
	struct ShaderBytecode
	{
		const u8* spirv { nullptr };
		u32 spirvSize { 0 };
		const u8* msl { nullptr };
		u32 mslSize { 0 };
		const u8* dxil { nullptr };
		u32 dxilSize { 0 };
	};

	// Resources the shader declares (shadercross -d JSON prints them). The draw's texture
	// is sampler 0, so samplerCount is at least 1.
	struct ShaderLayout
	{
		u32 samplerCount { 1 };
		u32 uniformBufferCount { 0 };
		u32 storageTextureCount { 0 };
		u32 storageBufferCount { 0 };
	};

	// ============================================================================
	// Shader — a custom fragment shader for BasicRenderer2D.
	//
	//   ogfx::Shader shader;
	//   shader.loadFromFiles("res/shaders/wave.frag", { .samplerCount = 1, .uniformBufferCount = 1 });
	//   shader.setUniforms(myParams);               // a std140-laid-out struct, slot 0
	//   gfx.beginShader(shader);  ...draws...  gfx.endShader();   // or ogfx::ScopedShader
	//
	// Only the fragment stage is customizable: the vertex shader and vertex format are
	// SDL's. See shaders/blur.frag.hlsl for the interface a shader must follow (inputs,
	// texture and uniform slots, and why it must output texture * vertex color).
	//
	// Shaders need SDL's GPU renderer. When it isn't available (isSupported() is false,
	// e.g. old drivers), drawing with a shader warns once and draws unshaded.
	//
	// GPU objects are created lazily: the compiled shader once per GPU device (shared by
	// every window, see RenderContext), the SDL render state once per renderer that draws
	// with it. A Shader can be loaded before any window exists.
	// ============================================================================
	class Shader : public ostd::Object
	{
		public:
			inline Shader(void) { setTypeName("ogfx::Shader"); }
			~Shader(void);
			Shader(const Shader&) = delete;
			Shader& operator=(const Shader&) = delete;

			// True when shaders can be used (a GPU device exists for SDL's GPU renderer).
			static bool isSupported(void);

			bool loadFromMemory(const ShaderBytecode& bytecode, const ShaderLayout& layout = { });
			// Loads <basePath>.spv, <basePath>.msl and <basePath>.dxil (whichever exist).
			bool loadFromFiles(const String& basePath, const ShaderLayout& layout = { });
			void unload(void);
			inline bool isLoaded(void) const { return m_loaded; }
			inline const ShaderLayout& getLayout(void) const { return m_layout; }

			// Uniform data for a cbuffer slot. The struct must match the shader's cbuffer
			// layout, which follows std140-like packing rules: scalars are 4 bytes, float2 is
			// 8-aligned, float3/float4 are 16-aligned, a member never straddles a 16-byte
			// boundary, array elements take 16 bytes each, and the total is a multiple of 16.
			// Pad explicitly (see BlurUniforms in BasicRenderer.cpp).
			template<typename T>
			inline void setUniforms(const T& data, u32 slot = 0)
			{
				static_assert(std::is_trivially_copyable_v<T>, "Shader uniforms must be trivially copyable");
				static_assert(sizeof(T) % 16 == 0, "Shader uniform structs must be padded to a multiple of 16 bytes (std140)");
				static_assert(alignof(T) <= 16, "Shader uniform structs can't be over-aligned");
				setUniformData(slot, &data, cast<u32>(sizeof(T)));
			}
			// Low-level variant of setUniforms(). Takes effect for draws issued after the call.
			void setUniformData(u32 slot, const void* data, u32 size);

		private:
			// The per-renderer SDL render state, created on first use (nullptr when unsupported).
			SDL_GPURenderState* get_render_state(BasicRenderer2D& renderer);
			void warn_unsupported(void);
			void release_renderer(BasicRenderer2D& renderer);
			void upload_uniforms(SDL_GPURenderState* state) const;

		private:
			struct RendererState
			{
				BasicRenderer2D* renderer { nullptr };
				SDL_GPURenderState* state { nullptr };
			};

			ShaderBytecode m_bytecode;
			ShaderLayout m_layout;
			stdvec<u8> m_fileSpirv, m_fileMsl, m_fileDxil;
			stdvec<stdvec<u8>> m_uniforms;
			stdvec<RendererState> m_states;
			bool m_loaded { false };
			bool m_warnedUnsupported { false };

			friend class BasicRenderer2D;
	};

	// Activates a shader for the lifetime of the object.
	class ScopedShader
	{
		public:
			ScopedShader(BasicRenderer2D& renderer, Shader& shader);
			~ScopedShader(void);
			ScopedShader(const ScopedShader&) = delete;
			ScopedShader& operator=(const ScopedShader&) = delete;

		private:
			BasicRenderer2D& m_renderer;
	};
}
