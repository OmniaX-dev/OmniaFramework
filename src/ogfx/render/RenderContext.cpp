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

#include "RenderContext.hpp"
#include "BasicRenderer.hpp"
#include "Shader.hpp"
#include "../resources/EmbeddedShaders.hpp"
#include <ostd/io/Logger.hpp>

namespace
{
	// Shader formats the framework ships built-in shader blobs for (see shaders/build_shaders.py).
	constexpr SDL_GPUShaderFormat EmbeddedShaderFormats =
		(ogfx::embedded_shaders::HasFormat_SPV ? SDL_GPU_SHADERFORMAT_SPIRV : 0) |
		(ogfx::embedded_shaders::HasFormat_DXIL ? SDL_GPU_SHADERFORMAT_DXIL : 0) |
		(ogfx::embedded_shaders::HasFormat_MSL ? SDL_GPU_SHADERFORMAT_MSL : 0);

	u64 fnv1a(const u8* data, u32 size, u64 hash = 14695981039346656037ull)
	{
		for (u32 i = 0; i < size; i++)
		{
			hash ^= data[i];
			hash *= 1099511628211ull;
		}
		return hash;
	}
}

namespace ogfx
{
	RenderContext& RenderContext::get(void)
	{
		// Never destroyed: windows (and their renderers) may be static objects themselves, and
		// static objects are destroyed in reverse order of construction. This one is created
		// by the first window's initialize(), after such a window, so as a plain static it would
		// be gone by the time that window's destructor releases its renderer. The GPU device
		// is still released on BeforeSDLShutdown, when the last window shuts SDL down.
		static RenderContext* s_context = new RenderContext();
		return *s_context;
	}

	RenderContext::RenderContext(void)
	{
		setTypeName("ogfx::RenderContext");
		enableSignals();
		connectSignal(ostd::BuiltinSignals::BeforeSDLShutdown);
		validate();
	}

	SDL_Renderer* RenderContext::createRenderer(SDL_Window* window)
	{
		if (!window)
			return nullptr;
		// OGFX_DISABLE_GPU_RENDERER=1 forces the fallback path (useful to test it).
		const char* disableGPU = SDL_getenv("OGFX_DISABLE_GPU_RENDERER");
		const bool gpuDisabled = disableGPU && SDL_strcmp(disableGPU, "0") != 0;
		if (!gpuDisabled && create_device())
		{
			SDL_Renderer* renderer = SDL_CreateGPURenderer(m_device, window);
			if (renderer)
				return renderer;
			OX_WARN("ogfx: SDL_CreateGPURenderer failed (%s); falling back to the default renderer, shaders are disabled.", SDL_GetError());
		}
		return SDL_CreateRenderer(window, nullptr);
	}

	void RenderContext::releaseRenderer(SDL_Renderer* renderer)
	{
		if (!renderer)
			return;
		for (auto* r : m_renderers)
		{
			if (r->getSDLRenderer() == renderer)
				r->__release_device_resources();
		}
	}

	void RenderContext::registerRenderer(BasicRenderer2D& renderer)
	{
		if (std::find(m_renderers.begin(), m_renderers.end(), &renderer) == m_renderers.end())
			m_renderers.push_back(&renderer);
	}

	void RenderContext::unregisterRenderer(BasicRenderer2D& renderer)
	{
		std::erase(m_renderers, &renderer);
	}

	void RenderContext::dispatchRenderEvent(const SDL_Event& event)
	{
		if (event.type != SDL_EVENT_RENDER_TARGETS_RESET && event.type != SDL_EVENT_RENDER_DEVICE_RESET && event.type != SDL_EVENT_RENDER_DEVICE_LOST)
			return;
		// Copy: a handler could (in principle) create or destroy renderers.
		const auto renderers = m_renderers;
		for (auto* r : renderers)
		{
			if (r->getWindowID() == event.render.windowID)
				r->__on_render_event(event.type);
		}
	}

	SDL_GPUShaderFormat RenderContext::getShaderFormats(void) const
	{
		return m_device ? SDL_GetGPUShaderFormats(m_device) : SDL_GPU_SHADERFORMAT_INVALID;
	}

	SDL_GPUShader* RenderContext::acquireFragmentShader(SDL_GPUDevice* device, const ShaderBytecode& bytecode, const ShaderLayout& layout)
	{
		if (!device)
			return nullptr;
		const SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
		SDL_GPUShaderCreateInfo info;
		SDL_zero(info);
		if ((formats & SDL_GPU_SHADERFORMAT_SPIRV) && bytecode.spirv && bytecode.spirvSize > 0)
		{
			info.format = SDL_GPU_SHADERFORMAT_SPIRV;
			info.code = bytecode.spirv;
			info.code_size = bytecode.spirvSize;
			info.entrypoint = "main";
		}
		else if ((formats & SDL_GPU_SHADERFORMAT_DXIL) && bytecode.dxil && bytecode.dxilSize > 0)
		{
			info.format = SDL_GPU_SHADERFORMAT_DXIL;
			info.code = bytecode.dxil;
			info.code_size = bytecode.dxilSize;
			info.entrypoint = "main";
		}
		else if ((formats & SDL_GPU_SHADERFORMAT_MSL) && bytecode.msl && bytecode.mslSize > 0)
		{
			info.format = SDL_GPU_SHADERFORMAT_MSL;
			info.code = bytecode.msl;
			info.code_size = bytecode.mslSize;
			info.entrypoint = "main0"; // what SDL_shadercross / SPIRV-Cross name the MSL entry point
		}
		else
		{
			OX_ERROR("ogfx::Shader: no bytecode in a format the GPU device supports (driver: %s).", SDL_GetGPUDeviceDriver(device));
			return nullptr;
		}
		info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
		info.num_samplers = layout.samplerCount;
		info.num_uniform_buffers = layout.uniformBufferCount;
		info.num_storage_textures = layout.storageTextureCount;
		info.num_storage_buffers = layout.storageBufferCount;

		u64 key = fnv1a(cast<const u8*>(info.code), cast<u32>(info.code_size));
		const u32 meta[6] = { cast<u32>(info.format), layout.samplerCount, layout.uniformBufferCount, layout.storageTextureCount, layout.storageBufferCount, cast<u32>(info.code_size) };
		key = fnv1a(cast<const u8*>(cast<const void*>(meta)), sizeof(meta), key);
		key ^= cast<u64>(reinterpret_cast<uintptr_t>(device));
		auto it = m_shaderCache.find(key);
		if (it != m_shaderCache.end())
			return it->second;

		SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
		if (!shader)
		{
			OX_ERROR("ogfx::Shader: SDL_CreateGPUShader failed: %s", SDL_GetError());
			return nullptr;
		}
		m_shaderCache[key] = shader;
		return shader;
	}

	void RenderContext::handleSignal(ostd::Signal& signal)
	{
		if (signal.ID == ostd::BuiltinSignals::BeforeSDLShutdown)
			destroy_device();
	}

	bool RenderContext::create_device(void)
	{
		if (m_device)
			return true;
		if (m_deviceCreationFailed)
			return false;

		SDL_PropertiesID props = SDL_CreateProperties();
		// Only the formats we ship built-in shader blobs for: on Windows, without DXIL blobs,
		// this makes SDL pick the Vulkan backend instead of D3D12.
		const SDL_GPUShaderFormat formats = EmbeddedShaderFormats;
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, (formats & SDL_GPU_SHADERFORMAT_SPIRV) != 0);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_DXIL_BOOLEAN, (formats & SDL_GPU_SHADERFORMAT_DXIL) != 0);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_MSL_BOOLEAN, (formats & SDL_GPU_SHADERFORMAT_MSL) != 0);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, SDL_GetHintBoolean(SDL_HINT_RENDER_GPU_DEBUG, false));
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_PREFERLOWPOWER_BOOLEAN, SDL_GetHintBoolean(SDL_HINT_RENDER_GPU_LOW_POWER, false));
		// The same compatibility settings SDL's GPU renderer uses when it creates its own device.
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_D3D12_ALLOW_FEWER_RESOURCE_SLOTS_BOOLEAN, true);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_CLIP_DISTANCE_BOOLEAN, false);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_DEPTH_CLAMPING_BOOLEAN, false);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_INDIRECT_DRAW_FIRST_INSTANCE_BOOLEAN, false);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_ANISOTROPY_BOOLEAN, false);
		SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_METAL_ALLOW_MACFAMILY1_BOOLEAN, false);
		m_device = SDL_CreateGPUDeviceWithProperties(props);
		SDL_DestroyProperties(props);

		if (!m_device)
		{
			m_deviceCreationFailed = true;
			OX_WARN("ogfx: no GPU device available (%s); using the default renderer, shaders are disabled.", SDL_GetError());
			return false;
		}
		return true;
	}

	void RenderContext::destroy_device(void)
	{
		if (!m_device)
			return;
		for (auto& [key, shader] : m_shaderCache)
			SDL_ReleaseGPUShader(m_device, shader);
		m_shaderCache.clear();
		SDL_DestroyGPUDevice(m_device);
		m_device = nullptr;
		m_deviceCreationFailed = false;
	}
}
