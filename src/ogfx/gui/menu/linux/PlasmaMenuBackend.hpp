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

#include <ogfx/gui/menu/NativeMenuBackend.hpp>
#include <ogfx/gui/menu/linux/DBusMenuServer.hpp>
#include <ogfx/gui/menu/NativeWindowHandle.hpp>
#include <memory>

namespace ogfx
{
	namespace gui
	{
		class WaylandAppMenuBinder;

		// Owns one DBusMenuServer (session/protocol-agnostic) and, depending on
		// NativeWindowHandle::getSessionType(), associates it with the window via either the
		// X11 AppMenu registrar or the Wayland org_kde_kwin_appmenu protocol.
		class PlasmaMenuBackend : public NativeMenuBackend
		{
			public:
				PlasmaMenuBackend(void);
				~PlasmaMenuBackend(void) override;

				bool isAvailable(WindowCore& window) override;
				bool attach(WindowCore& window, std::function<void(i32)> activateCallback) override;
				void setMenu(const stdvec<ContextMenu::Entry>& topLevel) override;
				void detach(void) override;
				void update(void) override;

			private:
				DBusMenuServer m_server;
				std::unique_ptr<WaylandAppMenuBinder> m_waylandBinder;
				u64 m_x11WindowId { 0 };
				bool m_x11Registered { false };
		};
	}
}
