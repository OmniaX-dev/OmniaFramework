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

#include <ogfx/gui/ContextMenu.hpp>
#include <functional>
#include <dbus/dbus.h> // only ever included from other Linux-specific files - safe to use directly

namespace ogfx
{
	namespace gui
	{
		// Implements the com.canonical.dbusmenu interface over raw libdbus-1 - the same protocol
		// regardless of X11 or Wayland; only how a window gets ASSOCIATED with this server
		// (X11AppMenuRegistrar vs WaylandAppMenuBinder) differs between the two.
		//
		// Confirmed against this environment's real session bus: the AppMenu registrar
		// interface this pairs with on X11 is `com.canonical.AppMenu.Registrar`, methods
		// RegisterWindow(uo)/UnregisterWindow(u)/GetMenuForWindow(u)->(so) - verified live via
		// `busctl --user introspect com.canonical.AppMenu.Registrar /com/canonical/AppMenu/Registrar`
		// against kded6's org.kde.kappmenu module.
		class DBusMenuServer
		{
			public:
				~DBusMenuServer(void);

				// Connects to the session bus and registers our object path. False if no session
				// bus is reachable.
				bool connect(void);
				void disconnect(void);
				inline bool isConnected(void) const { return m_connection != nullptr; }

				// Call once per frame - non-blocking; processes any pending GetLayout/Event/...
				// calls that arrived since the last call.
				void update(void);

				// Replaces the served menu tree and emits LayoutUpdated(revision, 0) so any
				// client (Plasma's panel) that already has GetLayout's result cached knows to
				// re-fetch it.
				void setMenu(const stdvec<ContextMenu::Entry>& topLevel);

				// Invoked with an Entry's id whenever a client reports it was clicked (the
				// "clicked" Event(...) call) - this is how NativeMenu::activate() gets called.
				inline void setActivateCallback(std::function<void(i32)> cb) { m_onActivate = std::move(cb); }

				inline const String& getServiceName(void) const { return m_serviceName; }
				inline const String& getObjectPath(void) const { return m_objectPath; }
				inline DBusConnection* getConnection(void) const { return m_connection; }

			private:
				static DBusHandlerResult handle_message(DBusConnection* connection, DBusMessage* message, void* userData);
				DBusHandlerResult handle_message_impl(DBusMessage* message);
				DBusMessage* handle_get_layout(DBusMessage* msg);
				DBusMessage* handle_get_group_properties(DBusMessage* msg);
				DBusMessage* handle_get_property(DBusMessage* msg);
				DBusMessage* handle_event(DBusMessage* msg);
				DBusMessage* handle_event_group(DBusMessage* msg);
				DBusMessage* handle_about_to_show(DBusMessage* msg);
				DBusMessage* handle_about_to_show_group(DBusMessage* msg);
				DBusMessage* handle_properties_get(DBusMessage* msg);
				DBusMessage* handle_properties_get_all(DBusMessage* msg);
				DBusMessage* handle_introspect(DBusMessage* msg);

			private:
				DBusConnection* m_connection { nullptr };
				String m_serviceName;
				String m_objectPath { "/MenuBar" };
				stdvec<ContextMenu::Entry> m_topLevel;
				u32 m_revision { 1 };
				std::function<void(i32)> m_onActivate { nullptr };
		};
	}
}
