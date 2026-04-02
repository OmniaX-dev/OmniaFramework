/*
    OmniaFramework - A collection of useful functionality
    Copyright (C) 2025  OmniaX-Dev

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

#include <ostd/utils/Defines.hpp>
#include <ostd/io/Logger.hpp>
#include <ostd/utils/Signals.hpp>

#include <SDL3/SDL.h>
#include <ogfx/vendor/sdl3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

namespace ogfx
{
	class SDLSysten
	{
		public:
			static inline void acquire(void)
			{
		        if (s_refCount == 0)
		            init_sdl();
		        s_refCount++;
		    }

		    static inline void release(void)
			{
		        s_refCount--;
		        if (s_refCount == 0)
		            shutdown_sdl();
		    }

		private:
			static inline void init_sdl(void)
			{
				if (!SDL_Init(SDL_INIT_VIDEO))
				{
					OX_FATAL("SDL could not initialize! Error: %s\n", SDL_GetError());
					exit(ErrorSDLInitFailed);
				}
				if (!TTF_Init())
				{
					OX_FATAL("SDL_ttf could not initialize! Error: %s\n", SDL_GetError());
					SDL_Quit();
					exit(ErrorSDLTTFInitFailed);
				}
			}

			static inline void shutdown_sdl(void)
			{
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::BeforeSDLShutdown, ostd:: tSignalPriority::RealTime);
				TTF_Quit();
				SDL_Quit();
			}

	    private:
	    	inline static int s_refCount { 0 };

		public:
			inline static constexpr int32_t ErrorSDLInitFailed { 1 };
			inline static constexpr int32_t ErrorSDLTTFInitFailed { 2 };

	};
}
