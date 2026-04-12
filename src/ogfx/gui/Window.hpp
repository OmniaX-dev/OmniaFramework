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

namespace ogfx
{
	class WindowCore : public ostd::BaseObject
	{
		public: enum class eCursor : uint8_t
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

		    Count
		};
		public:
			inline WindowCore(void) {  }
			virtual ~WindowCore(void);
			inline WindowCore(int32_t width, int32_t height, const ostd::String& title) { initialize(width, height, title); }
			void initialize(int32_t width, int32_t height, const ostd::String& title);
			void mainLoop(void);
			void close(void);
			void setSize(int32_t width, int32_t height);
			void setTitle(const ostd::String& title);
			void setCursor(eCursor cursor);
			void enableResizable(bool enable = true);
			void setIcon(const ostd::String& iconFilePath);
			void setBlockingEventsRefreshFPS(uint32_t fps);
			void requestRedraw(void);
			void handleSignal(ostd::Signal& signal) override;

			inline const ostd::Stylesheet* theme(void) const { return m_guiTheme; }
			inline void loadDefaultTHeme(void) { setTheme(DefaultTheme); }
			inline ostd::Stylesheet::VariableList& getDefaultStylesheetVariableList(void) { return m_defaultStylesheetVariables; }
			inline virtual void setTheme(const ostd::Stylesheet& theme) {  }

			inline bool isInitialized(void) const { return m_initialized; }
			inline bool isRunning(void) const { return m_running; }
			inline bool isVisible(void) const { return m_visible; }
			inline bool isResizeable(void) const { return m_resizeable; }
			inline void hide(void) { SDL_HideWindow(m_window); m_visible = false; }
			inline void show(void) { SDL_ShowWindow(m_window); m_visible = true; }
			inline ostd::String getTitle(void) const { return m_title; }
			inline int32_t getWindowWidth(void) const { return m_windowWidth; }
			inline int32_t getWindowHeight(void) const { return m_windowHeight; }
			inline ostd::Color getClearColor(void) const { return m_clearColor; }
			inline void setClearColor(const ostd::Color& color) { m_clearColor = color; }
			inline SDL_Renderer* getSDLRenderer(void) { return m_renderer; }
			inline void enableBlockingEvents(bool enable = true) { m_blockingEvents = enable; }
			inline bool isBlockingEventsEnabled(void) const { return m_blockingEvents; }
			inline ostd::ConsoleOutputHandler& out(void) { return m_out; }
			inline GraphicsWindowOutputHandler& wout(void) { return m_wout; }

		protected:
			MouseEventData get_mouse_state(void);
			virtual void handle_events(void);
			virtual void before_render(void);
			virtual void after_render(void);
			inline virtual void __on_event(SDL_Event& event) {  }
			inline virtual void __on_window_init(int32_t width, int32_t height, const ostd::String& title) {  }
			inline virtual void __on_window_destroy(void) {  }
			inline virtual void __on_window_close(void) {  }
			inline virtual void __on_signal(ostd::Signal& signal) {  }
			inline virtual void __main_loop(void) = 0;

		private:
			void __handle_event(SDL_Event& event);
			void __load_default_stylesheet_variables(void);

		protected:
			SDL_Window* m_window { nullptr };
			SDL_Renderer* m_renderer { nullptr };
			ostd::ConsoleOutputHandler m_out;
			GraphicsWindowOutputHandler m_wout;
			const ostd::Stylesheet* m_guiTheme { nullptr };

		private:
			ostd::Color m_clearColor { 10, 10, 10, 255 };

			int32_t m_windowWidth { 0 };
			int32_t m_windowHeight { 0 };
			ostd::String m_title { "" };
			int32_t m_blockingEventsDelay { 33 };

			bool m_running { false };
			bool m_initialized { false };
			bool m_visible { true };
			bool m_blockingEvents { false };
			bool m_resizeable { true };
			bool m_refreshScreen { true };

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

		public:
			inline static constexpr int32_t MaxBlockingEventsFPS { 240 };
			inline static constexpr int32_t DefaultBlockingEventsFPS { 30 };

		private:
			inline static ostd::Stylesheet DefaultTheme;
			std::unordered_map<ostd::String, std::pair<ostd::String, bool>> m_defaultStylesheetVariables;

	};
	class GraphicsWindow : public WindowCore
	{
		public:
			inline GraphicsWindow(void) {  }
			inline GraphicsWindow(int32_t width, int32_t height, const ostd::String& title) { initialize(width, height, title); }

			inline virtual void onRender(void) {  }
			inline virtual void onUpdate(void) {  }
			inline virtual void onFixedUpdate(double frameTime_s) {  }
			inline virtual void onInitialize(void) {  }
			inline virtual void onDestroy(void) {  }
			inline virtual void onClose(void) { }
			inline virtual void onSDLEvent(SDL_Event& event) { }
			inline virtual void onSignal(ostd::Signal& signal) {  }

			inline int32_t getFPS(void) const { return m_fps; }

		protected:
			void __on_window_init(int32_t width, int32_t height, const ostd::String& title) override;
			void __on_event(SDL_Event& event) override;
			void __on_window_destroy(void) override;
			void __on_window_close(void) override;
			void __main_loop(void) override;
			void __on_signal(ostd::Signal& signal) override;

		private:
			int32_t m_fps { 0 };
			ostd::StepTimer m_fixedUpdateTImer;
			ostd::StepTimer m_fpsUpdateTimer;
			ostd::Timer m_fpsUpdateClock;
			uint64_t m_frameTimeAcc { 0 };
			int32_t m_frameCount { 0 };
	};
	namespace gui
	{
		class Window : public WindowCore
		{
			public:
				inline Window(void) {  }
				inline Window(int32_t width, int32_t height, const ostd::String& title) { initialize(width, height, title); }
				void addWidget(Widget& widget);
				void setTheme(const ostd::Stylesheet& theme) override;

				inline virtual void onInitialize(void) {  }
				inline virtual void onDestroy(void) {  }
				inline virtual void onClose(void) {  }
				inline virtual void onSDLEvent(SDL_Event& event) {  }
				inline virtual void onRedraw(BasicRenderer2D& gfx) {  }
				inline virtual void onUpdate(void) {  }
				inline virtual void onSignal(ostd::Signal& signal) {  }

			protected:
				void __on_window_init(int32_t width, int32_t height, const ostd::String& title) override;
				void __on_event(SDL_Event& event) override;
				void __on_window_destroy(void) override;
				void __on_window_close(void) override;
				void __main_loop(void) override;
				void __on_signal(ostd::Signal& signal) override;

			protected:
				BasicRenderer2D m_gfx;
				widgets::RootWidget m_rootWidget { *this };
		};
	}
}
