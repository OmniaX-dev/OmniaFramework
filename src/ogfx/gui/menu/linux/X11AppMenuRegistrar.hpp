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

struct DBusConnection;

namespace ogfx
{
	namespace gui
	{
		// com.canonical.AppMenu.Registrar - verified live against this environment's own
		// session bus (busctl --user introspect com.canonical.AppMenu.Registrar
		// /com/canonical/AppMenu/Registrar, served by kded6's org.kde.kappmenu module):
		// RegisterWindow(uo), UnregisterWindow(u), GetMenuForWindow(u)->(so).
		struct X11AppMenuRegistrar
		{
			// Associates an X11 window with a DBusMenuServer's (service, object path). False if
			// the registrar service isn't present on the bus (no Plasma/Unity-style appmenu
			// support in the current session).
			static bool registerWindow(DBusConnection* connection, u64 x11WindowId, const String& serviceName, const String& objectPath);
			static void unregisterWindow(DBusConnection* connection, u64 x11WindowId);
			static bool isAvailable(DBusConnection* connection);
		};
	}
}
