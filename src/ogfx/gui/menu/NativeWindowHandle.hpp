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

#include <ogfx/gui/Window.hpp>

namespace ogfx
{
	namespace gui
	{
		// SDL3's unified property-based native-handle API (replacing the old SDL_GetWindowWMInfo),
		// confirmed against the SDL3 headers actually installed in this environment
		// (SDL_PROP_WINDOW_X11_WINDOW_NUMBER etc., SDL_video.h).
		struct NativeWindowHandle
		{
			enum class eSessionType { Unknown, X11, Wayland };
			static eSessionType getSessionType(void);

			static u64 getX11WindowId(WindowCore& window);   // 0 if not running on X11
			static void* getX11Display(WindowCore& window);  // nullptr if not running on X11

			static void* getWaylandSurface(WindowCore& window); // nullptr if not running on Wayland
			static void* getWaylandDisplay(WindowCore& window); // nullptr if not running on Wayland
		};
	}
}
