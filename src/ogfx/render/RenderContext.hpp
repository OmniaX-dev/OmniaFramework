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
#include <ogfx/utils/SDLInclude.hpp>

namespace ogfx
{
	class BasicRenderer2D;

	// ============================================================================
	// RenderContext — framework-level rendering state shared by all windows.
	//
	//   - Owns the single SDL_GPUDevice that every window's GPU renderer is
	//     created on (SDL_CreateGPURenderer lets several renderers share one
	//     device), so compiled GPU shaders can be shared between windows. The
	//     device is created lazily with the first renderer and released on
	//     BeforeSDLShutdown, after the last window (and so the last renderer) is
	//     gone.
	//   - Falls back to SDL's default renderer when no GPU device can be
	//     created; shaders are then unsupported (see isGPUAvailable()).
	//   - Keeps track of every BasicRenderer2D, so render/device reset events
	//     (which arrive on the one global SDL event queue, polled by whichever
	//     window's loop runs) reach the renderers of the window they belong to.
	//
	// This is internal framework plumbing: application code normally never
	// needs to use it directly.
	// ============================================================================
	class RenderContext : public ostd::Object
	{
		public:
			static RenderContext& get(void);

			// Creates the SDL renderer for a window: a GPU renderer on the shared
			// device when possible, otherwise SDL's default renderer.
			SDL_Renderer* createRenderer(SDL_Window* window);
			// Must be called before SDL_DestroyRenderer(): lets every BasicRenderer2D
			// using that SDL renderer release its textures and GPU state first.
			void releaseRenderer(SDL_Renderer* renderer);

			void registerRenderer(BasicRenderer2D& renderer);
			void unregisterRenderer(BasicRenderer2D& renderer);
			// Routes SDL_EVENT_RENDER_* events to the renderers of event.render.windowID.
			void dispatchRenderEvent(const SDL_Event& event);

			inline bool isGPUAvailable(void) const { return m_device != nullptr; }
			inline SDL_GPUDevice* getGPUDevice(void) const { return m_device; }
			// Shader formats the shared device accepts (0 when there is no device).
			SDL_GPUShaderFormat getShaderFormats(void) const;
			void handleSignal(ostd::Signal& signal) override;

		private:
			RenderContext(void);
			bool create_device(void);
			void destroy_device(void);

		private:
			SDL_GPUDevice* m_device { nullptr };
			bool m_deviceCreationFailed { false };
			stdvec<BasicRenderer2D*> m_renderers;
	};
}
