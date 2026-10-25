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

#include <ogfx/gui/Events.hpp>
#include <ogfx/gui/WidgetManager.hpp>
#include <ostd/data/Object.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/io/Stylesheet.hpp>
#include <ostd/utils/Defines.hpp>
#include <ostd/utils/Time.hpp>
#include <ogfx/gui/Layout.hpp>
#include <memory>
#include <functional>

namespace ogfx
{
	struct Icon;
	namespace gui
	{
		class Window;
		class MessageDialog;
		class Widget : public ostd::Object, public Rectangle
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
				FocusLost,
				FocusGained,
				WindowClosed,
				WindowResized,
				WindowFocused,
				WindowFocusLost,
				ActionPerformed
			};
			public: using EventCallback = std::function<void(const Event&)>;
			public:
				// ================================== MAIN =================================
				Widget(const Rectangle& bounds, Window& window);
				Widget(Widget&&) = default;
				Widget& operator=(Widget&&) = delete;
				Widget(const Widget&) = delete;
				Widget& operator=(const Widget&) = delete;
				virtual ~Widget(void);
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
				bool setTabIndex(i32 index);
				virtual void setCallback(eCallback type, EventCallback callback);
				using Rectangle::contains; bool contains(Vec2 p, bool includeBounds = false) const override;
				// Shows a modal message box (a SubWindow owned by the window) with an OK
				// button. The icon overload draws the icon left of the text.
				MessageDialog& messageBox(const String& title, const String& text);
				MessageDialog& messageBox(const String& title, const String& text, const Icon& icon);
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



				// ============================== LAYOUT ==============================
				// See ogfx/gui/Layout.hpp for an overview of the anchor system.

				// Geometry setters: they move/resize the widget and update the offsets
				// so the new rectangle is kept when the parent is later resized.
				void setx(f32 xx) override;
				void sety(f32 yy) override;
				void setw(f32 ww) override;
				void seth(f32 hh) override;
				using Rectangle::setPosition;
				void setSize(const Vec2& size);
				void setSize(f32 ww, f32 hh);
				void setBounds(f32 xx, f32 yy, f32 ww, f32 hh);
				inline void setBounds(const Rectangle& bounds) { setBounds(bounds.x, bounds.y, bounds.w, bounds.h); }

				// Anchors (fractions of the parent's content area). If keepRect is true,
				// offsets are recomputed so the widget doesn't move right now; otherwise
				// the offsets are kept and the widget jumps to the new anchored rect.
				void setAnchors(const Anchors& anchors, bool keepRect = false);
				inline void setAnchors(f32 l, f32 t, f32 r, f32 b, bool keepRect = false) { setAnchors(Anchors { l, t, r, b }, keepRect); }
				void setAnchor(eSide side, f32 value, bool keepRect = false);
				inline const Anchors& getAnchors(void) const { return m_layoutData.anchors; }

				// Pixel offsets from the anchors.
				void setOffsets(const Offsets& offsets);
				inline void setOffsets(f32 l, f32 t, f32 r, f32 b) { setOffsets(Offsets { l, t, r, b }); }
				void setOffset(eSide side, f32 value);
				inline const Offsets& getOffsets(void) const { return m_layoutData.offsets; }

				// Sets anchors AND offsets for a common configuration. margin is the
				// distance (in pixels) kept from the anchored edges.
				void setAnchorPreset(eAnchorPreset preset, ePresetResizeMode resizeMode = ePresetResizeMode::KeepSize, f32 margin = 0.0f);

				// Percentage-based placement: x/y/w/h are fractions (0..1) of the parent's
				// content area, e.g. { 0.1f, 0.1f, 0.5f, 0.8f } = start at 10%/10%, take
				// 50% of the width and 80% of the height. Optional pixel insets.
				void setRelativeBounds(const Rectangle& fractions, const Offsets& insets = { 0, 0, 0, 0 });

				void setGrowDirection(eGrowDirection horizontal, eGrowDirection vertical);
				inline eGrowDirection getHGrowDirection(void) const { return m_layoutData.growH; }
				inline eGrowDirection getVGrowDirection(void) const { return m_layoutData.growV; }

				// Minimum size. getCombinedMinSize() = max(user minimum, content minimum).
				void setMinSize(const Vec2& size);
				inline void setMinSize(f32 ww, f32 hh) { setMinSize(Vec2 { ww, hh }); }
				inline Vec2 getMinSize(void) const { return m_layoutData.minSize; }
				Vec2 getCombinedMinSize(void) const;

				// Container-child properties.
				void setSizeFlags(eSizeFlags horizontal, eSizeFlags vertical);
				inline void setHSizeFlags(eSizeFlags flags) { setSizeFlags(flags, m_layoutData.sizeFlagsV); }
				inline void setVSizeFlags(eSizeFlags flags) { setSizeFlags(m_layoutData.sizeFlagsH, flags); }
				inline eSizeFlags getHSizeFlags(void) const { return m_layoutData.sizeFlagsH; }
				inline eSizeFlags getVSizeFlags(void) const { return m_layoutData.sizeFlagsV; }
				void setStretchRatio(f32 ratio);
				inline f32 getStretchRatio(void) const { return m_layoutData.stretchRatio; }

				inline const LayoutData& getLayoutData(void) const { return m_layoutData; }

				// True when something other than the widget itself decides its size:
				// either the parent is a Container, or the anchors stretch on an axis.
				// Auto-sizing widgets (Button, Label, ...) switch off auto-sizing then.
				bool isLayoutManaged(void) const;

				// Re-applies the anchors of all children (or, for Containers, re-arranges
				// them). Called automatically on resize; call it manually only if you
				// changed something the layout depends on behind the widget's back.
				virtual void layoutChildren(void);

				// Recomputes the widget's own rect from its anchors/offsets.
				void updateLayout(void);

				// Content-based minimum size; widgets with intrinsic content (text,
				// handles, ...) override this. Expressed in widget size (incl. padding).
				inline virtual Vec2 getContentMinSize(void) const { return { 0, 0 }; }

				// Widgets call this when their content minimum may have changed; it
				// notifies the parent Container (or re-applies the anchors) if it did.
				void updateMinimumSize(void);

				// Resizes the widget honoring its grow direction (used by auto-sizing
				// widgets so e.g. a right-anchored label grows towards the left).
				void applyAutoSize(const Vec2& newSize);

				// True for Containers: children are positioned by the parent, not by
				// their anchors.
				inline virtual bool managesChildLayout(void) const { return false; }

				// Sets the rect on behalf of the layout system (Containers): offsets
				// are left untouched. Not meant for regular user code.
				void setLayoutRect(const Rectangle& rect);

				// Converts a content-area size into the matching widget size (adds
				// padding and content offset, mirroring getContentBounds()).
				Vec2 contentToWidgetSize(const Vec2& contentSize) const;

				// Z-layer among siblings: a parent keeps its children sorted by layer
				// (stable), so a higher layer is always drawn above and hit-tested
				// before a lower one. Regular widgets are 0; SubWindows use 1 and up
				// (see eZLayer in SubWindow.hpp). Call the parent's
				// getChildren().sortByLayer() when the value returned changes.
				inline virtual i32 getZLayer(void) const { return 0; }
				// True for widgets that swallow mouse input aimed at the siblings
				// below them (modal SubWindows).
				inline virtual bool blocksInputBelow(void) const { return false; }
				// ====================================================================



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
				inline static void setDragAndDropData(ostd::Object& data) { s_dragAndDropData = &data; s_hasDragAndDropData = true; }
				inline static void clearDragAndDropData(void) { s_dragAndDropData = nullptr; }
				inline static ostd::Object* getDragAndDropData(void) { return s_dragAndDropData; }
				inline i32 getTabIndex(void) const { return m_tabIndex; }
				inline Window& getWindow(void) const { return *m_window; }
				inline Widget* getParent(void) { return m_parent; }
				inline const Widget* getParent(void) const { return m_parent; }
				inline ogfx::MouseEventData::eButton getPressedMouseButton(void) const { return m_pressedButton; }
				inline const stdvec<String>& getThemeIDList(void) const { return m_themeIDList; }
				inline const ostd::Stylesheet::QualifierList& getThemeQualifierList(void) const { return m_qualifierList; }
				inline String getStylesheetCategoryName(void) const { return m_stylesheetCategoryName; }
				inline WidgetManager& getChildren(void) { return m_widgets; }
				OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
				OSTD_PARAM_GETSET(Color, BackgroundColor, m_backgroundColor);
				OSTD_PARAM_GETSET(Color, BorderColor, m_borderColor);
				OSTD_PARAM_GETSET(Color, FocusBorderColor, m_focusBorderColor);
				OSTD_PARAM_GETSET(i32, BorderRadius, m_borderRadius);
				OSTD_PARAM_GETSET(i32, BorderWidth, m_borderWidth);
				OSTD_PARAM_GETSET(i32, FocusBorderWidth, m_focusBorderWidth);
				OSTD_PARAM_GETSET(i32, FontSize, m_fontSize);
				OSTD_PARAM_GETSET(ostd::ColorGradient, BackgroundGradient, m_backgroundGradient);
				inline Rectangle getPadding(void) const { return m_padding; }
				void setPadding(const Rectangle& padding);
				OSTD_PARAM_GETSET(Rectangle, Margin, m_margin);
				inline Vec2 getContentOffset(void) const { return m_contentOffset; }
				void setContentOffset(const Vec2& offset);
				OSTD_PARAM_GETSET(String, TooltipText, m_tooltipText);

				// BOOL PARAMETERS
				inline bool isVisible(void) const { return m_visible; }
				inline void show(void) { setVisible(true); }
				inline void hide(void) { setVisible(false); }
				inline bool isChildrenEnabled(void) const { return m_allowChildren; }
				inline bool isEnabled(void) const { return m_enabled; }
				inline void disable(void) { enable(false); }
				inline bool isFocused(void) const { return m_focused; }
				inline bool isMouseInside(void) const { return m_mouseInside; }
				inline bool hasChildren(void) const { return m_allowChildren && !m_childrenSuspended && m_widgets.widgetCount() > 0; }
				inline bool isRootChild(void) const { return m_rootChild; }
				inline void setRootChild(void) { m_rootChild = true; }
				inline virtual bool isInvalid(void) const override { return ostd::Object::isInvalid() || (m_parent == nullptr && !m_rootChild); }
				OSTD_BOOL_PARAM_GETSET_E(Focus, m_allowFocus);
				OSTD_BOOL_PARAM_GETSET_E(StopEvents, m_stopEvents);
				OSTD_BOOL_PARAM_GETSET_E(DragAndDrop, m_acceptDragAndDrop);
				OSTD_BOOL_PARAM_GETSET_E(Background, m_showBackground);
				OSTD_BOOL_PARAM_GETSET_E(Border, m_showBorder);
				OSTD_BOOL_PARAM_GETSET_E(FocusBorder, m_showFocusBorder);
				OSTD_BOOL_PARAM_GETSET_E(ManualDraw, m_manualDraw);
				OSTD_BOOL_PARAM_GETSET_E(TopMost, m_topMost);
				OSTD_BOOL_PARAM_GETSET_E(IgnoreScroll, m_ignoreScroll);
				OSTD_BOOL_PARAM_GETSET_E(VScroll, m_vScrollEnabled);
				OSTD_BOOL_PARAM_GETSET_E(HScroll, m_hScrollEnabled);
				OSTD_BOOL_PARAM_GETSET_E(BackgroundGradient, m_useBackgroundGradient);
				OSTD_BOOL_PARAM_GETSET_E(Theming, m_enableTheming);
				OSTD_BOOL_PARAM_GETSET_E(Tooltip, m_enableTooltip);
				// ==========================================================================



				// ================================= EVENT ENABLERS =================================
				OSTD_BOOL_PARAM_GETSET_E_NEG(MousePressedEvent, m_disableMousePressed);
				OSTD_BOOL_PARAM_GETSET_E_NEG(MouseReleasedEvent, m_disableMouseReleased);
				OSTD_BOOL_PARAM_GETSET_E_NEG(MouseMovedEvent, m_disableMouseMoved);
				OSTD_BOOL_PARAM_GETSET_E_NEG(MouseScrolledEvent, m_disableMouseScrolled);
				OSTD_BOOL_PARAM_GETSET_E_NEG(MouseEnteredEvent, m_disableMouseEntered);
				OSTD_BOOL_PARAM_GETSET_E_NEG(MouseExitedEvent, m_disableMouseExited);
				OSTD_BOOL_PARAM_GETSET_E_NEG(MouseDraggedEvent, m_disableMouseDragged);
				OSTD_BOOL_PARAM_GETSET_E_NEG(KeyPressedEvent, m_disableKeyPressed);
				OSTD_BOOL_PARAM_GETSET_E_NEG(KeyReleasedEvent, m_disableKeyReleased);
				OSTD_BOOL_PARAM_GETSET_E_NEG(TextEnteredEvent, m_disableTextEntered);
				// ==================================================================================

			protected:
				void apply_common_theme_values(const ostd::Stylesheet& theme);
				inline void disableChildren(void) { m_allowChildren = false; }
				// Suspended children stay attached but are neither drawn nor sent
				// events (used by minimized SubWindows).
				inline void suspendChildren(bool suspend = true) { m_childrenSuspended = suspend; }
				inline bool areChildrenSuspended(void) const { return m_childrenSuspended; }
				inline void setStylesheetCategoryName(const String& name) { m_stylesheetCategoryName = name; }
				inline String property(const ostd::String& prop) const { return m_stylesheetCategoryName +  "." + prop; }

			private:
				Vec2 parent_content_size(void) const;
				void sync_offsets_h(void);
				void sync_offsets_v(void);
				void apply_anchors(bool force);
				void notify_size_changed(void);
				void begin_size_batch(void);
				void end_size_batch(void);
				void parent_child_list_changed(void);
				// Forgets hover/press state (recursively). Hidden widgets get no mouse-exit
				// events, so without this a widget hidden under the cursor would still think
				// the mouse is over it when shown again (and e.g. a Button would react to the
				// next mouse release anywhere).
				void reset_mouse_state(void);

			private:
				// ======= CORE =======
				Window* m_window { nullptr };
				Widget* m_parent { nullptr };
				WidgetManager m_widgets;
				LayoutData m_layoutData;
				Vec2 m_lastMinSize { 0, 0 };
				bool m_applyingLayout { false };
				i32 m_sizeBatchDepth { 0 };
				bool m_sizeChangedPending { false };

				bool m_rootChild { false };
				bool m_focused { false };
				i32 m_tabIndex { -1 };
				String m_tooltipText { "" };
				MouseEventData::eButton m_pressedButton { MouseEventData::eButton::None };
				// ====================



				// ======= BOOL =======
				bool m_allowChildren { true };
				bool m_childrenSuspended { false };
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
				bool m_enableTooltip { false };
				// ====================



				// ======= Event Enablers =======
				bool m_disableMousePressed { false };
				bool m_disableMouseReleased { false };
				bool m_disableMouseMoved { false };
				bool m_disableMouseScrolled { false };
				bool m_disableMouseEntered { false };
				bool m_disableMouseExited { false };
				bool m_disableMouseDragged { false };
				bool m_disableKeyPressed { false };
				bool m_disableKeyReleased { false };
				bool m_disableTextEntered { false };
				// ==============================



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
				i32 m_borderRadius { 0 };
				i32 m_borderWidth { 2 };
				Color m_borderColor { 255, 255, 255 };
				bool m_showBorder { false };
				i32 m_focusBorderWidth { 2 };
				Color m_focusBorderColor { 0, 150, 20 };
				bool m_showFocusBorder { false };
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
				static ostd::Object* s_dragAndDropData;
				static bool s_hasDragAndDropData;
				friend class WidgetManager;
				friend class FocusManager;
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
				EventCallback callback_onFocusGained { nullptr };
				EventCallback callback_onFocusLost { nullptr };
				EventCallback callback_onWindowClosed { nullptr };
				EventCallback callback_onWindowResized { nullptr };
				EventCallback callback_onWindowFocused { nullptr };
				EventCallback callback_onWindowFocusLost { nullptr };
				EventCallback callback_onActionPerformed { nullptr };

			public:
				// ====================== DUMMIES ======================
				inline virtual void applyTheme(const ostd::Stylesheet& theme) {  }
				inline virtual Vec2 getScrollOffset(void) const { return { 0, 0 }; }
				inline virtual void setScrollOffset(const Vec2& offset) {  }
				inline virtual void addScrollOffset(const Vec2& offset) {  }
				inline virtual bool needsVScroll(void) const { return false; }
				inline virtual bool needsHScroll(void) const { return false; }
				inline virtual void resetScroll(bool horizontal = true, bool vertical = true, bool propagate = true) {  }
				inline virtual f32 getVScrollbarSize(void) const { return 0; }
				inline virtual f32 getHScrollbarSize(void) const { return 0; }
				inline virtual void refresh(void) {  }
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
				inline virtual void onBoundsChanged(const Vec2& newSize) {  }
		};
	}
}
