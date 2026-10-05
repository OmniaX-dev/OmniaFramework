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

namespace ogfx
{
	class WindowCore;

	namespace gui
	{
		// One virtual interface, swappable concrete backends chosen per-platform - mirrors how
		// ostd::io::OutputHandlerBase is subclassed by ConsoleOutputHandler/LogFileOutputHandler/
		// GraphicsWindowOutputHandler (see src/ostd/io/IOHandlers.hpp).
		//
		// A backend mirrors the app's existing MenuBar into a native OS global menu. It never
		// owns the menu data - NativeMenu (NativeMenu.hpp) rebuilds and pushes a plain
		// stdvec<ContextMenu::Entry> tree whenever the app's MenuBar changes, resolving each
		// leaf's activation through the same ContextMenu::Instance::onActivate callbacks the
		// app already registered - nothing about the app-facing MenuBar/ContextMenu API changes.
		class NativeMenuBackend
		{
			public:
				inline virtual ~NativeMenuBackend(void) = default;

				// Cheap, side-effect-free probe: can this backend actually attach right now?
				// (e.g. is a Plasma session reachable over DBus; on macOS, always true.)
				virtual bool isAvailable(WindowCore& window) = 0;

				// One-time setup (DBus service registration, Wayland protocol binding, setting
				// NSApp.mainMenu, ...). Returns false if attaching failed even though
				// isAvailable() returned true (session dropped mid-handshake, etc.) - the caller
				// falls through to the next candidate backend (or the in-widget MenuBar) either way.
				// activateCallback is how this backend reports a click back - call it with the
				// id exactly as it appeared in the Entry tree handed to setMenu(), whenever the
				// OS reports that item was chosen (a DBusMenu Event("clicked") call, an
				// NSMenuItem action, ...). The backend should hold onto it until detach().
				virtual bool attach(WindowCore& window, std::function<void(i32)> activateCallback) = 0;

				// Replaces the whole menu tree. Called once right after a successful attach(),
				// and again any time the app's MenuBar structure or an Entry's enabled/checked
				// state changes.
				virtual void setMenu(const stdvec<ContextMenu::Entry>& topLevel) = 0;

				virtual void detach(void) = 0;

				// Called once per frame while attached, regardless of whether the menu tree
				// changed - lets a backend drive its own I/O (pumping a DBus connection, say).
				// Default no-op; most backends (Cocoa, Noop) don't need it.
				inline virtual void update(void) {  }
		};
	}
}
