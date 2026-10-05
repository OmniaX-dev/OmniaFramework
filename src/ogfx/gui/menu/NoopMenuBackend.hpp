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
		// The default/fallback backend: never available, so NativeMenu always falls through to
		// it on platforms/sessions with no native global menu support (Windows, GNOME, a plain
		// X11 WM, ...) - the in-widget MenuBar keeps behaving exactly as it does today.
		class NoopMenuBackend : public NativeMenuBackend
		{
			public:
				inline bool isAvailable(WindowCore&) override { return false; }
				inline bool attach(WindowCore&, std::function<void(i32)>) override { return false; }
				inline void setMenu(const stdvec<ContextMenu::Entry>&) override {  }
				inline void detach(void) override {  }
		};
	}
}
