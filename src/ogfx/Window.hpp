#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <ostd/BaseObject.hpp>
#include <SFML/Window.hpp>
#include <ogfx/DataStructures.hpp>

namespace ogfx
{
	class Event : public ostd::BaseObject
	{
		public:
			inline Event(sf::Event& evt) { m_handled = false; setTypeName("ogfx::Event"); validate(); m_event = evt; }
			inline sf::Event& sf(void) { return m_event; }
			inline bool isHandled(void) { return m_handled; }
			inline void markAsHandled(void) { m_handled = true; }

		private:
			sf::Event m_event;
			bool m_handled; //TODO: Add more stuff (type, key and such) to this class in order to avoid using the SFML event directly if possible
	};

	class Window : public ostd::BaseObject
	{
		public:
			inline Window(void) { invalidate(); }
			inline Window(ostd::UI16Point size) { create(size, "OmniaX Window", tContextSettings()); }
			inline Window(ostd::UI16Point size, ostd::String title) { create(size, title, tContextSettings()); }
			inline Window(ostd::UI16Point size, ostd::String title, tContextSettings contextSettings) { create(size, title, contextSettings); }
			Window& create(ostd::UI16Point size, ostd::String title, tContextSettings contextSettings);

			inline Window& enableVSync(bool v = true) { m_sfml_window.setVerticalSyncEnabled(v); return *this; }
			inline Window& hideMouseCursor(bool v = true) { m_sfml_window.setMouseCursorVisible(!v); return *this; }
			inline Window& disableMouseMove(bool v = true) { m_disable_mouse_moved_event = v; return *this; }
			inline Window& close(void) { m_running = false; return *this; }
			inline Window& renderFrame(void) { m_sfml_window.display(); return *this; }

			inline bool isRunning(void) { return m_running; }
			inline uint32_t getTitleBarSize(void) { return m_win_deco_sizes.y; }
			inline uint32_t getBorderSize(void) { return m_win_deco_sizes.x; }
			inline sf::Window& sf(void) { return m_sfml_window; }
			inline sf::Vector2i getSize(void) { return (sf::Vector2i)m_sfml_window.getSize(); }
			inline sf::Vector2i getPosition(void) { return (sf::Vector2i)m_sfml_window.getPosition(); }
			inline bool isMouseMoveEnabled(void) { return !m_disable_mouse_moved_event; }

			void handleEvents(void);
			Window& centerMouse(void);

			void printOpenGLInfo(void);

			inline Window& initialize(void) { __calc_win_deco_sizes(); return *this; }
		
		private:
			void __calc_win_deco_sizes(void);

		private:
			sf::Window m_sfml_window;
			bool m_running { true };
			sf::Vector2i m_win_deco_sizes { 0, 0 };
			bool m_disable_mouse_moved_event { false };
	};
}

#endif
