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

#include "Widgets.hpp"
#include "gui/Events.hpp"
#include "io/Memory.hpp"
#include "utils/Keycodes.hpp"
#include <ogfx/render/BasicRenderer.hpp>
#include <ogfx/gui/Window.hpp>

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
				}
				else
					w->m_focused = false;
			}
			widget.m_focused = true;
			widget.onFocusGained(Event(m_window));
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

			int32_t currentTabIndex = (m_focused != nullptr ? m_focused->m_tabIndex : std::numeric_limits<int32_t>::max());
			for (Widget* w : m_widgetList)
			{
				int tab_i = w->m_tabIndex;
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
				w->__draw(gfx);
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
			for (int32_t i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				if (!w->contains(event.mouse->position_x, event.mouse->position_y, true))
					continue;
				w->__onMousePressed(event);
				m_mousePressedOnWidget = w;
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::onMouseReleased(const Event& event)
		{
			for (int32_t i = m_widgetList.size() - 1; i >= 0; i--)
			{
				Widget* w = m_widgetList[i];
				if (w == nullptr) continue;
				if (w->isInvalid()) continue;
				event.mouse->mousePressedOnWidget = m_mousePressedOnWidget;
				w->__onMouseReleased(event);
				requestFocus(*w);
				m_mousePressedOnWidget = nullptr;
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::onMouseMoved(const Event& event)
		{
			for (int32_t i = m_widgetList.size() - 1; i >= 0; i--)
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
				if (event.isHandled())
					break;
			}
		}

		void WidgetManager::onMouseDragged(const Event& event)
		{
			for (int32_t i = m_widgetList.size() - 1; i >= 0; i--)
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
				w->__onWIndowFocused(event);
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




		Widget::Widget(const ostd::Rectangle& bounds, WindowCore& window) : Rectangle(bounds), m_widgets(window, *this)
		{
			m_window = &window;
		}

		bool Widget::addChild(Widget& child)
		{
			if (!m_allowChildren)
				return false;
			return m_widgets.addWidget(child);
		}

		ostd::Vec2 Widget::getGlobalPosition(void) const
		{
			ostd::Vec2 glob = getPosition();
			if (!m_rootChild && m_parent != nullptr)
				glob += m_parent->getGlobalPosition();
			return glob;
		}

		bool Widget::contains(ostd::Vec2 p, bool includeBounds) const
		{
			return ostd::Rectangle(getGlobalPosition(), getSize()).contains(p, includeBounds);
		}

		void Widget::addThemeOverride(const ostd::String& key, ostd::Stylesheet::TypeVariant value, const ostd::String& themeID, const ostd::String& qualifier, bool propagate)
		{
			ostd::String fullKey = "@" + themeID;
			if (qualifier.new_trim() != "")
				fullKey += ":" + qualifier;
			fullKey += "." + key;
			m_themeOverrides.push_back({ fullKey, value, propagate });
		}

		void Widget::reloadTheme(void)
		{
			if (getWindow().theme() == nullptr)
				return;

			auto& theme = *getWindow().theme();
			applyTheme(theme);
			ostd::Stylesheet& const_cast_theme = const_cast<ostd::Stylesheet&>(theme);

			for (auto& rule : m_themeOverrides)
			{
				auto currentValue = theme.getFull(rule.fullKey);
				if (currentValue == nullptr)
					return;
				const_cast_theme.setFull(rule.fullKey, rule.value);
				__applyTheme(const_cast_theme, rule.propagate);
				const_cast_theme.setFull(rule.fullKey, *currentValue);
			}
		}

		void Widget::setThemeQualifier(const ostd::String& qualifier, bool value)
		{
		    for (auto& [name, state] : m_qualifierList)
		    {
		        if (name == qualifier)
		        {
		            state = value;
					reloadTheme();
		            return;
		        }
		    }
		}

		bool Widget::getThemeQualifier(const ostd::String& qualifier)
		{
			for (auto& [name, state] : m_qualifierList)
			{
				if (name == qualifier)
					return state;
			}
			return false;
		}

		bool Widget::addThemeID(const ostd::String& id)
		{
			if (STDVEC_CONTAINS(m_themeIDList, id))
				return false;
			m_themeIDList.push_back(id);
			return true;
		}

		bool Widget::removeThemeID(const ostd::String& id)
		{
			if (!STDVEC_CONTAINS(m_themeIDList, id))
				return false;
			STDVEC_REMOVE(m_themeIDList, id);
			return true;
		}

		void Widget::__draw(ogfx::BasicRenderer2D& gfx)
		{
			if (isDrawBoxEnabled())
				gfx.fillRect({ getGlobalPosition(), getSize() }, getDrawBoxColor());
			onDraw(gfx);
			if (hasChildren())
				m_widgets.draw(gfx);
		}

		void Widget::__update(void)
		{
			onUpdate();
			if (hasChildren())
				m_widgets.update();
		}

		void Widget::__onMousePressed(const Event& event)
		{
			setThemeQualifier("pressed");
			if (hasChildren())
				m_widgets.onMousePressed(event);
			if (!event.isHandled())
			{
				if (callback_onMousePressed)
					callback_onMousePressed(event);
				onMousePressed(event);
				m_pressedButton = event.mouse->button;
			}
		}

		void Widget::__onMouseReleased(const Event& event)
		{
			setThemeQualifier("pressed", false);
			m_pressedButton = ogfx::MouseEventData::eButton::None;
			if (hasChildren())
				m_widgets.onMouseReleased(event);
			if (!event.isHandled())
			{
				if (callback_onMouseReleased)
					callback_onMouseReleased(event);
				onMouseReleased(event);
			}
		}

		void Widget::__onMouseMoved(const Event& event)
		{
			if (hasChildren())
				m_widgets.onMouseMoved(event);
			if (!event.isHandled())
			{
				if (callback_onMouseMoved)
					callback_onMouseMoved(event);
				onMouseMoved(event);
			}
		}

		void Widget::__onMouseEntered(const Event& event)
		{
			setThemeQualifier("hover");
			if (callback_onMouseEntered)
				callback_onMouseEntered(event);
			onMouseEntered(event);
		}

		void Widget::__onMouseExited(const Event& event)
		{
			setThemeQualifier("hover", false);
			if (callback_onMouseExited)
				callback_onMouseExited(event);
			onMouseExited(event);
		}

		void Widget::__onMouseDragged(const Event& event)
		{
			if (hasChildren())
				m_widgets.onMouseDragged(event);
			if (!event.isHandled())
			{
				if (callback_onMouseDragged)
					callback_onMouseDragged(event);
				onMouseDragged(event);
			}
		}

		void Widget::__onKeyPressed(const Event& event)
		{
			if (hasChildren())
				m_widgets.onKeyPressed(event);
			if (!event.isHandled())
			{
				if (callback_onKeyPressed)
					callback_onKeyPressed(event);
				onKeyPressed(event);
			}
		}

		void Widget::__onKeyReleased(const Event& event)
		{
			m_pressedButton = ogfx::MouseEventData::eButton::None;
			if (hasChildren())
				m_widgets.onKeyReleased(event);
			if (!event.isHandled())
			{
				if (callback_onKeyReleased)
					callback_onKeyReleased(event);
				onKeyReleased(event);
			}
		}

		void Widget::__onTextEntered(const Event& event)
		{
			if (hasChildren())
				m_widgets.onTextEntered(event);
			if (!event.isHandled())
			{
				if (callback_onTextEntered)
					callback_onTextEntered(event);
				onTextEntered(event);
			}
		}

		void Widget::__onWindowClosed(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowClosed(event);
			if (!event.isHandled())
			{
				if (callback_onWindowClosed)
					callback_onWindowClosed(event);
				onWindowClosed(event);
			}
		}

		void Widget::__onWindowResized(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowResized(event);
			if (!event.isHandled())
			{
				if (callback_onWindowResized)
					callback_onWindowResized(event);
				onWindowResized(event);
			}
		}

		void Widget::__onWIndowFocused(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowFocused(event);
			if (!event.isHandled())
			{
				if (callback_onWindowFocused)
					callback_onWindowFocused(event);
				onWindowFocused(event);
			}
		}

		void Widget::__onWindowFocusLost(const Event& event)
		{
			if (hasChildren())
				m_widgets.onWindowFocusLost(event);
			if (!event.isHandled())
			{
				if (callback_onWindowFocusLost)
					callback_onWindowFocusLost(event);
				onWindowFocusLost(event);
			}
		}

		void Widget::__applyTheme(const ostd::Stylesheet& theme, bool propagate)
		{
			if (propagate && hasChildren())
				m_widgets.onThemeApplied(theme);
			applyTheme(theme);
		}



		namespace widgets
		{
			RootWidget::RootWidget(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window)
			{
				disableDrawBox();
				m_rootChild = true;
				setSize(static_cast<float>(window.getWindowWidth()), static_cast<float>(window.getWindowHeight()));
				setTypeName("ogfx::gui::widgets::RootWidget");
			}

			void RootWidget::onWindowResized(const Event& event)
			{
				setSize(static_cast<float>(event.windowResized->new_width), static_cast<float>(event.windowResized->new_height));
			}

			void RootWidget::applyTheme(const ostd::Stylesheet& theme)
			{
				m_color = getThemeValue<ostd::Color>(theme, "window.backgroundColor", getWindow().getClearColor());
			}

			void RootWidget::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				gfx.fillRect({ 0, 0, static_cast<float>(getWindow().getWindowWidth()), static_cast<float>(getWindow().getWindowHeight()) }, m_color);
			}




			Label& Label::create(const ostd::String& text)
			{
				setText(text);
				setPadding({ 5, 5, 5, 5 });
				setTypeName("ogfx::gui::widgets::Label");
				disableDrawBox();
				enableBackground(false);
				validate();
				return *this;
			}

			void Label::applyTheme(const ostd::Stylesheet& theme)
			{
				setColor(getThemeValue<ostd::Color>(theme, "label.textColor", ostd::Colors::White));
				setBackGroundColor(getThemeValue<ostd::Color>(theme, "label.backgroundColor", ostd::Colors::Transparent));
				setFontSize(getThemeValue<int32_t>(theme, "label.fontSize", 20));
				m_borderRadius = getThemeValue<int32_t>(theme, "label.borderRadius", 10);
				m_borderWidth = getThemeValue<int32_t>(theme, "label.borderWidth", 2);
				m_showBorder = getThemeValue<bool>(theme, "label.showBorder", false);
				m_borderColor = getThemeValue<ostd::Color>(theme, "label.borderColor", ostd::Colors::White);
				enableBackground(getThemeValue<bool>(theme, "label.showBackground", false));
				setPadding(getThemeValue<ostd::Rectangle>(theme, "label.padding", { 5, 5, 5, 5 }));
			}

			void Label::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				if (m_textChanged)
					__update_size(gfx);
				if (m_showBackground)
					gfx.fillRoundRect({ getGlobalPosition(), getSize() }, m_backgroundColor, m_borderRadius);
				if (m_showBorder)
					gfx.drawRoundRect({ getGlobalPosition(), getSize() }, m_borderColor, m_borderRadius, m_borderWidth);
				gfx.drawString(getText(), getGlobalPosition() + ostd::Vec2 { getPadding().left(), getPadding().top() }, getColor(), getFontSize());
			}

			void Label::setText(const ostd::String& text)
			{
				m_text = text;
				m_textChanged = true;
			}

			void Label::__update_size(ogfx::BasicRenderer2D& gfx)
			{
				auto size = gfx.getStringSize(getText(), getFontSize());
				size.x += getPadding().left();
				size.x += getPadding().right();
				size.y += getPadding().top();
				size.y += getPadding().bottom();
				setSize({ static_cast<float>(size.x), static_cast<float>(size.y) });
				m_textChanged = false;
			}

		}
	}
}
