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

#pragma once

#include <ogfx/gui/Events.hpp>
#include <ogfx/gui/WidgetManager.hpp>
#include <ostd/data/BaseObject.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/io/Stylesheet.hpp>
#include <functional>

namespace ogfx
{
	namespace gui
	{
		class Widget : public ostd::BaseObject, public ostd::Rectangle
		{
			private: struct ThemeOverride
			{
				ostd::String fullKey;
				ostd::Stylesheet::TypeVariant value;
				bool propagate;
			};
			public: using EventCallback = std::function<void(const Event&)>;
			public:
				Widget(const ostd::Rectangle& bounds, WindowCore& window);
				bool addChild(Widget& child);
				virtual ostd::Vec2 getGlobalPosition(void) const;
				virtual ostd::Vec2 getGlobalContentPosition(void) const;
				virtual ostd::Rectangle getGlobalBounds(void) const;
				virtual ostd::Rectangle getContentBounds(void) const;
				virtual ostd::Rectangle getGlobalContentBounds(void) const;
				using ostd::Rectangle::contains;
				bool contains(ostd::Vec2 p, bool includeBounds = false) const override;
				void enable(bool enable = true);
				virtual void applyTheme(const ostd::Stylesheet& theme) = 0;
				void addThemeOverride(const ostd::String& fullKey, ostd::Stylesheet::TypeVariant value, bool propagate = true);
				void reloadTheme(void);
				void setThemeQualifier(const ostd::String& qualifier, bool value = true);
				bool getThemeQualifier(const ostd::String& qualifier) const;
				bool addThemeID(const ostd::String& id);
				bool removeThemeID(const ostd::String& id);
				inline const ostd::Stylesheet::QualifierList& getThemeQualifierList(void) const { return m_qualifierList; }

				inline virtual void onDraw(ogfx::BasicRenderer2D& gfx) {  }
				inline virtual void onUpdate(void) {  }

				inline virtual void onMousePressed(const Event& event) {  }
				inline virtual void onMouseReleased(const Event& event) {  }
				inline virtual void onMouseMoved(const Event& event) {  }
				inline virtual void onDragAndDrop(const Event& event) {  }
				inline virtual void onMouseEntered(const Event& event) {  }
				inline virtual void onMouseExited(const Event& event) {  }
				inline virtual void onMouseDragged(const Event& event) {  }
				inline virtual void onKeyPressed(const Event& event) {  }
				inline virtual void onKeyReleased(const Event& event) {  }
				inline virtual void onTextEntered(const Event& event) {  }
				inline virtual void onFocusGained(const Event& event) {  }
				inline virtual void onFocusLost(const Event& event) {  }
				inline virtual void onWindowClosed(const Event& event) {  }
				inline virtual void onWindowResized(const Event& event) {  }
				inline virtual void onWindowFocused(const Event& event) {  }
				inline virtual void onWindowFocusLost(const Event& event) {  }

				void __draw(ogfx::BasicRenderer2D& gfx);
				void __update(void);
				void __onMousePressed(const Event& event);
				void __onMouseReleased(const Event& event);
				void __onDragAndDrop(const Event& event);
				void __onMouseMoved(const Event& event);
				void __onMouseEntered(const Event& event);
				void __onMouseExited(const Event& event);
				void __onMouseDragged(const Event& event);
				void __onKeyPressed(const Event& event);
				void __onKeyReleased(const Event& event);
				void __onTextEntered(const Event& event);
				void __onWindowClosed(const Event& event);
				void __onWindowResized(const Event& event);
				void __onWindowFocused(const Event& event);
				void __onWindowFocusLost(const Event& event);
				void __applyTheme(const ostd::Stylesheet& theme, bool propagate);

				inline virtual void setMousePressedCallback(EventCallback callback) { callback_onMousePressed = callback; }
				inline virtual void setMouseReleasedCallback(EventCallback callback) { callback_onMouseReleased = callback; }
				inline virtual void setMouseMovedCallback(EventCallback callback) { callback_onMouseMoved = callback; }
				inline virtual void setDragAndDropCallback(EventCallback callback) { callback_onDragAndDrop = callback; }
				inline virtual void setMouseEnteredCallback(EventCallback callback) { callback_onMouseEntered = callback; }
				inline virtual void setMouseExitedCallback(EventCallback callback) { callback_onMouseExited = callback; }
				inline virtual void setMouseDraggedCallback(EventCallback callback) { callback_onMouseDragged = callback; }
				inline virtual void setKeyPressedCallback(EventCallback callback) { callback_onKeyPressed = callback; }
				inline virtual void setKeyReleasedCallback(EventCallback callback) { callback_onKeyReleased = callback; }
				inline virtual void setTextEnteredCallback(EventCallback callback) { callback_onTextEntered = callback; }
				inline virtual void setWindowClosedCallback(EventCallback callback) { callback_onWindowClosed = callback; }
				inline virtual void setWindowResizedCallback(EventCallback callback) { callback_onWindowResized = callback; }
				inline virtual void setWindowFocusedCallback(EventCallback callback) { callback_onWindowFocused = callback; }
				inline virtual void setWindowFocusLostCallback(EventCallback callback) { callback_onWindowFocusLost = callback; }

				inline bool hasChildren(void) const { return m_allowChildren && m_widgets.widgetCount() > 0; }
				inline virtual bool isInvalid(void) const override { return ostd::BaseObject::isInvalid() || (m_parent == nullptr && !m_rootChild); }
				inline void setTabIndex(int32_t tabIndex) { m_tabIndex = tabIndex; }
				inline int32_t getTabIndex(void) const { return m_tabIndex; }
				inline bool isFocused(void) const { return m_focused; }
				inline WindowCore& getWindow(void) { return *m_window; }
				inline Widget* getParent(void) { return m_parent; }
				inline int32_t getZIndex(void) const { return m_zIndex; }
				inline bool isChildrenEnabled(void) const { return m_allowChildren; }
				inline void setPadding(const ostd::Rectangle& pad) { m_padding = pad; }
				inline Rectangle getPadding(void) const { return m_padding; }
				inline bool isMouseInside(void) const { return m_mouseInside; }
				inline ogfx::MouseEventData::eButton getPressedMouseButton(void) const { return m_pressedButton; }
				inline const std::vector<ostd::String>& getThemeIDList(void) const { return m_themeIDList; }
				inline bool isEnabled(void) const { return m_enabled; }
				inline bool isFocusEnabled(void) const { return m_allowFocus; }
				inline void enableFocus(bool enable = true) { m_allowFocus = enable; }
				inline void disableFocus(void) { enableFocus(false); }
				inline void disable(void) { enable(false); }
				inline void enableStopEvents(bool enable = true) { m_stopEvents = enable; }
				inline bool isDragAndDropEnabled(void) const { return m_acceptDragAndDrop; }
				inline void enableDragAndDrop(bool enable = true) { m_acceptDragAndDrop = enable; }
				inline void disableDragAndDrop(void) { enableDragAndDrop(false); }

				template<typename T>
				inline T getThemeValue(const ostd::Stylesheet &theme, const ostd::String& key, const T& fallback)
				{
					return theme.get<T>(key,  fallback, getThemeIDList(), getThemeQualifierList());
				}

				inline static void setDragAndDropData(ostd::BaseObject& data) { s_dragAndDropData = &data; s_hasDragAndDropData = true; }
				inline static void clearDragAndDropData(void) { s_dragAndDropData = nullptr; }
				inline static ostd::BaseObject* getDragAndDropData(void) { return s_dragAndDropData; }

			protected:
				inline void disableChildren(void) { m_allowChildren = false; }
				inline void disableDrawBox(void) { m_drawBox = false; }
				inline void enableDrawBox(void) { m_drawBox = true; }
				inline bool isDrawBoxEnabled(void) const { return m_drawBox; }
				inline void setDrawBoxColor(const ostd::Color& color) { m_drawBoxColor = color; }
				inline ostd::Color getDrawBoxColor(void) { return m_drawBoxColor; }

			protected:
				bool m_rootChild { false };
				int32_t m_borderRadius { 10 };
				int32_t m_borderWidth { 2 };
				ostd::Color m_borderColor { 255, 255, 255 };
				bool m_showBorder { false };

				EventCallback callback_onMousePressed { nullptr };
				EventCallback callback_onMouseReleased { nullptr };
				EventCallback callback_onDragAndDrop { nullptr };
				EventCallback callback_onMouseMoved { nullptr };
				EventCallback callback_onMouseEntered { nullptr };
				EventCallback callback_onMouseExited { nullptr };
				EventCallback callback_onMouseDragged { nullptr };
				EventCallback callback_onKeyPressed { nullptr };
				EventCallback callback_onKeyReleased { nullptr };
				EventCallback callback_onTextEntered { nullptr };
				EventCallback callback_onWindowClosed { nullptr };
				EventCallback callback_onWindowResized { nullptr };
				EventCallback callback_onWindowFocused { nullptr };
				EventCallback callback_onWindowFocusLost { nullptr };

			private:
				WindowCore* m_window { nullptr };
				Widget* m_parent { nullptr };
				bool m_focused { false };
				int32_t m_tabIndex { -1 };
				int32_t m_zIndex { -1 };
				WidgetManager m_widgets;
				bool m_allowChildren { true };
				bool m_mouseInside { false };
				bool m_allowFocus { true };
				bool m_enabled { true };
				bool m_stopEvents { true };
				bool m_clipContents { true };
				bool m_acceptDragAndDrop { false };
				MouseEventData::eButton m_pressedButton { MouseEventData::eButton::None };

				std::vector<ostd::String> m_themeIDList;
				ostd::Stylesheet::QualifierList m_qualifierList {
					{ "disabled", false },
					{ "pressed", false },
					{ "hover", false },
					{ "focused", false },
					{ "active", false }
				};
				std::vector<ThemeOverride> m_themeOverrides;

				bool m_drawBox { true };
				ostd::Color m_drawBoxColor { 255, 0, 0 };

				ostd::Rectangle m_padding { 0, 0, 0, 0 };

				public:
				static ostd::BaseObject* s_dragAndDropData;
				static bool s_hasDragAndDropData;

				friend class WidgetManager;
		};
	}
}
