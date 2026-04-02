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

#include <ostd/data/BaseObject.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/data/Color.hpp>
#include <ogfx/gui/Themes.hpp>
#include <functional>

namespace ogfx
{
	class BasicRenderer2D;
	class WindowCore;
	namespace gui
	{
		class Event;
		class Widget;
		class WidgetManager
		{
			public:
				WidgetManager(WindowCore& window, Widget& owner);
				bool hasWidget(Widget& widget);
				bool requestFocus(Widget& widget);
				bool addWidget(Widget& widget);
				Widget* focusNext(void);

				void draw(ogfx::BasicRenderer2D& gfx);
				void update(void);
				void onThemeApplied(const Theme& theme);
				void onMousePressed(const Event& event);
				void onMouseReleased(const Event& event);
				void onMouseMoved(const Event& event);
				void onMouseDragged(const Event& event);
				void onKeyPressed(const Event& event);
				void onKeyReleased(const Event& event);
				void onTextEntered(const Event& event);
				void onWindowClosed(const Event& event);
				void onWindowResized(const Event& event);
				void onWindowFocused(const Event& event);
				void onWindowFocusLost(const Event& event);

				inline int32_t widgetCount(void) const { return m_widgetList.size(); }
				inline WindowCore& getWindow(void) { return m_window; }

			private:
				WindowCore& m_window;
				Widget& m_owner;
				std::vector<Widget*> m_widgetList;
				Widget* m_focused { nullptr };
				bool m_tabNavigationEnabled { true };
				Widget* m_mousePressedOnWidget { nullptr };
		};
		class Widget : public ostd::BaseObject, public ostd::Rectangle
		{
			public: using EventCallback = std::function<void(const Event&)>;
			public:
				Widget(const ostd::Rectangle& bounds, WindowCore& window);
				bool addChild(Widget& child);
				ostd::Vec2 getGlobalPosition(void) const;
				virtual void applyTheme(const Theme& theme) = 0;
				void applyThemeValue(Theme& theme, const ostd::String& key, Theme::ThemeValue value, bool propagate);

				inline virtual void onDraw(ogfx::BasicRenderer2D& gfx) {  }
				inline virtual void onUpdate(void) {  }

				inline virtual void onMousePressed(const Event& event) {  }
				inline virtual void onMouseReleased(const Event& event) {  }
				inline virtual void onMouseMoved(const Event& event) {  }
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
				void __onMouseMoved(const Event& event);
				void __onMouseDragged(const Event& event);
				void __onKeyPressed(const Event& event);
				void __onKeyReleased(const Event& event);
				void __onTextEntered(const Event& event);
				void __onWindowClosed(const Event& event);
				void __onWindowResized(const Event& event);
				void __onWIndowFocused(const Event& event);
				void __onWindowFocusLost(const Event& event);
				void __applyTheme(const Theme& theme, bool propagate);

				inline virtual void setMousePressedCallback(EventCallback callback) { callback_onMousePressed = callback; }
				inline virtual void setMouseReleasedCallback(EventCallback callback) { callback_onMouseReleased = callback; }
				inline virtual void setMouseMovedCallback(EventCallback callback) { callback_onMouseMoved = callback; }
				inline virtual void setMouseDraggedCallback(EventCallback callback) { callback_onMouseDragged = callback; }
				inline virtual void setKeyPressedCallback(EventCallback callback) { callback_onKeyPressed = callback; }
				inline virtual void setKeyReleasedCallback(EventCallback callback) { callback_onKeyReleased = callback; }
				inline virtual void setTextEnteredCallback(EventCallback callback) { callback_onTextEntered = callback; }
				inline virtual void setWindowClosedCallback(EventCallback callback) { callback_onWindowClosed = callback; }
				inline virtual void setWindowResizedCallback(EventCallback callback) { callback_onWindowResized = callback; }
				inline virtual void setWIndowFocusedCallback(EventCallback callback) { callback_onWindowFocused = callback; }
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
				inline Rectangle getPadding(void) { return m_padding; }

			protected:
				inline void disableChildren(void) { m_allowChildren = false; }
				inline void disableDrawBox(void) { m_drawBox = false; }
				inline void enableDrawBox(void) { m_drawBox = true; }
				inline bool isDrawBoxEnabled(void) const { return m_drawBox; }
				inline void setDrawBoxColor(const ostd::Color& color) { m_drawBoxColor = color; }
				inline ostd::Color getDrawBoxColor(void) { return m_drawBoxColor; }

			protected:
				bool m_rootChild { false };

				EventCallback callback_onMousePressed { nullptr };
				EventCallback callback_onMouseReleased { nullptr };
				EventCallback callback_onMouseMoved { nullptr };
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

				bool m_drawBox { true };
				ostd::Color m_drawBoxColor { 255, 0, 0 };

				ostd::Rectangle m_padding { 0, 0, 0, 0 };

				friend class WidgetManager;
		};
		namespace widgets
		{
			class RootWidget : public Widget
			{
				public:
					RootWidget(WindowCore& window);
					void onWindowResized(const Event& event) override;
					void applyTheme(const Theme& theme) override;

				private:
			};
			class Label : public Widget
			{
				public:
					inline Label(WindowCore& window) : Widget({ 0, 0, 0, 0 }, window) { create(""); }
					inline Label(WindowCore& window, const ostd::String& text) : Widget({ 0, 0, 0, 0 }, window) { create(text); }
					Label& create(const ostd::String& text);
					void applyTheme(const Theme& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void setText(const ostd::String& text);
					inline ostd::String getText(void) const { return m_text; }
					inline ostd::Color getColor(void) const { return m_color; }
					inline void setColor(const ostd::Color& color) { m_color = color; }
					inline int32_t getFontSize(void) const { return m_fontSize; }
					inline void setFontSize(int32_t fontSize) { m_fontSize = fontSize; }
					inline void setBackGroundColor(const ostd::Color& color) { m_backgroundColor = color; }
					inline ostd::Color getBackgroundColor(const ostd::Color& color) { return m_backgroundColor; }
					inline void enableBackground(bool enable = true) { m_showBackground = enable; }
					inline bool isBackgoundEnabled(void) const { return m_showBackground; }

				private:
					void __update_size(ogfx::BasicRenderer2D& gfx);

				private:
					ostd::String m_text { "" };
					bool m_textChanged { false };
					int32_t m_fontSize { 20 };
					ostd::Color m_color { 255, 255, 255 };
					int32_t m_borderRadius { 10 };
					int32_t m_borderWidth { 2 };
					bool m_showBorder { false };
					bool m_showBackground { false };
					ostd::Color m_backgroundColor { ostd::Colors::Transparent };
					ostd::Color m_borderColor { 255, 255, 255 };

				private:
			};
		}
	}
}
