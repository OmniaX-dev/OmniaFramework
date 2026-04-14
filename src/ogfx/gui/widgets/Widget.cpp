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

#include "Widget.hpp"
#include "gui/Events.hpp"
#include "io/Memory.hpp"
#include "../..//render/BasicRenderer.hpp"
#include "../Window.hpp"

namespace ogfx
{
	namespace gui
	{
		ostd::BaseObject* Widget::s_dragAndDropData { nullptr };
		bool Widget::s_hasDragAndDropData { false };

		Widget::Widget(const ostd::Rectangle& bounds, WindowCore& window) : Rectangle(bounds), m_widgets(window, *this)
		{
			m_window = &window;
		}

		bool Widget::addChild(Widget& child)
		{
			if (!m_allowChildren)
				return false;
			child.reloadTheme();
			return m_widgets.addWidget(child);
		}

		ostd::Vec2 Widget::getGlobalPosition(void) const
		{
			ostd::Vec2 glob = getPosition();
			if (!m_rootChild && m_parent != nullptr)
				glob += m_parent->getGlobalPosition() + m_parent->getPadding().getPosition();
			glob += m_margin.getPosition();
			return glob;
		}

		ostd::Vec2 Widget::getGlobalContentPosition(void) const
		{
			return getGlobalPosition() + getContentBounds().getPosition();
		}

		ostd::Rectangle Widget::getGlobalBounds(void) const
		{
			return { getGlobalPosition(), getSize() + (m_margin.getSize() * 2) };
		}

		ostd::Rectangle Widget::getContentBounds(void) const
		{
			auto pad = getPadding();
			return { pad.getPosition(), (getSize() - (pad.getSize() * 2)) };
		}

		ostd::Rectangle Widget::getGlobalContentBounds(void) const
		{
			auto pad = getPadding();
			return { getGlobalContentPosition(), getContentBounds().getSize() };
		}

		bool Widget::contains(ostd::Vec2 p, bool includeBounds) const
		{
			return ostd::Rectangle(getGlobalPosition(), getSize()).contains(p, includeBounds);
		}

		void Widget::enable(bool enable)
		{
			m_enabled = enable;
			setThemeQualifier("disabled", !enable);
		}

		void Widget::addThemeOverride(const ostd::String& fullKey, ostd::Stylesheet::TypeVariant value)
		{
			m_themeOverrides.push_back({ fullKey, value });
		}

		void Widget::reloadTheme(void)
		{
			if (getWindow().theme() == nullptr)
				return;

			auto& theme = *getWindow().theme();
			applyTheme(theme);
			ostd::Stylesheet& const_cast_theme = const_cast<ostd::Stylesheet&>(theme);

			struct tBackup {
				const ostd::Stylesheet::TypeVariant* ptr;
				ostd::Stylesheet::TypeVariant val;
				ostd::String key;
			};

			std::vector<tBackup> backup;
			for (auto& rule : m_themeOverrides)
			{
				auto currentValuePtr = theme.getFull(rule.fullKey);
				ostd::Stylesheet::TypeVariant currentValue;
				if (currentValuePtr)
					currentValue = *currentValuePtr;
				backup.push_back({ currentValuePtr, currentValue, rule.fullKey });
				const_cast_theme.setFull(rule.fullKey, rule.value);
			}
			__applyTheme(const_cast_theme, false);
			for (auto&[ptr, val, key] : backup)
			{
				if (ptr == nullptr)
					const_cast_theme.removeRule(key);
				else
					const_cast_theme.setFull(key, val);
			}
		}

		void Widget::setThemeQualifier(const ostd::String& qualifier, bool value)
		{
		    for (auto& [name, state] : m_qualifierList)
		    {
		        if (name == qualifier)
		        {
					if (state == value) return;
		            state = value;
					reloadTheme();
		            return;
		        }
		    }
		}

		bool Widget::getThemeQualifier(const ostd::String& qualifier) const
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
			else
			{
				if (m_showBackground && m_showBorder)
					gfx.outlinedRoundRect({ getGlobalPosition(), getSize() }, m_backgroundColor, m_borderColor, m_borderRadius, m_borderWidth);
				else if (m_showBackground)
					gfx.fillRoundRect({ getGlobalPosition(), getSize() }, m_backgroundColor, m_borderRadius);
			}
			onDraw(gfx);
			if (hasChildren())
				m_widgets.draw(gfx);
			if (m_showBorder)
				gfx.drawRoundRect({ getGlobalPosition(), getSize() }, m_borderColor, m_borderRadius, m_borderWidth);
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

		void Widget::__onDragAndDrop(const Event& event)
		{
			if (hasChildren())
				m_widgets.onMouseReleased(event);
			if (!event.isHandled())
			{
				if (callback_onDragAndDrop)
					callback_onDragAndDrop(event);
				onDragAndDrop(event);
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

		void Widget::__onMouseScrolled(const Event& event)
		{
			if (hasChildren())
				m_widgets.onMouseScrolled(event);
			if (!event.isHandled())
			{
				if (callback_onMouseScrolled)
					callback_onMouseScrolled(event);
				onMouseScrolled(event);
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

		void Widget::__onWindowFocused(const Event& event)
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
	}
}
