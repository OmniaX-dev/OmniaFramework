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

#include "CocoaMenuBackend.hpp"
#import <Cocoa/Cocoa.h>

// Action target for every leaf NSMenuItem we create. One instance per CocoaMenuBackend, shared
// across every item, with an entry's id stashed in NSMenuItem.tag - mirrors how the DBusMenu
// backend will resolve activation by small integer id (see NativeMenu::assign_ids_and_bind).
@interface OmniaMenuTarget : NSObject
{
	@public std::function<void(i32)> callback;
}
- (void)itemClicked:(id)sender;
@end

@implementation OmniaMenuTarget
- (void)itemClicked:(id)sender
{
	if (callback)
		callback((i32)[(NSMenuItem*)sender tag]);
}
@end

namespace ogfx
{
	namespace gui
	{
		namespace
		{
			struct Impl
			{
				OmniaMenuTarget* target { nil };
				// mainMenu as it was right before we attached, minus appMenuItem (below) -
				// restored on detach() so we never permanently discard whatever SDL (or the app
				// itself) had set up.
				NSMenu* previousMainMenu { nil };
				// The special index-0 "application menu" item (About/Preferences/Services/Hide/
				// Quit), extracted from previousMainMenu exactly once at attach() time and reused
				// as-is on every setMenu() call - re-deriving "item at index 0" on each call would
				// grab the wrong item from the second call onward, since by then it's already
				// been moved into whatever menu bar we built last time.
				NSMenuItem* appMenuItem { nil };
			};

			NSMenu* build_menu(const stdvec<ContextMenu::Entry>& entries, OmniaMenuTarget* target)
			{
				NSMenu* menu = [[NSMenu alloc] init];
				for (auto& e : entries)
				{
					if (e.isSeparator)
					{
						[menu addItem:[NSMenuItem separatorItem]];
						continue;
					}

					NSString* title = [NSString stringWithUTF8String:e.text.c_str()];
					NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
					item.enabled = e.enabled;
					item.tag = e.id;
					if (e.checkable)
						item.state = e.checked ? NSControlStateValueOn : NSControlStateValueOff;

					if (!e.submenus.empty())
					{
						NSMenu* sub = build_menu(e.submenus, target);
						[sub setTitle:title];
						[item setSubmenu:sub];
					}
					else
					{
						[item setTarget:target];
						[item setAction:@selector(itemClicked:)];
					}
					[menu addItem:item];
				}
				return menu;
			}
		}

		CocoaMenuBackend::CocoaMenuBackend(void)
		{
			m_impl = new Impl();
		}

		CocoaMenuBackend::~CocoaMenuBackend(void)
		{
			delete static_cast<Impl*>(m_impl);
		}

		bool CocoaMenuBackend::isAvailable(WindowCore& window)
		{
			return true;
		}

		bool CocoaMenuBackend::attach(WindowCore& window, std::function<void(i32)> activateCallback)
		{
			auto* impl = static_cast<Impl*>(m_impl);
			impl->target = [[OmniaMenuTarget alloc] init];
			impl->target->callback = activateCallback;
			// NSApp.mainMenu is process-wide (not per-window), so WindowCore isn't otherwise
			// needed here - SDL's own Cocoa video backend is what actually creates NSApplication.
			impl->previousMainMenu = [NSApp mainMenu];
			impl->appMenuItem = nil;
			if (impl->previousMainMenu && impl->previousMainMenu.numberOfItems > 0)
			{
				impl->appMenuItem = [impl->previousMainMenu itemAtIndex:0];
				[impl->previousMainMenu removeItem:impl->appMenuItem];
			}
			return true;
		}

		void CocoaMenuBackend::setMenu(const stdvec<ContextMenu::Entry>& topLevel)
		{
			auto* impl = static_cast<Impl*>(m_impl);
			if (!impl->target)
				return;

			// Index 0 is always the cached application menu (see Impl::appMenuItem) so Cmd+Q
			// etc. keep working regardless of what SDL itself set up; our tree replaces
			// everything after it. Reparenting an NSMenuItem that's already attached elsewhere
			// requires removing it from its current menu first - relevant from the second
			// setMenu() call onward, since by then it's still sitting in the PREVIOUS newBar.
			NSMenu* newBar = [[NSMenu alloc] init];
			if (impl->appMenuItem)
			{
				if (impl->appMenuItem.menu)
					[impl->appMenuItem.menu removeItem:impl->appMenuItem];
				[newBar addItem:impl->appMenuItem];
			}

			for (auto& top : topLevel)
			{
				NSString* title = [NSString stringWithUTF8String:top.text.c_str()];
				NSMenuItem* topItem = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
				NSMenu* sub = build_menu(top.submenus, impl->target);
				[sub setTitle:title];
				[topItem setSubmenu:sub];
				[newBar addItem:topItem];
			}

			[NSApp setMainMenu:newBar];
		}

		void CocoaMenuBackend::detach(void)
		{
			auto* impl = static_cast<Impl*>(m_impl);
			if (impl->appMenuItem)
			{
				if (impl->appMenuItem.menu)
					[impl->appMenuItem.menu removeItem:impl->appMenuItem];
				if (impl->previousMainMenu)
					[impl->previousMainMenu insertItem:impl->appMenuItem atIndex:0];
			}
			[NSApp setMainMenu:impl->previousMainMenu];
			impl->target = nil;
			impl->previousMainMenu = nil;
			impl->appMenuItem = nil;
		}
	}
}
