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
#include <ostd/io/Logger.hpp>

namespace
{
	// Shader formats the framework ships built-in shader blobs for.
	constexpr SDL_GPUShaderFormat EmbeddedShaderFormats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
}

namespace ogfx
{
	RenderContext& RenderContext::get(void)
	{
		static RenderContext s_context;
		return s_context;
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
		SDL_DestroyGPUDevice(m_device);
		m_device = nullptr;
		m_deviceCreationFailed = false;
	}
}
