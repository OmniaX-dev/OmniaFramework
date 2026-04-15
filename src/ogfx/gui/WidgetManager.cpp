/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2025  OmniaX-Dev

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

#include "WidgetManager.hpp"
#include "widgets/Widget.hpp"
#include "../render/BasicRenderer.hpp"
#include "../utils/Keycodes.hpp"

namespace ogfx
{
	namespace gui
	{
		WidgetManager::WidgetManager(WindowCore& window, Widget& owner) : m_window(window), m_owner(owner)
		{
			//
		}

		bool WidgetManager::hasWidget(Widget& widget)
		{
			bool found = std::ranges::find(m_widgetList, &widget) != m_widgetList.end();
			return found && widget.isValid();
		}

		bool WidgetManager::requestFocus(Widget& widget)
		{
			if (!hasWidget(widget)) return false;
			if (&widget == m_focused) return true;
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (w->m_focused)
				{
					w->m_focused = false;
					w->onFocusLost(Event(m_window));
					w->setThemeQualifier("focused", false);
				}
				else
					w->m_focused = false;
			}
			widget.m_focused = true;
			widget.onFocusGained(Event(m_window));
			widget.setThemeQualifier("focused", true);
			m_focused = &widget;
			return true;
		}

		bool WidgetManager::addWidget(Widget& widget)
		{
			if (hasWidget(widget)) return false;
			widget.m_parent = &m_owner;
			m_widgetList.push_back(&widget);
			std::ranges::sort(m_widgetList, {}, [](Widget* w) {
				return w->m_zIndex;
			});
			return true;
		}

		Widget* WidgetManager::focusNext(void)
		{
			if (m_widgetList.empty())
				return nullptr;

			Widget* next = nullptr;
			Widget* smallest = nullptr;

			i32 currentTabIndex = (m_focused != nullptr ? m_focused->m_tabIndex : std::numeric_limits<i32>::max());
			for (Widget* w : m_widgetList)
			{
				i32 tab_i = w->m_tabIndex;
				if (tab_i < 0) continue;
				if (!smallest || tab_i < smallest->m_tabIndex)
					smallest = w;
				if (tab_i > currentTabIndex)
				{
					if (!next || tab_i < next->m_tabIndex)
						next = w;
				}
			}
			Widget*  w = next ? next : smallest;
			requestFocus(*w);
			return w;
		}

		void WidgetManager::draw(ogfx::BasicRenderer2D& gfx)
		{
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				gfx.pushClippingRect({ w->getGlobalPosition(), w->getSize() }, true);
				w->__draw(gfx);
				gfx.popClippingRect();
			}
		}

		void WidgetManager::update(void)
		{
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				w->__update();
			}
		}

		void WidgetManager::onThemeApplied(const ostd::Stylesheet& theme)
		{
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				w->__applyTheme(theme, true);
			}
		}

		void WidgetManager::onMousePressed(const Event& event)
		{
			for (i32 i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->contains(event.mouse->position_x, event.mouse->position_y, true))
					continue;
				w->__onMousePressed(event);
				m_mousePressedOnWidget = w;
				if (event.isHandled() || w->m_stopEvents)
					break;
			}
		}

		void WidgetManager::onMouseReleased(const Event& event)
		{
			if (m_mousePressedOnWidget != nullptr)
			{
				event.mouse->mousePressedOnWidget = m_mousePressedOnWidget;
				m_mousePressedOnWidget->__onMouseReleased(event);
				// processDragAndDrop(m_mousePressedOnWidget, event);
			}
			for (i32 i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (w == m_mousePressedOnWidget) continue;
				event.mouse->mousePressedOnWidget = m_mousePressedOnWidget;
				w->__onMouseReleased(event);
				processDragAndDrop(w, event);
				// requestFocus(*w);
				if (w->isMouseInside() && w->m_stopEvents)
					break;
			}
			m_mousePressedOnWidget = nullptr;
		}

		void WidgetManager::onMouseMoved(const Event& event)
		{
			for (i32 i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->contains(event.mouse->position_x, event.mouse->position_y, true))
				{
					if (w->m_mouseInside)
					{
						w->m_mouseInside = false;
						w->__onMouseExited(event);
					}
					continue;
				}
				if (!w->m_mouseInside)
				{
					w->m_mouseInside = true;
					w->__onMouseEntered(event);
				}
				else
				{
					if (w->m_pressedButton != ogfx::MouseEventData::eButton::None)
						w->__onMouseDragged(event);
					else
						w->__onMouseMoved(event);
				}
				if (event.isHandled() || w->m_stopEvents)
				{
					bool mouseOut = false;
					for (i32 j = i - 1; j >= 0; j--)
					{
						Widget* ww = m_widgetList[j];
						if (ww->m_mouseInside)
						{
							ww->m_mouseInside = false;
							ww->__onMouseExited(event);
						}
					}
					break;
				}
			}
		}

		void WidgetManager::onMouseScrolled(const Event& event)
		{
			for (i32 i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->contains(event.mouse->position_x, event.mouse->position_y, true))
					continue;
				w->__onMouseScrolled(event);
				if (event.isHandled() || w->m_stopEvents)
					break;
			}
		}

		void WidgetManager::onMouseDragged(const Event& event)
		{
			for (i32 i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->contains(event.mouse->position_x, event.mouse->position_y, true))
					continue;
				w->__onMouseDragged(event);
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::onKeyPressed(const Event& event)
		{
			if (!m_focused) return;
			m_focused->__onKeyPressed(event);
		}

		void WidgetManager::onKeyReleased(const Event& event)
		{
			if (!m_focused) return;
			m_focused->__onKeyReleased(event);
			if (m_tabNavigationEnabled && event.keyboard->keyCode == KeyCode::Tab)
				focusNext();
		}

		void WidgetManager::onTextEntered(const Event& event)
		{
			if (!m_focused) return;
			m_focused->__onTextEntered(event);
		}

		void WidgetManager::onWindowClosed(const Event& event)
		{
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				w->__onWindowClosed(event);
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::onWindowResized(const Event& event)
		{
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				w->__onWindowResized(event);
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::onWindowFocused(const Event& event)
		{
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				w->__onWindowFocused(event);
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::onWindowFocusLost(const Event& event)
		{
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				w->__onWindowFocusLost(event);
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::processDragAndDrop(Widget* widget, const Event& event)
		{
			if (widget == nullptr) return;
			if (!widget->isDragAndDropEnabled()) return;
			if (!widget->isMouseInside()) return;
			if (Widget::s_dragAndDropData != nullptr)
			{
				auto& const_cast_event = const_cast<Event&>(event);
				const_cast_event.drop.textOrFilePath = "";
				const_cast_event.drop.dropType = DropEventData::eDropType::InApp;
				const_cast_event.drop.userObject = Widget::s_dragAndDropData;
				widget->__onDragAndDrop(const_cast_event);
				Widget::clearDragAndDropData();
			}
		}
	}
}
