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

namespace ogfx
{
	namespace gui
	{
		// macOS's process-wide menu bar (NSApp.mainMenu). Plain C++ interface - the actual
		// Cocoa/Objective-C types only appear in CocoaMenuBackend.mm, so this header is safe to
		// include from ordinary C++ translation units (NativeMenu.cpp) without pulling in
		// Objective-C syntax. Compiled/linked only under APPLE - see CMakeLists.txt.
		class CocoaMenuBackend : public NativeMenuBackend
		{
			public:
				CocoaMenuBackend(void);
				~CocoaMenuBackend(void) override;

				// No probing needed - the app's own process always has a menu bar on macOS.
				bool isAvailable(WindowCore& window) override;
				bool attach(WindowCore& window, std::function<void(i32)> activateCallback) override;
				void setMenu(const stdvec<ContextMenu::Entry>& topLevel) override;
				void detach(void) override;

			private:
				// Opaque handle to the Objective-C-side state (the action target object, and the
				// NSMenu* we had before attaching, so detach() can restore it) - kept as void* so
				// this header never needs an Objective-C compiler to parse it.
				void* m_impl { nullptr };
		};
	}
}
