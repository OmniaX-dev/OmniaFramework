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

namespace ogfx
{
	namespace gui
	{
		class Window : public ostd::BaseObject
		{
			public: enum class eCursor { Arrow = 0, IBeam };
			public: struct ErrorCodes {
				inline static constexpr uint16_t ExitSuccess { 0 };
				inline static constexpr uint16_t SDLInitFailed { 1 };
				inline static constexpr uint16_t SDLImageInitFailed { 2 };
				inline static constexpr uint16_t Uninitialized { 3 };
			};
			public:
				inline Window(void) {  }
				~Window(void);
				inline Window(int32_t width, int32_t height, const ostd::String& windowTitle) { initialize(width, height, windowTitle); }
				void initialize(int32_t width, int32_t height, const ostd::String& windowTitle);
				void close(void);
				int32_t startMainLoop(void);
				void setSize(int32_t width, int32_t height);
				void setTitle(const ostd::String& title);
				void setCursor(eCursor cursor);
				void enableResizable(bool enable = true);
				void setIcon(const ostd::String& iconFilePath);

				inline virtual void onInitialize(void) {  }
				inline virtual void onDestroy(void) {  }
				inline virtual void onClose(void) { }

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
				inline SDL_Renderer* getSDLRenderer(void) const { return m_renderer; }
			private:
				void __handle_events(void);

			protected:
				SDL_Window* m_window { nullptr };
				SDL_Renderer* m_renderer { nullptr };

			private:
				ostd::Color m_clearColor { 10, 10, 10, 255 };

				int32_t m_windowWidth { 0 };
				int32_t m_windowHeight { 0 };
				ostd::String m_title { "" };

				bool m_running { false };
				bool m_initialized { false };
				bool m_visible { true };
				bool m_resizeable { true };

				SDL_Cursor* m_cursor_IBeam { nullptr };
				SDL_Cursor* m_cursor_Arrow { nullptr };
		};
		class WindowResizedData : public ostd::BaseObject
		{
			public:
				inline WindowResizedData(Window& parent, int32_t _oldx, int32_t _oldy, int32_t _newx, int32_t _newy) : parentWindow(parent), old_width(_oldx), old_height(_oldy), new_width(_newx), new_height(_newy)
				{
					setTypeName("ogfx::gui::WindowResizedData");
					validate();
				}

			public:
				int32_t new_width;
				int32_t new_height;
				int32_t old_width;
				int32_t old_height;
				Window& parentWindow;
		};
		class MouseEventData : public ostd::BaseObject
		{
			public: enum class eButton { None = 0, Left, Middle, Right };
			public:
				inline MouseEventData(Window& parent, int32_t mousex, int32_t mousey, eButton btn) : parentWindow(parent), position_x(mousex), position_y(mousey), button(btn)
				{
					setTypeName("ogfx::gui::MouseEventData");
					validate();
				}

			public:
				int32_t position_x;
				int32_t position_y;
				eButton button;
				Window& parentWindow;
		};
		class KeyEventData : public ostd::BaseObject
		{
			public: enum class eKeyEvent { Pressed = 0, Released, Text };
			public:
				inline KeyEventData(Window& parent, int32_t key, char _text, eKeyEvent evt) : parentWindow(parent), keyCode(key), text(_text), eventType(evt)
				{
					setTypeName("ogfx::gui::KeyEventData");
					validate();
				}

			public:
				int32_t keyCode;
				char text;
				eKeyEvent eventType;
				Window& parentWindow;
		};
	}
}
