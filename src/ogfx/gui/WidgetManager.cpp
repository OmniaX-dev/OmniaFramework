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

#include "WidgetManager.hpp"
#include "Window.hpp"
#include "widgets/Widget.hpp"
#include "../render/BasicRenderer.hpp"
#include "../../ostd/io/Memory.hpp"

namespace ogfx
{
	namespace gui
	{
		WidgetManager::WidgetManager(Window& window, Widget& owner) : m_window(window), m_owner(owner)
		{
			//
		}

		bool WidgetManager::hasWidget(Widget& widget)
		{
			bool found = std::ranges::find(m_widgetList, &widget) != m_widgetList.end();
			return found && widget.isValid();
		}

		bool WidgetManager::addWidget(Widget& widget)
		{
			if (hasWidget(widget)) return false;
			widget.m_parent = &m_owner;
			m_widgetList.insert(m_widgetList.begin() + layer_insert_index(widget.getZLayer()), &widget);
			widget.refresh();
			return true;
		}

		bool WidgetManager::removeWidget(Widget& widget)
		{
			STDVEC_REMOVE(m_widgetList, &widget);
			if (m_mousePressedOnWidget == &widget)
				m_mousePressedOnWidget = nullptr;
			// A removed widget has no parent anymore (getParent() == nullptr tells whether it is
			// in a tree, e.g. which page of a TabPanel is shown).
			if (widget.m_parent == &m_owner)
				widget.m_parent = nullptr;
			return true;
		}

		bool WidgetManager::bringToFront(Widget& widget)
		{
			auto it = std::ranges::find(m_widgetList, &widget);
			if (it == m_widgetList.end())
				return false;
			m_widgetList.erase(it);
			m_widgetList.insert(m_widgetList.begin() + layer_insert_index(widget.getZLayer()), &widget);
			return true;
		}

		void WidgetManager::sortByLayer(void)
		{
			std::ranges::stable_sort(m_widgetList, {}, [](const Widget* w) { return w ? w->getZLayer() : 0; });
		}

		void WidgetManager::draw(ogfx::BasicRenderer2D& gfx)
		{
			// The window draws the layered children of its root itself (SubWindows,
			// above everything else and with the blurred backdrop they may need).
			const bool skipLayered = (&m_owner == &m_window.getRootWidget());
			for (auto& w : m_widgetList)
			{
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->isVisible()) continue;
				if (w->isManualDrawEnabled()) continue;
				if (skipLayered && w->getZLayer() > 0) continue;
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
				if (!w->isThemingEnabled()) continue;
				w->__applyTheme(theme, true);
				w->reloadTheme();
			}
		}

		void WidgetManager::onMousePressed(const Event& event)
		{
			const i32 floor = input_floor();
			for (i32 i = m_widgetList.size() - 1; i >= floor; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->isVisible()) continue;
				if (!hit_test(w, event.mouse->position_x, event.mouse->position_y))
					continue;
				event.mouse->mousePressedOnWidget = w;
				m_window.getFocusManager().requestFocus(*w);
				w->__onMousePressed(event);
				m_mousePressedOnWidget = w;
				// Clicking a layered widget (a SubWindow) raises it within its layer.
				if (w->getZLayer() > 0)
				{
					bringToFront(*w);
					break;
				}
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
			const i32 floor = input_floor();
			for (i32 i = m_widgetList.size() - 1; i >= floor; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->isVisible()) continue;
				if (w == m_mousePressedOnWidget) continue;
				event.mouse->mousePressedOnWidget = m_mousePressedOnWidget;
				w->__onMouseReleased(event);
				processDragAndDrop(w, event);
				if (w->isMouseInside() && w->m_stopEvents)
					break;
			}
			m_mousePressedOnWidget = nullptr;
		}

		void WidgetManager::onMouseMoved(const Event& event)
		{
			if (m_mousePressedOnWidget != nullptr &&
				m_mousePressedOnWidget->m_pressedButton != MouseEventData::eButton::None &&
				!m_mousePressedOnWidget->contains(event.mouse->position_x, event.mouse->position_y, true))
			{
				m_mousePressedOnWidget->__onMouseDragged(event);
			}
			// Children below a modal one are treated as not hovered.
			const i32 floor = input_floor();
			for (i32 i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->isVisible()) continue;
				if (i < floor || !hit_test(w, event.mouse->position_x, event.mouse->position_y))
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
					if (w->m_pressedButton != MouseEventData::eButton::None)
						w->__onMouseDragged(event);
					else
						w->__onMouseMoved(event);
				}
				if (event.isHandled() || w->m_stopEvents)
				{
					// bool mouseOut = false;
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
			const i32 floor = input_floor();
			for (i32 i = m_widgetList.size() - 1; i >= floor; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->isVisible()) continue;
				if (!hit_test(w, event.mouse->position_x, event.mouse->position_y))
					continue;
				w->__onMouseScrolled(event);
				if (event.isHandled() || w->m_stopEvents)
					break;
			}
		}

		void WidgetManager::onMouseDragged(const Event& event)
		{
			const i32 floor = input_floor();
			for (i32 i = m_widgetList.size() - 1; i >= floor; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->isVisible()) continue;
				// if (!hit_test(w, event.mouse->position_x, event.mouse->position_y))
				//     continue;
				w->__onMouseDragged(event);
				if (event.isHandled())
					break;
			}
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

		bool WidgetManager::hit_test(Widget* widget, f32 x, f32 y) const
		{
			if (!widget->contains(x, y, true))
				return false;
			// Root-level owners don't clip their children; ignore-scroll children
			// (scrollbars, overlays) live outside the clipped content area.
			if (m_owner.isRootChild() || widget->isIgnoreScrollEnabled())
				return true;
			return m_owner.getGlobalPureContentBounds().contains({ x, y }, true);
		}

		i32 WidgetManager::input_floor(void) const
		{
			for (i32 i = m_widgetList.size() - 1; i >= 0; i--)
			{
				const Widget* w = m_widgetList[i];
				if (w && !w->isInvalid() && w->isVisible() && w->blocksInputBelow())
					return i;
			}
			return 0;
		}

		i32 WidgetManager::layer_insert_index(i32 layer) const
		{
			i32 index = m_widgetList.size();
			while (index > 0)
			{
				const Widget* w = m_widgetList[index - 1];
				if ((w ? w->getZLayer() : 0) <= layer)
					break;
				index--;
			}
			return index;
		}

		void WidgetManager::processDragAndDrop(Widget* widget, const Event& event)
		{
			if (widget == nullptr) return;
			if (!widget->isVisible()) return;
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
