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

#include <ogfx/utils/SDLInclude.hpp>
#include <ostd/utils/Signals.hpp>
#include <ostd/utils/Time.hpp>
#include <ostd/io/IOHandlers.hpp>
#include <ogfx/gui/Events.hpp>
#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/widgets/RootWidget.hpp>
#include <ogfx/render/BasicRenderer.hpp>
#include <ogfx/gui/WindowOutputHandler.hpp>
#include <ogfx/gui/ContextMenu.hpp>
#include <ogfx/gui/MenuBar.hpp>
#include <ogfx/gui/ToolBar.hpp>

namespace ogfx
{
	class WindowCore : public ostd::BaseObject
	{
		public: struct FileDialogFilter
		{
			String description { "" };
			stdvec<String> extensionList;
		};
		public: enum class eCursor : u8
		{
			Default = 0,
			Text,
			Wait,
			Crosshair,
			Progress,
			NWSE_Resize,
			NESW_Resize,
			EW_Resize,
			NS_Resize,
			Move,
			NotAllowed,
			Pointer,
			NW_Resize,
			N_Resize,
			NE_Resize,
			E_Resize,
			SE_Resize,
			S_Resize,
			SW_Resize,
			W_Resize,

			Count,
			NoCursor = 255
		};
		public: enum class eTextAlign : u8 {
			Default = 0,
			Center,
			Left
		};
		public: enum class eWindowState : u8 {
			Default = 0,
			Maximized,
			Minimized
		};
		public: struct State
		{
			bool fullscreen { false };
			bool borderless { false };
			bool resizable { false };
			bool minimized { false };
			bool maximized { false };
			bool alwaysOnTop { false };
		};
		public: using FileDialogFilterList = stdvec<ogfx::WindowCore::FileDialogFilter>;
		public:
			inline WindowCore(void) {  }
			virtual ~WindowCore(void);
			inline WindowCore(i32 width, i32 height, const String& title) { initialize(width, height, title); }
			void initialize(i32 width, i32 height, const String& title);
			void mainLoop(void);
			void close(void);
			void setSize(i32 width, i32 height);
			void setTitle(const String& title);
			void setCursor(eCursor cursor);
			void setUserScale(f32 scale);
			void setPosition(const IPoint& pos);
			void setWindowState(eWindowState state);
			State getWindowState(void) const;
			eCursor getCurosr(void) const;
			void enableResizable(bool enable = true);
			void setIcon(const String& iconFilePath);
			void setBlockingEventsRefreshFPS(u32 fps);
			void requestRedraw(void);
			void handleSignal(ostd::Signal& signal) override;
			stdvec<String> showOpenFileDialog(const FileDialogFilterList& filterList, bool multiselect, const String& defaultPath = "") const;
			stdvec<String> showOpenFolderDialog(bool multiselect, const String& defaultPath = "") const;
			String showSaveFileDialog(const FileDialogFilterList& filterList, const String& defaultPath = "") const;

			inline String showOpenFileDialog(const FileDialogFilterList& filterList) const { auto list = showOpenFileDialog(filterList, false); return (list.size() == 0 ? "" : list[0]); }
			inline String showOpenFolderDialog(void) const { auto list = showOpenFolderDialog(false); return (list.size() == 0 ? "" : list[0]); }
			inline const ostd::Stylesheet* theme(void) const { return m_guiTheme; }
			inline void loadDefaultTHeme(void) { setTheme(DefaultTheme); }
			inline ostd::Stylesheet::VariableList& getDefaultStylesheetVariableList(void) { return m_defaultStylesheetVariables; }
			inline virtual void setTheme(const ostd::Stylesheet& theme) {  }

			inline BasicRenderer2D& getGFX(void) { return m_gfx; }
			inline bool isInitialized(void) const { return m_initialized; }
			inline bool isRunning(void) const { return m_running; }
			inline bool isVisible(void) const { return m_visible; }
			inline bool isResizeable(void) const { return m_resizeable; }
			inline void hide(void) { SDL_HideWindow(m_window); m_visible = false; }
			inline void show(void) { SDL_ShowWindow(m_window); m_visible = true; }
			inline String getTitle(void) const { return m_title; }
			inline i32 getWindowWidth(void) const { return m_windowWidth; }
			inline i32 getWindowHeight(void) const { return m_windowHeight; }
			inline Color getClearColor(void) const { return m_clearColor; }
			inline void setClearColor(const Color& color) { m_clearColor = color; }
			inline SDL_Renderer* getSDLRenderer(void) { return m_renderer; }
			inline void enableBlockingEvents(bool enable = true) { m_blockingEvents = enable; }
			inline bool isBlockingEventsEnabled(void) const { return m_blockingEvents; }
			inline ostd::ConsoleOutputHandler& out(void) { return m_out; }
			inline GraphicsWindowOutputHandler& wout(void) { return m_wout; }
			inline f32 getScaleFactor(void) const { return m_systemScale * m_userScale; }
			inline void updateScalleFactor(void) { setUserScale(m_userScale); }
			inline Vec2 getMousePosition(void) const { return m_mousePosition; }
			inline void startTooltipTimer(const String& text) { m_tooltipText = text; m_tooltipTimer.startCount(ostd::eTimeUnits::Seconds); }
			inline void stopTooltipTimer(void) { m_tooltipText = ""; m_tooltipTimer.endCount(); }
			inline void restartTooltipTimer(void) { m_tooltipTimer.restart(ostd::eTimeUnits::Seconds); }
			inline u64 readTooltipTimer(void) { return m_tooltipTimer.read(); }
			inline bool isTooltipShown(void) { return m_tooltipText.new_trim() != "" && m_tooltipTimer.read() >= 2; }
			inline String getTooltipText(void) const { return m_tooltipText; }

		protected:
			MouseEventData get_mouse_state(SDL_Event& event);
			virtual void handle_events(void);
			virtual void before_render(void);
			virtual void after_render(void);
			inline virtual void __on_event(SDL_Event& event) {  }
			inline virtual void __on_window_init(i32 width, i32 height, const String& title) {  }
			inline virtual void __on_window_destroy(void) {  }
			inline virtual void __on_window_close(void) {  }
			inline virtual void __on_signal(ostd::Signal& signal) {  }
			inline virtual void __on_update(f64 delta) {  }
			inline virtual void __on_fixed_update(void) {  }
			inline virtual void __main_loop(void) = 0;

		private:
			void __handle_event(SDL_Event& event);
			void __load_default_stylesheet_variables(void);
			void __store_file_dialog_filters(const FileDialogFilterList& filters) const;

		protected:
			SDL_Window* m_window { nullptr };
			SDL_Renderer* m_renderer { nullptr };
			BasicRenderer2D m_gfx;
			ostd::ConsoleOutputHandler m_out;
			GraphicsWindowOutputHandler m_wout;
			const ostd::Stylesheet* m_guiTheme { nullptr };

		private:
			Color m_clearColor { 10, 10, 10, 255 };

			i32 m_windowWidth { 0 };
			i32 m_windowHeight { 0 };
			String m_title { "" };
			i32 m_blockingEventsDelay { 33 };
			Vec2 m_mousePosition { 0, 0 };
			ostd::Timer m_tooltipTimer;
			String m_tooltipText { "" };

			bool m_running { false };
			bool m_initialized { false };
			bool m_visible { true };
			bool m_blockingEvents { false };
			bool m_resizeable { true };
			bool m_refreshScreen { true };
			bool m_invertHorizontalScroll { true };

			f32 m_systemScale { 1.0f };
			f32 m_userScale { 1.0f };

			SDL_Cursor* m_cursor_Default      { nullptr };
			SDL_Cursor* m_cursor_Text         { nullptr };
			SDL_Cursor* m_cursor_Wait         { nullptr };
			SDL_Cursor* m_cursor_Crosshair    { nullptr };
			SDL_Cursor* m_cursor_Progress     { nullptr };
			SDL_Cursor* m_cursor_NWSE_Resize  { nullptr };
			SDL_Cursor* m_cursor_NESW_Resize  { nullptr };
			SDL_Cursor* m_cursor_EW_Resize    { nullptr };
			SDL_Cursor* m_cursor_NS_Resize    { nullptr };
			SDL_Cursor* m_cursor_Move         { nullptr };
			SDL_Cursor* m_cursor_NotAllowed   { nullptr };
			SDL_Cursor* m_cursor_Pointer      { nullptr };
			SDL_Cursor* m_cursor_NW_Resize    { nullptr };
			SDL_Cursor* m_cursor_N_Resize     { nullptr };
			SDL_Cursor* m_cursor_NE_Resize    { nullptr };
			SDL_Cursor* m_cursor_E_Resize     { nullptr };
			SDL_Cursor* m_cursor_SE_Resize    { nullptr };
			SDL_Cursor* m_cursor_S_Resize     { nullptr };
			SDL_Cursor* m_cursor_SW_Resize    { nullptr };
			SDL_Cursor* m_cursor_W_Resize     { nullptr };

			mutable stdvec<String> m_patternStorage;
			mutable stdvec<SDL_DialogFileFilter> m_sdlFilters;

		public:
			inline static constexpr i32 MaxBlockingEventsFPS { 240 };
			inline static constexpr i32 DefaultBlockingEventsFPS { 30 };

			inline static const IPoint WindowsPositionCenter { -1, -1 };

		private:
			inline static ostd::Stylesheet DefaultTheme;
			stdumap<String, std::pair<String, bool>> m_defaultStylesheetVariables;

	};
	class GraphicsWindow : public WindowCore
	{
		public:
			inline GraphicsWindow(void) {  }
			inline GraphicsWindow(i32 width, i32 height, const String& title) { initialize(width, height, title); }

			inline virtual void onRender(void) {  }
			inline virtual void onUpdate(void) {  }
			inline virtual void onFixedUpdate(f64 frameTime_s) {  }
			inline virtual void onInitialize(void) {  }
			inline virtual void onDestroy(void) {  }
			inline virtual void onClose(void) { }
			inline virtual void onSDLEvent(SDL_Event& event) { }
			inline virtual void onSignal(ostd::Signal& signal) {  }

			inline i32 getFPS(void) const { return m_fps; }

		protected:
			void __on_window_init(i32 width, i32 height, const String& title) override;
			void __on_event(SDL_Event& event) override;
			void __on_window_destroy(void) override;
			void __on_window_close(void) override;
			void __main_loop(void) override;
			void __on_signal(ostd::Signal& signal) override;

		private:
			i32 m_fps { 0 };
			ostd::StepTimer m_fixedUpdateTImer;
			ostd::StepTimer m_fpsUpdateTimer;
			ostd::Timer m_fpsUpdateClock;
			u64 m_frameTimeAcc { 0 };
			i32 m_frameCount { 0 };
	};
	namespace gui
	{
		class Window : public WindowCore
		{
			public:
				inline Window(void) {  }
				inline Window(i32 width, i32 height, const String& title) { initialize(width, height, title); }
				void addWidget(Widget& widget, const Vec2& position = { 0, 0 });
				void setTheme(const ostd::Stylesheet& theme) override;
				inline void showContextMenu(const Vec2& pos) { m_cmenu.show(pos); }
				inline void setContextMenu(const ContextMenu::Instance& instance) { m_cmenu.setInstance(instance); }
				inline bool isContextMenuVisible(void) const { return m_cmenu.isVisible(); }
				inline bool isMenuBarVisible(void) const { return m_menubar.isVisible(); }
				inline void hideContextMenu(void) { m_cmenu.hide(); }
				inline MenuBar& getMenuBar(void) { return m_menubar; }
				inline ToolBar& getToolBar(void) { return m_toolbar; }

				inline virtual void onInitialize(void) {  }
				inline virtual void onDestroy(void) {  }
				inline virtual void onClose(void) {  }
				inline virtual void onSDLEvent(SDL_Event& event) {  }
				inline virtual void onRedraw(BasicRenderer2D& gfx) {  }
				inline virtual void onUpdate(f64 delta) {  }
				inline virtual void onFixedUpdate(void) {  }
				inline virtual void onSignal(ostd::Signal& signal) {  }

			protected:
				void __on_window_init(i32 width, i32 height, const String& title) override;
				void __on_event(SDL_Event& event) override;
				void __on_window_destroy(void) override;
				void __on_window_close(void) override;
				void __main_loop(void) override;
				void __on_signal(ostd::Signal& signal) override;
				void __on_update(f64 delta) override;
				void __on_fixed_update(void) override;

			protected:
				widgets::RootWidget m_rootWidget { *this };
				ostd::StepTimer m_fixedUpdateTimer;
				ostd::StepTimer::TimePoint m_lastFrameTime;
				ContextMenu m_cmenu { *this };
				MenuBar m_menubar { *this };
				ToolBar m_toolbar { *this };

				friend class widgets::RootWidget;
		};
	}
}
