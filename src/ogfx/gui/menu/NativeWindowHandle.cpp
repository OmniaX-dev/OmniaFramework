#include "NativeWindowHandle.hpp"
#include <cstring>

namespace ogfx
{
	namespace gui
	{
		NativeWindowHandle::eSessionType NativeWindowHandle::getSessionType(void)
		{
			const char* driver = SDL_GetCurrentVideoDriver();
			if (!driver) return eSessionType::Unknown;
			if (std::strcmp(driver, "x11") == 0) return eSessionType::X11;
			if (std::strcmp(driver, "wayland") == 0) return eSessionType::Wayland;
			return eSessionType::Unknown;
		}

		u64 NativeWindowHandle::getX11WindowId(WindowCore& window)
		{
			SDL_PropertiesID props = SDL_GetWindowProperties(window.getSDLWindow());
			return (u64)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
		}

		void* NativeWindowHandle::getX11Display(WindowCore& window)
		{
			SDL_PropertiesID props = SDL_GetWindowProperties(window.getSDLWindow());
			return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
		}

		void* NativeWindowHandle::getWaylandSurface(WindowCore& window)
		{
			SDL_PropertiesID props = SDL_GetWindowProperties(window.getSDLWindow());
			return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
		}

		void* NativeWindowHandle::getWaylandDisplay(WindowCore& window)
		{
			SDL_PropertiesID props = SDL_GetWindowProperties(window.getSDLWindow());
			return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
		}
	}
}
