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

#include "FocusManager.hpp"
#include "Window.hpp"
#include "../utils/Keycodes.hpp"

namespace ogfx
{
	namespace gui
	{
		FocusManager::FocusManager(Window& window) : m_window(window)
		{

		}

		bool FocusManager::requestFocus(Widget& widget)
		{
			if (&widget == m_focused) return true;
			if (!widget.isFocusEnabled()) return false;
			if (m_window.getToolBar().isVisible())
			{
				if (requestFocus(widget, m_window.getToolBar()))
					return true;
			}
			if (m_window.getStatusBar().isVisible())
			{
				if (requestFocus(widget, m_window.getStatusBar()))
					return true;
			}
			return requestFocus(widget, m_window.m_rootWidget);
		}

		bool FocusManager::requestFocus(Widget& widget, Widget& root)
		{
			if (&widget == m_focused) return true;
			if (!widget.isFocusEnabled()) return false;
			if (widget.isInvalid()) return false;
			if (!widget.isVisible()) return false;
			if (!root.hasChildren() || !root.isChildrenEnabled())
				return false;
			if (!root.getChildren().hasWidget(widget))
			{
				for (auto& child : root.getChildren().m_widgetList)
				{
					if (child && requestFocus(widget, *child))
						return true;
				}
				return false;
			}

			if (m_focused)
			{
				m_focused->m_focused = false;
				if (m_focused->callback_onFocusLost)
					m_focused->callback_onFocusLost(Event(m_window));
				m_focused->onFocusLost(Event(m_window));
				m_focused->setThemeQualifier("focused", false);
			}

			widget.m_focused = true;
			if (widget.callback_onFocusGained)
				widget.callback_onFocusGained(Event(m_window));
			widget.onFocusGained(Event(m_window));
			widget.setThemeQualifier("focused", true);
			m_focused = &widget;
			return true;
		}

		Widget* FocusManager::focusNext(void)
		{
			if (m_tabIndexMap.empty())
				return nullptr;

			i32 startIndex = -1, smallest = i32Range::max();
			if (m_focused)
				startIndex = m_focused->m_tabIndex;
			i32 nextIndex = i32Range::max();
			Widget* nextPtr = nullptr;
			Widget* smallestPtr = nullptr;

			for (auto&[wptr, index] : m_tabIndexMap)
			{
				if (index < smallest)
				{
					smallest = index;
					smallestPtr = wptr;
				}
				if (index > startIndex && index < nextIndex)
				{
					nextIndex = index;
					nextPtr = wptr;
				}
			}

			Widget*  w = nullptr;
			if (!nextPtr)
			{
				if (!smallestPtr)
					return nullptr;
				w = smallestPtr;
			}
			else
				w = nextPtr;

			requestFocus(*w);
			return w;
		}

		bool FocusManager::registerTabIndex(Widget& widget, i32 index)
		{
			if (index >= 0)
			{
				m_tabIndexMap[&widget] = index;
				return true;
			}
			return false;
		}

		void FocusManager::unregisterTabIndex(Widget& widget)
		{
			if (m_tabIndexMap.count(&widget) > 0)
				m_tabIndexMap.erase(m_tabIndexMap.find(&widget));
		}

		void FocusManager::clearFocus(void)
		{
			if (m_focused)
			{
				m_focused->m_focused = false;
				m_focused = nullptr;
			}
		}

		void FocusManager::onKeyReleased(const Event& event)
		{
			if (isTabNavigationEnabled() && event.keyboard->keyCode == KeyCode::Tab)
				focusNext();
			else if (event.keyboard->keyCode == KeyCode::Space || event.keyboard->keyCode == KeyCode::Return || event.keyboard->keyCode == KeyCode::Return2 || event.keyboard->keyCode == KeyCode::KpEnter)
			{
				if (!m_focused)
					return;
				if (m_focused->callback_onActionPerformed)
					m_focused->callback_onActionPerformed(event);
			}
		}
	}
}
