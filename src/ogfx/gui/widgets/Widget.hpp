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
#include <ostd/utils/Defines.hpp>
#include <functional>

namespace ogfx
{
	namespace gui
	{
		class Widget : public ostd::BaseObject, public Rectangle
		{
			private: struct ThemeOverride
			{
				String fullKey;
				ostd::Stylesheet::TypeVariant value;
			};
			public: enum class eCallback
			{
				MousePressed = 0,
				MouseReleased,
				MouseMoved,
				MouseScrolled,
				DragAndDrop,
				MouseEntered,
				MouseExited,
				MouseDragged,
				KeyPressed,
				KeyReleased,
				TextEntered,
				WindowClosed,
				WindowResized,
				WindowFocused,
				WindowFocusLost
			};
			public: using EventCallback = std::function<void(const Event&)>;
			public:
				// ================================== MAIN =================================
				Widget(const Rectangle& bounds, WindowCore& window);
				bool addWidget(Widget& child, const Vec2& position = { 0, 0 }, bool __skip_callback = false);
				bool removeWidget(Widget& child);
				void enable(bool enable = true);
				void addThemeOverride(const String& fullKey, ostd::Stylesheet::TypeVariant value);
				void reloadTheme(bool propagate = false);
				void setThemeQualifier(const String& qualifier, bool value = true);
				bool getThemeQualifier(const String& qualifier) const;
				bool addThemeID(const String& id);
				bool removeThemeID(const String& id);
				void setVisible(bool v);
				void setCallback(eCallback type, EventCallback callback);
				using Rectangle::contains; bool contains(Vec2 p, bool includeBounds = false) const override;
				template<typename T>
				inline T getThemeValue(const ostd::Stylesheet &theme, const String& key, const T& fallback)
				{
					return theme.get<T>(property(key), fallback, getThemeIDList(), getThemeQualifierList());
				}
				// ==========================================================================



				// ================================== CORE =================================
				void __draw(ogfx::BasicRenderer2D& gfx);
				void __update(void);
				void __onMousePressed(const Event& event);
				void __onMouseReleased(const Event& event);
				void __onDragAndDrop(const Event& event);
				void __onMouseMoved(const Event& event);
				void __onMouseScrolled(const Event& event);
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
				// ==========================================================================



				// =============================== DIMENSIONS ===============================
				virtual Vec2 getGlobalPosition(void) const;
				virtual Vec2 getGlobalContentPosition(void) const;
				virtual Rectangle getGlobalBounds(void) const;
				virtual Rectangle getContentBounds(void) const;
				virtual Rectangle getPureContentBounds(void) const;
				virtual Rectangle getGlobalContentBounds(void) const;
				virtual Rectangle getGlobalPureContentBounds(void) const;
				virtual Rectangle getContentExtents(void) const;
				// ==========================================================================



				// ================================= GETSET =================================
				inline static void setDragAndDropData(ostd::BaseObject& data) { s_dragAndDropData = &data; s_hasDragAndDropData = true; }
				inline static void clearDragAndDropData(void) { s_dragAndDropData = nullptr; }
				inline static ostd::BaseObject* getDragAndDropData(void) { return s_dragAndDropData; }
				inline WindowCore& getWindow(void) { return *m_window; }
				inline Widget* getParent(void) { return m_parent; }
				inline i32 getZIndex(void) const { return m_zIndex; }
				inline ogfx::MouseEventData::eButton getPressedMouseButton(void) const { return m_pressedButton; }
				inline const stdvec<String>& getThemeIDList(void) const { return m_themeIDList; }
				inline const ostd::Stylesheet::QualifierList& getThemeQualifierList(void) const { return m_qualifierList; }
				inline String getStylesheetCategoryName(void) const { return m_stylesheetCategoryName; }
				OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
				OSTD_PARAM_GETSET(Color, BackgroundColor, m_backgroundColor);
				OSTD_PARAM_GETSET(Color, BorderColor, m_borderColor);
				OSTD_PARAM_GETSET(i32, BorderRadius, m_borderRadius);
				OSTD_PARAM_GETSET(i32, BorderWidth, m_borderWidth);
				OSTD_PARAM_GETSET(i32, FontSize, m_fontSize);
				OSTD_PARAM_GETSET(ostd::ColorGradient, BackgroundGradient, m_backgroundGradient);
				OSTD_PARAM_GETSET(Rectangle, Padding, m_padding);
				OSTD_PARAM_GETSET(Rectangle, Margin, m_margin);
				OSTD_PARAM_GETSET(i32, TabIndex, m_tabIndex);
				OSTD_PARAM_GETSET(Vec2, ContentOffset, m_contentOffset);

				// BOOL PARAMETERS
				inline bool isVisible(void) const { return m_visible; }
				inline void show(void) { setVisible(true); }
				inline void hide(void) { setVisible(false); }
				inline bool isChildrenEnabled(void) const { return m_allowChildren; }
				inline bool isEnabled(void) const { return m_enabled; }
				inline void disable(void) { enable(false); }
				inline bool isFocused(void) const { return m_focused; }
				inline bool isMouseInside(void) const { return m_mouseInside; }
				inline bool hasChildren(void) const { return m_allowChildren && m_widgets.widgetCount() > 0; }
				inline bool isRootChild(void) const { return m_rootChild; }
				inline void setRootChild(void) { m_rootChild = true; }
				inline virtual bool isInvalid(void) const override { return ostd::BaseObject::isInvalid() || (m_parent == nullptr && !m_rootChild); }
				OSTD_BOOL_PARAM_GETSET_E(Focus, m_allowFocus);
				OSTD_BOOL_PARAM_GETSET_E(StopEvents, m_stopEvents);
				OSTD_BOOL_PARAM_GETSET_E(DragAndDrop, m_acceptDragAndDrop);
				OSTD_BOOL_PARAM_GETSET_E(Background, m_showBackground);
				OSTD_BOOL_PARAM_GETSET_E(Border, m_showBorder);
				OSTD_BOOL_PARAM_GETSET_E(ManualDraw, m_manualDraw);
				OSTD_BOOL_PARAM_GETSET_E(TopMost, m_topMost);
				OSTD_BOOL_PARAM_GETSET_E(IgnoreScroll, m_ignoreScroll);
				OSTD_BOOL_PARAM_GETSET_E(VScroll, m_vScrollEnabled);
				OSTD_BOOL_PARAM_GETSET_E(HScroll, m_vScrollEnabled);
				OSTD_BOOL_PARAM_GETSET_E(BackgroundGradient, m_useBackgroundGradient);
				OSTD_BOOL_PARAM_GETSET_E(Theming, m_enableTheming);
				// ==========================================================================

			protected:
				void apply_common_theme_values(const ostd::Stylesheet& theme);
				inline void disableChildren(void) { m_allowChildren = false; }
				inline void setStylesheetCategoryName(const String& name) { m_stylesheetCategoryName = name; }
				inline String property(const ostd::String& prop) const { return m_stylesheetCategoryName +  "." + prop; }

			private:
				// ======= CORE =======
				WindowCore* m_window { nullptr };
				Widget* m_parent { nullptr };
				WidgetManager m_widgets;

				bool m_rootChild { false };
				bool m_focused { false };
				i32 m_tabIndex { -1 };
				i32 m_zIndex { -1 };
				MouseEventData::eButton m_pressedButton { MouseEventData::eButton::None };
				// ====================



				// ======= BOOL =======
				bool m_allowChildren { true };
				bool m_mouseInside { false };
				bool m_allowFocus { true };
				bool m_enabled { true };
				bool m_stopEvents { true };
				bool m_clipContents { true };
				bool m_acceptDragAndDrop { false };
				bool m_visible { true };
				bool m_ignoreScroll { false};
				bool m_manualDraw { false };
				bool m_topMost { false };
				bool m_vScrollEnabled { false };
				bool m_hScrollEnabled { false };
				bool m_enableTheming { true };
				// ====================



				// ====== THEMING =====
				String m_stylesheetCategoryName { "" };
				stdvec<String> m_themeIDList;
				ostd::Stylesheet::QualifierList m_qualifierList {
					{ "disabled", false },
					{ "pressed", false },
					{ "hover", false },
					{ "focused", false },
					{ "active", false }
				};
				stdvec<ThemeOverride> m_themeOverrides;
				Rectangle m_padding { 0, 0, 0, 0 };
				Rectangle m_margin { 0, 0, 0, 0 };
				Vec2 m_contentOffset { 0, 0 };
				i32 m_borderRadius { 10 };
				i32 m_borderWidth { 2 };
				Color m_borderColor { 255, 255, 255 };
				bool m_showBorder { false };
				Color m_backgroundColor { Colors::Transparent };
				bool m_showBackground { false };
				i32 m_fontSize { 20 };
				Color m_textColor { 255, 255, 255 };
				bool m_useBackgroundGradient { false };
				ColorGradient m_backgroundGradient {
					{ Color { 160, 160, 160 }, Color { 120, 120, 120 } },
					{ 1.0f }
				};
				// ====================



				// ====== STATIC ======
				static ostd::BaseObject* s_dragAndDropData;
				static bool s_hasDragAndDropData;
				friend class WidgetManager;
				// ====================




			//  =============================== CALLBACKS/DUMMIES ===============================
			protected:
				EventCallback callback_onMousePressed { nullptr };
				EventCallback callback_onMouseReleased { nullptr };
				EventCallback callback_onDragAndDrop { nullptr };
				EventCallback callback_onMouseMoved { nullptr };
				EventCallback callback_onMouseScrolled { nullptr };
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

			public:
				// ====================== DUMMIES ======================
				inline virtual void applyTheme(const ostd::Stylesheet& theme) {  }
				inline virtual Vec2 getScrollOffset(void) const { return { 0, 0 }; }
				inline virtual void setScrollOffset(const Vec2& offset) {  }
				inline virtual void addScrollOffset(const Vec2& offset) {  }
				inline virtual bool needsVScroll(void) const { return false; }
				inline virtual bool needsHScroll(void) const { return false; }
				inline virtual f32 getVScrollbarSize(void) const { return 0; }
				inline virtual f32 getHScrollbarSize(void) const { return 0; }
				// =====================================================

				inline virtual void onDraw(ogfx::BasicRenderer2D& gfx) {  }
				inline virtual void afterDraw(ogfx::BasicRenderer2D& gfx) {  }
				inline virtual void beforeDraw(ogfx::BasicRenderer2D& gfx) {  }
				inline virtual void onUpdate(void) {  }
				inline virtual void onWidgetAdded(Widget& child) {  }
				inline virtual void onMousePressed(const Event& event) {  }
				inline virtual void onMouseReleased(const Event& event) {  }
				inline virtual void onMouseMoved(const Event& event) {  }
				inline virtual void onMouseScrolled(const Event& event) {  }
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
		};
	}
}
