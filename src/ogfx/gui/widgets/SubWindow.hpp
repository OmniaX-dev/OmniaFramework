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

#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/widgets/Button.hpp>
#include <ogfx/render/RenderTarget.hpp>
#include <ogfx/utils/Animation.hpp>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// SubWindow — a floating window inside the main window.
		//
		//   SubWindow tools { window, "Tools" };
		//   tools.setBounds({ 80, 80, 360, 260 });
		//   tools.addWidget(someWidget);
		//   tools.open();                       // adds it to the window and raises it
		//
		// SubWindows are children of the window's root widget, so theming, focus and
		// events work like for any other widget. They live on z-layers above the
		// regular widgets (see eZLayer) and the window draws them itself, after the
		// widget tree and below the menubar/toolbar/statusbar. Clicking a SubWindow
		// raises it within its layer; the topmost one gets the "active" theme
		// qualifier, so (subwindow:active) styles the focused window.
		//
		// Title bar buttons (setButtons()): close, maximize (fills the area between
		// the bars and follows it when the window is resized), minimize (shrinks the
		// window to its title bar, lined up along the bottom edge) and always-on-top.
		// Dragging the title bar moves the window, double-clicking it maximizes /
		// restores, and the edges and corners resize it.
		//
		// Blur: with blurEnabled (theme or enableBlur()) the window draws whatever is
		// behind it, blurred, under its (ideally translucent) background, like the
		// frosted panel in the blur example. It needs shader support; the blurred
		// area is rectangular, so blurred windows look best with borderRadius = 0.
		//
		// Theme category: "subwindow" (see DefaultThemeDark.oss for every key).
		// ============================================================================
		class SubWindow : public Widget
		{
			public: enum class eZLayer : i32
			{
				Normal = 1,
				AlwaysOnTop = 2,
				Modal = 3
			};
			public: enum class eState : u8
			{
				Normal = 0,
				Maximized,
				Minimized
			};
			public: struct Buttons
			{
				inline static constexpr u8 None = 0;
				inline static constexpr u8 Close = 1;
				inline static constexpr u8 Maximize = 2;
				inline static constexpr u8 Minimize = 4;
				inline static constexpr u8 AlwaysOnTop = 8;
				inline static constexpr u8 All = Close | Maximize | Minimize | AlwaysOnTop;
			};
			// Return false to keep the window open.
			public: using CloseCallback = std::function<bool(SubWindow&)>;
			public: using StateChangedCallback = std::function<void(SubWindow&, eState)>;
			public:
				inline SubWindow(Window& window) : Widget({ 60, 60, 400, 300 }, window) { create("Window"); }
				inline SubWindow(Window& window, const String& title) : Widget({ 60, 60, 400, 300 }, window) { create(title); }
				~SubWindow(void) override;
				SubWindow& create(const String& title);

				// Shows the window (adding it to the window first if needed) and raises it.
				void open(void);
				// Runs the close callback, then hides the window. Windows owned by the
				// gui::Window (message boxes) are destroyed at the start of the next frame.
				void close(void);
				void maximize(void);
				void minimize(void);
				// Back to the normal state (from maximized or minimized).
				void restore(void);
				inline void toggleMaximized(void) { if (isMaximized()) restore(); else maximize(); }
				void bringToFront(void);
				void setAlwaysOnTop(bool alwaysOnTop = true);
				// Modal windows block mouse and keyboard input to everything else in the
				// window (they sit on the top layer, over a theme-colored overlay).
				void setModal(bool modal = true);
				// Centers the window in the area between the bars.
				void centerInParent(void);

				inline eState getState(void) const { return m_state; }
				inline bool isMaximized(void) const { return m_state == eState::Maximized; }
				inline bool isMinimized(void) const { return m_state == eState::Minimized; }
				inline bool isAlwaysOnTop(void) const { return m_alwaysOnTop; }
				inline bool isModal(void) const { return m_modal; }
				inline String getTitle(void) const { return m_title; }
				inline void setTitle(const String& title) { m_title = title; }
				inline u8 getButtons(void) const { return m_buttons; }
				inline void setButtons(u8 buttons) { m_buttons = buttons; }
				inline bool hasButton(u8 button) const { return (m_buttons & button) != 0; }
				inline void setCloseCallback(CloseCallback callback) { m_closeCallback = std::move(callback); }
				inline void setStateChangedCallback(StateChangedCallback callback) { m_stateChangedCallback = std::move(callback); }
				inline f32 getTitlebarHeight(void) const { return m_titlebarHeight; }
				inline Color getModalOverlayColor(void) const { return m_modalOverlayColor; }

				OSTD_BOOL_PARAM_GETSET_E(Resizable, m_resizable);
				OSTD_BOOL_PARAM_GETSET_E(Draggable, m_draggable);
				// Set from code, these win over the theme's blur.enabled / blur.radius /
				// shadow.enabled (which are otherwise re-read on every theme pass).
				inline void enableBlur(bool enable = true) { m_blurEnabled = enable; m_blurEnabledSet = true; }
				inline void disableBlur(void) { enableBlur(false); }
				inline bool isBlurEnabled(void) const { return m_blurEnabled; }
				inline void setBlurRadius(f32 radius) { m_blurRadius = std::max(0.0f, radius); m_blurRadiusSet = true; }
				inline f32 getBlurRadius(void) const { return m_blurRadius; }
				inline void enableShadow(bool enable = true) { m_shadowEnabled = enable; m_shadowEnabledSet = true; }
				inline void disableShadow(void) { enableShadow(false); }
				inline bool isShadowEnabled(void) const { return m_shadowEnabled; }
				OSTD_PARAM_GETSET(Color, BlurTint, m_blurTint);

				i32 getZLayer(void) const override;
				inline bool blocksInputBelow(void) const override { return m_modal; }
				void layoutChildren(void) override;
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void beforeDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMousePressed(const Event& event) override;
				void onMouseReleased(const Event& event) override;
				void onMouseMoved(const Event& event) override;
				void onMouseDragged(const Event& event) override;
				void onMouseExited(const Event& event) override;

				// ===== Used by gui::Window =====
				// True when the window wants the blurred backdrop this frame.
				bool __needsBackdrop(void) const;
				// Copies the part of the backdrop behind the window (plus the blur reach)
				// into the window's own target; beforeDraw() then draws it blurred.
				void __captureBackdrop(ogfx::BasicRenderer2D& gfx, const RenderTarget& backdrop);
				void __drawShadow(ogfx::BasicRenderer2D& gfx);
				// Places a minimized window in the given slot along the bottom edge.
				void __setMinimizedSlot(i32 slot);
				// Keeps a normal window reachable after the parent area shrank.
				void __clampToParent(void);
				inline void __setOwnedByWindow(void) { m_ownedByWindow = true; }
				inline bool __isOwnedByWindow(void) const { return m_ownedByWindow; }

			protected: enum class eButtonId : i8
			{
				None = -1,
				Close = 0,
				Maximize,
				Minimize,
				AlwaysOnTop,
				Count
			};
			protected: struct ResizeEdges
			{
				bool left { false };
				bool top { false };
				bool right { false };
				bool bottom { false };
				inline bool any(void) const { return left || top || right || bottom; }
			};

			protected:
				Rectangle titlebar_rect(void) const;
				// Global rect of a title bar button; empty when the button isn't shown.
				Rectangle button_rect(eButtonId id) const;
				eButtonId button_at(const Vec2& pos) const;
				ResizeEdges resize_edges_at(const Vec2& pos) const;
				void update_cursor(const Vec2& pos);
				void set_cursor(u8 cursor);
				void draw_titlebar(ogfx::BasicRenderer2D& gfx);
				void draw_button(ogfx::BasicRenderer2D& gfx, eButtonId id);
				void activate_button(eButtonId id);
				void save_layout(void);
				void restore_layout(void);
				void set_state(eState state);
				void layer_changed(void);
				Vec2 parent_area_size(void) const;
				Vec2 minimum_window_size(void) const;

			private:
				String m_title { "" };
				eState m_state { eState::Normal };
				u8 m_buttons { Buttons::All };
				bool m_alwaysOnTop { false };
				bool m_modal { false };
				bool m_resizable { true };
				bool m_draggable { true };
				bool m_ownedByWindow { false };
				CloseCallback m_closeCallback { nullptr };
				StateChangedCallback m_stateChangedCallback { nullptr };

				// Normal-state layout, kept while maximized/minimized.
				Anchors m_savedAnchors;
				Offsets m_savedOffsets;
				bool m_hasSavedLayout { false };

				// Interaction
				eButtonId m_hoveredButton { eButtonId::None };
				eButtonId m_pressedButtonId { eButtonId::None };
				bool m_dragging { false };
				ResizeEdges m_resizing;
				Vec2 m_pressMouse { 0, 0 };
				Rectangle m_pressBounds { 0, 0, 0, 0 };
				u64 m_lastTitleClickMs { 0 };
				bool m_cursorSet { false };
				u8 m_currentCursor { 0 };  // WindowCore::eCursor

				// Blur
				bool m_blurEnabled { false };
				bool m_blurEnabledSet { false };
				f32 m_blurRadius { 16.0f };
				bool m_blurRadiusSet { false };
				Color m_blurTint { Colors::White };
				RenderTarget m_blurSnippet;
				Rectangle m_blurDest { 0, 0, 0, 0 };
				Rectangle m_blurSrc { 0, 0, 0, 0 };
				bool m_blurReady { false };

				// ===== Theme =====
				f32 m_titlebarHeight { 32 };
				Color m_titlebarColor { 45, 45, 50 };
				Color m_titlebarBorderColor { 20, 20, 20 };
				i32 m_titlebarBorderWidth { 1 };
				Color m_titleColor { 220, 220, 220 };
				i32 m_titlebarFontSize { 18 };
				i32 m_titleTextAlign { 2 };
				f32 m_buttonSize { 22 };
				f32 m_buttonSpacing { 4 };
				f32 m_buttonBorderRadius { 4 };
				f32 m_buttonGlyphThickness { 2 };
				Color m_buttonColor { Colors::Transparent };
				Color m_buttonHoverColor { 255, 255, 255, 40 };
				Color m_buttonPressedColor { 255, 255, 255, 80 };
				Color m_buttonActiveColor { 47, 176, 0, 160 };
				Color m_buttonGlyphColor { 200, 200, 200 };
				Color m_buttonGlyphHoverColor { 255, 255, 255 };
				Color m_closeHoverColor { 200, 30, 40 };
				Color m_closePressedColor { 150, 20, 30 };
				f32 m_resizeGripSize { 6 };
				Vec2 m_minimumSize { 160, 80 };
				f32 m_minimizedWidth { 220 };
				bool m_shadowEnabled { true };
				bool m_shadowEnabledSet { false };
				i32 m_shadowSize { 10 };
				Color m_shadowColor { 0, 0, 0, 90 };
				Vec2 m_shadowOffset { 0, 3 };
				Color m_modalOverlayColor { 0, 0, 0, 110 };

			public:
				inline static constexpr f32 MinimizedSpacing { 4 };
				inline static constexpr u64 DoubleClickMs { 400 };
		};

		// ============================================================================
		// MessageDialog — a modal SubWindow with a text, an optional icon and an OK
		// button. Usually created through Widget::messageBox() / Window::messageBox(),
		// which hand ownership to the window (it is destroyed once closed).
		// (Not called MessageBox: windows.h defines that name as a macro.)
		// The icon's sprite sheet must outlive the message box.
		// Theme: "subwindow" with the "messagebox" theme ID, so
		// (@messagebox subwindow) { ... } styles message boxes only.
		// ============================================================================
		class MessageDialog : public SubWindow
		{
			public:
				MessageDialog(Window& window, const String& title, const String& text);
				MessageDialog(Window& window, const String& title, const String& text, const Icon& icon);

				void setText(const String& text);
				inline String getText(void) const { return m_text; }
				inline Button& getOkButton(void) { return m_okButton; }

				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;
				// Sizes the box to its text / icon / button.
				void fitToContent(void);

			private:
				void build(void);
				Vec2 text_size(void) const;

			private:
				String m_text { "" };
				stdvec<String> m_lines;
				Animation m_icon;
				bool m_hasIcon { false };
				Button m_okButton;
				f32 m_iconSize { 48 };
				f32 m_spacing { 16 };
				f32 m_lineSpacing { 4 };
				f32 m_minWidth { 280 };
		};
	}
}
