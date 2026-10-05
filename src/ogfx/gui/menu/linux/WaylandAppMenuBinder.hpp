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

#include <ostd/data/Types.hpp>
#include <ostd/string/String.hpp>

namespace ogfx
{
	namespace gui
	{
		// Binds KWin's org_kde_kwin_appmenu Wayland protocol extension (vendored XML at
		// linux/protocols/appmenu.xml, codegen'd by wayland-scanner at build time - see
		// CMakeLists.txt) to associate a window's wl_surface with a DBusMenuServer's
		// (service, object path), mirroring what X11AppMenuRegistrar does via DBus on X11.
		//
		// Deliberately reuses SDL's own wl_display connection (via NativeWindowHandle) rather
		// than opening a second one - a wl_surface is only meaningful on the connection that
		// created it. This is the one piece of this whole feature I'm least able to verify
		// without a live Wayland compositor actually advertising the protocol.
		class WaylandAppMenuBinder
		{
			public:
				~WaylandAppMenuBinder(void);

				// False if the compositor doesn't advertise org_kde_kwin_appmenu_manager at all
				// (not Plasma/KWin, or an older KWin without appmenu support).
				bool bind(void* wlDisplay, void* wlSurface);
				void setAddress(const String& serviceName, const String& objectPath);
				void release(void);
				inline bool isBound(void) const { return m_impl != nullptr; }

			private:
				void* m_impl { nullptr };
		};
	}
}
