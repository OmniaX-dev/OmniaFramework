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

#include "Shader.hpp"
#include "BasicRenderer.hpp"
#include "RenderContext.hpp"
#include <ostd/io/Logger.hpp>
#include <fstream>
#include <cstring>

namespace
{
	bool read_file(const String& path, stdvec<u8>& out)
	{
		out.clear();
		std::ifstream file(path.cpp_str(), std::ios::binary | std::ios::ate);
		if (!file)
			return false;
		const std::streamsize size = file.tellg();
		if (size <= 0)
			return false;
		out.resize(cast<size_t>(size));
		file.seekg(0);
		return cast<bool>(file.read(reinterpret_cast<char*>(out.data()), size));
	}
}

namespace ogfx
{
	// ===================================================== SHADER =====================================================
	Shader::~Shader(void)
	{
		unload();
	}

	bool Shader::isSupported(void)
	{
		return RenderContext::get().isGPUAvailable();
	}

	bool Shader::loadFromMemory(const ShaderBytecode& bytecode, const ShaderLayout& layout)
	{
		unload();
		if ((!bytecode.spirv || bytecode.spirvSize == 0) && (!bytecode.msl || bytecode.mslSize == 0) && (!bytecode.dxil || bytecode.dxilSize == 0))
		{
			OX_ERROR("ogfx::Shader::loadFromMemory(): no bytecode given.");
			return false;
		}
		m_bytecode = bytecode;
		m_layout = layout;
		m_layout.samplerCount = std::max(m_layout.samplerCount, 1u);
		m_loaded = true;
		validate();
		return true;
	}

	bool Shader::loadFromFiles(const String& basePath, const ShaderLayout& layout)
	{
		unload();
		ShaderBytecode bytecode;
		if (read_file(String(basePath).add(".spv"), m_fileSpirv))
		{
			bytecode.spirv = m_fileSpirv.data();
			bytecode.spirvSize = cast<u32>(m_fileSpirv.size());
		}
		if (read_file(String(basePath).add(".msl"), m_fileMsl))
		{
			bytecode.msl = m_fileMsl.data();
			bytecode.mslSize = cast<u32>(m_fileMsl.size());
		}
		if (read_file(String(basePath).add(".dxil"), m_fileDxil))
		{
			bytecode.dxil = m_fileDxil.data();
			bytecode.dxilSize = cast<u32>(m_fileDxil.size());
		}
		if (!bytecode.spirv && !bytecode.msl && !bytecode.dxil)
		{
			OX_ERROR("ogfx::Shader::loadFromFiles(): no %s.spv/.msl/.dxil found.", basePath.c_str());
			return false;
		}
		// loadFromMemory() would unload() (and so clear the file buffers) first.
		m_bytecode = bytecode;
		m_layout = layout;
		m_layout.samplerCount = std::max(m_layout.samplerCount, 1u);
		m_loaded = true;
		validate();
		return true;
	}

	void Shader::unload(void)
	{
		const auto states = m_states;
		for (const auto& entry : states)
			release_renderer(*entry.renderer);
		m_states.clear();
		m_bytecode = { };
		m_fileSpirv.clear();
		m_fileMsl.clear();
		m_fileDxil.clear();
		m_uniforms.clear();
		m_loaded = false;
		invalidate();
	}

	void Shader::setUniformData(u32 slot, const void* data, u32 size)
	{
		if (!data || size == 0)
			return;
		if (slot >= m_uniforms.size())
			m_uniforms.resize(slot + 1);
		auto& stored = m_uniforms[slot];
		if (stored.size() == size && std::memcmp(stored.data(), data, size) == 0)
			return;
		// Geometry already batched with this shader was issued with the old values.
		for (const auto& entry : m_states)
		{
			if (entry.state)
				entry.renderer->flush_if_using(entry.state);
		}
		stored.assign(cast<const u8*>(data), cast<const u8*>(data) + size);
		for (const auto& entry : m_states)
		{
			if (entry.state)
				SDL_SetGPURenderStateFragmentUniforms(entry.state, slot, stored.data(), size);
		}
	}

	SDL_GPURenderState* Shader::get_render_state(BasicRenderer2D& renderer)
	{
		if (!m_loaded)
			return nullptr;
		for (const auto& entry : m_states)
		{
			if (entry.renderer == &renderer)
				return entry.state;
		}

		// First use with this renderer. A failure is remembered (as a null state) so it is
		// only reported once.
		SDL_GPURenderState* state = nullptr;
		SDL_Renderer* sdlRenderer = renderer.getSDLRenderer();
		SDL_GPUDevice* device = sdlRenderer ? SDL_GetGPURendererDevice(sdlRenderer) : nullptr;
		if (!device)
			warn_unsupported();
		else if (SDL_GPUShader* gpuShader = RenderContext::get().acquireFragmentShader(device, m_bytecode, m_layout))
		{
			SDL_GPURenderStateCreateInfo info;
			SDL_zero(info);
			info.fragment_shader = gpuShader;
			state = SDL_CreateGPURenderState(sdlRenderer, &info);
			if (!state)
				OX_ERROR("ogfx::Shader: SDL_CreateGPURenderState failed: %s", SDL_GetError());
			else
				upload_uniforms(state);
		}
		m_states.push_back({ &renderer, state });
		renderer.register_shader(*this);
		return state;
	}

	void Shader::warn_unsupported(void)
	{
		if (m_warnedUnsupported)
			return;
		m_warnedUnsupported = true;
		OX_WARN("ogfx::Shader: shaders need SDL's GPU renderer, which is not available; drawing unshaded.");
	}

	void Shader::release_renderer(BasicRenderer2D& renderer)
	{
		for (size_t i = 0; i < m_states.size(); i++)
		{
			if (m_states[i].renderer != &renderer)
				continue;
			SDL_GPURenderState* state = m_states[i].state;
			m_states.erase(m_states.begin() + i);
			renderer.unregister_shader(*this, state);
			// Without a live renderer the state went away with the SDL renderer.
			if (state && renderer.isInitialized())
				SDL_DestroyGPURenderState(state);
			return;
		}
	}

	void Shader::upload_uniforms(SDL_GPURenderState* state) const
	{
		for (u32 slot = 0; slot < m_uniforms.size(); slot++)
		{
			if (!m_uniforms[slot].empty())
				SDL_SetGPURenderStateFragmentUniforms(state, slot, m_uniforms[slot].data(), cast<u32>(m_uniforms[slot].size()));
		}
	}
	// ===================================================== SHADER =====================================================





	// ===================================================== SCOPED SHADER =====================================================
	ScopedShader::ScopedShader(BasicRenderer2D& renderer, Shader& shader) : m_renderer(renderer)
	{
		m_renderer.beginShader(shader);
	}

	ScopedShader::~ScopedShader(void)
	{
		m_renderer.endShader();
	}
	// ===================================================== SCOPED SHADER =====================================================
}
