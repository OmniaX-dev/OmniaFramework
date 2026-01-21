#pragma once

#include <ogfx/SDLInclude.hpp>
#include <ostd/Signals.hpp>
#include <ostd/IOHandlers.hpp>
#include <ostd/Time.hpp>

namespace ogfx
{
	class WindowBase : public ostd::BaseObject
	{
		public: enum class eCursor { Arrow = 0, IBeam };
		public:
			inline WindowBase(void) {  }
			~WindowBase(void);
			inline WindowBase(int32_t width, int32_t height, const ostd::String& windowTitle) { initialize(width, height, windowTitle); }
			void initialize(int32_t width, int32_t height, const ostd::String& windowTitle);
			void close(void);
			void update(void);
			void setSize(int32_t width, int32_t height);
			void setTitle(const ostd::String& title);
			void setCursor(eCursor cursor);
			void enableResizable(bool enable = true);

			inline virtual void onRender(void) {  }
			inline virtual void onUpdate(void) {  }
			inline virtual void onFixedUpdate(double frameTime_s) {  }
			inline virtual void onInitialize(void) {  }
			inline virtual void onDestroy(void) {  }
			inline virtual void onClose(void) { }

			inline bool isInitialized(void) const { return m_initialized; }
			inline bool isRunning(void) const { return m_running; }
			inline void hide(void) { SDL_HideWindow(m_window); }
			inline void show(void) { SDL_ShowWindow(m_window); }
			inline ostd::String getTitle(void) const { return m_title; }
			inline int32_t getFPS(void) const { return m_fps; }
			inline int32_t getWindowWidth(void) const { return m_windowWidth; }
			inline int32_t getWindowHeight(void) const { return m_windowHeight; }
			inline bool isMouseDragEventEnabled(void) const { return m_deagEventEnabled; }
			inline void enableMouseDragEvent(bool enable = true) { m_deagEventEnabled = enable; }
			inline ostd::Color getClearColor(void) const { return m_clearColor; }
			inline void setClearColor(const ostd::Color& color) { m_clearColor = color; }
			inline SDL_Renderer* getSDLRenderer(void) const { return m_renderer; }
		private:
			void __handle_events(void);

		protected:
			ostd::ConsoleOutputHandler out;

			SDL_Window* m_window { nullptr };
			SDL_Renderer* m_renderer { nullptr };
			bool m_refreshScreen { true };

		private:
			ostd::Color m_clearColor { 10, 10, 10, 255 };

			int32_t m_windowWidth { 0 };
			int32_t m_windowHeight { 0 };
			ostd::String m_title { "" };
			int32_t m_fps { 0 };

			ostd::StepTimer m_fixedUpdateTImer;
			ostd::StepTimer m_fpsUpdateTimer;
			ostd::Timer m_fpsUpdateClock;
			uint64_t m_frameTimeAcc { 0 };
			int32_t m_frameCount { 0 };

			bool m_deagEventEnabled { false };
			bool m_running { false };
			bool m_initialized { false };

			SDL_Cursor* m_cursor_IBeam { nullptr };
			SDL_Cursor* m_cursor_Arrow { nullptr };
	};
	class WindowResizedData : public ostd::BaseObject
	{
		public:
			inline WindowResizedData(WindowBase& parent, int32_t _oldx, int32_t _oldy, int32_t _newx, int32_t _newy) : parentWindow(parent), old_width(_oldx), old_height(_oldy), new_width(_newx), new_height(_newy)
			{
				setTypeName("ogfx::WindowResizedData");
				validate();
			}

		public:
			int32_t new_width;
			int32_t new_height;
			int32_t old_width;
			int32_t old_height;
			WindowBase& parentWindow;
	};
	class MouseEventData : public ostd::BaseObject
	{
		public: enum class eButton { None = 0, Left, Middle, Right };
		public:
			inline MouseEventData(WindowBase& parent, int32_t mousex, int32_t mousey, eButton btn) : parentWindow(parent), position_x(mousex), position_y(mousey), button(btn)
			{
				setTypeName("ogfx::MouseEventData");
				validate();
			}

		public:
			int32_t position_x;
			int32_t position_y;
			eButton button;
			WindowBase& parentWindow;
	};
	class KeyEventData : public ostd::BaseObject
	{
		public: enum class eKeyEvent { Pressed = 0, Released, Text };
		public:
			inline KeyEventData(WindowBase& parent, int32_t key, char _text, eKeyEvent evt) : parentWindow(parent), keyCode(key), text(_text), eventType(evt)
			{
				setTypeName("ogfx::KeyEventData");
				validate();
			}

		public:
			int32_t keyCode;
			char text;
			eKeyEvent eventType;
			WindowBase& parentWindow;
	};
}
