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
#include <ogfx/gui/MenuBar.hpp>
#include <memory>
#include <functional>

namespace ogfx
{
	namespace gui
	{
		// Window-facing coordinator: owns the ordered list of platform candidate backends,
		// picks the first one that's actually available, and translates MenuBar's slots into
		// the plain Entry tree + activation-by-id map a NativeMenuBackend needs. None of this
		// changes how MenuBar/ContextMenu work for callers - it only ever reads their existing
		// public shape (MenuBar::Menu == {label, ContextMenu::Instance}).
		class NativeMenu
		{
			public:
				inline NativeMenu(WindowCore& window) : m_window(window) { register_candidates(); }

				// Tries each candidate in order; true if one attached. Safe to call again later
				// (e.g. a session reconnects) - re-tries from the top.
				bool enable(void);
				void disable(void);
				inline bool isActive(void) const { return m_backend != nullptr; }

				// Rebuilds the activation map and pushes the tree to the active backend, if any.
				// A no-op (aside from caching) when no backend is attached, so callers can call
				// this unconditionally after any MenuBar mutation without checking isActive() first.
				void setMenu(const stdvec<MenuBar::Menu>& menus);

				// Call once per frame - forwards to the active backend's own update(), if any
				// (e.g. pumping a DBus connection). Safe to call unconditionally.
				inline void update(void) { if (m_backend) m_backend->update(); }

				// Looked up by id and invoked when a backend reports a click - see Event(...) in
				// the DBusMenu backend and the NSMenuItem action trampoline in the Cocoa backend.
				void activate(i32 id);

			private:
				void register_candidates(void);
				void flatten(const stdvec<MenuBar::Menu>& menus, stdvec<ContextMenu::Entry>& outTopLevel);
				void assign_ids_and_bind(stdvec<ContextMenu::Entry>& entries, const ContextMenu::Instance::Callback& onActivate);

			private:
				WindowCore& m_window;
				stdvec<std::unique_ptr<NativeMenuBackend>> m_candidates;
				NativeMenuBackend* m_backend { nullptr };
				stdumap<i32, std::function<void(void)>> m_activationMap;
				i32 m_nextSyntheticId { -2 }; // -1 already means "no id" (ContextMenu::Entry convention)
		};
	}
}
