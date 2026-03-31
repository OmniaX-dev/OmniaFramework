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

#include "Window.hpp"
#include "../../ostd/utils/Time.hpp"
#include "io/IOHandlers.hpp"
#include "string/String.hpp"
#include <SDL2/SDL_render.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>

namespace ogfx
{
	WindowCore::~WindowCore(void)
	{
		__on_window_destroy();
		SDL_DestroyCursor(m_cursor_IBeam);
		SDL_DestroyCursor(m_cursor_Arrow);
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		TTF_Quit();
	}

	void WindowCore::initialize(int32_t width, int32_t height, const ostd::String& title)
	{
		if (m_initialized) return;
		m_windowWidth = width;
		m_windowHeight = height;
		m_title = title;
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			printf( "SDL could not initialize! Error: %s\n", SDL_GetError() );
			exit(1);
		}
		m_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_RESIZABLE);
		m_renderer = SDL_CreateRenderer(m_window, nullptr);
		SDL_SetWindowMinimumSize(m_window, m_windowWidth, m_windowHeight);
		SDL_SetWindowTitle(m_window, m_title.c_str());
		SDL_StartTextInput(m_window);

		m_cursor_Arrow = SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_DEFAULT);
		m_cursor_IBeam = SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_TEXT);

		m_initialized = true;
		m_running = true;

		setTypeName("ogfx::WindowCore");
		enableSignals(true);
		validate();
		setSize(m_windowWidth, m_windowHeight);

		connectSignal(ostd::tBuiltinSignals::KeyPressed);
		connectSignal(ostd::tBuiltinSignals::KeyReleased);
		connectSignal(ostd::tBuiltinSignals::TextEntered);
		connectSignal(ostd::tBuiltinSignals::MousePressed);
		connectSignal(ostd::tBuiltinSignals::MouseReleased);
		connectSignal(ostd::tBuiltinSignals::MouseMoved);
		connectSignal(ostd::tBuiltinSignals::OnGuiEvent);
		connectSignal(ostd::tBuiltinSignals::WindowClosed);
		connectSignal(ostd::tBuiltinSignals::WindowResized);

		__on_window_init(width, height, title);
	}

	void WindowCore::close(void)
	{
		__on_window_close();
		m_running = false;
		ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowClosed, ostd::tSignalPriority::Normal, *this);
	}

	void WindowCore::setSize(int32_t width, int32_t height)
	{
		if (!isInitialized()) return;
		SDL_SetWindowSize(m_window, width, height);
		ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime);
	}

	void WindowCore::setTitle(const ostd::String& title)
	{
		if (!isInitialized()) return;
		m_title = title;
		SDL_SetWindowTitle(m_window, m_title.c_str());
	}

	void WindowCore::setCursor(eCursor cursor)
	{
		switch (cursor)
		{
			case eCursor::Arrow:
				SDL_SetCursor(m_cursor_Arrow);
			break;
			case eCursor::IBeam:
				SDL_SetCursor(m_cursor_IBeam);
			break;
			default:
				SDL_SetCursor(m_cursor_Arrow);
			break;
		}
	}

	void WindowCore::enableResizable(bool enable)
	{
		SDL_SetWindowResizable(m_window, enable);
		m_resizeable = enable;
	}

	void WindowCore::setIcon(const ostd::String& iconFilePath)
	{
		SDL_Surface* appIcon = IMG_Load(iconFilePath);
		if (appIcon == nullptr)
		{
			//TODO: Error
			return;
		}
		SDL_SetWindowIcon(m_window, appIcon);
		SDL_DestroySurface(appIcon);
	}

	MouseEventData WindowCore::get_mouse_state(void)
	{
		float mx = 0, my = 0;
		uint32_t btn = SDL_GetMouseState(&mx, &my);
		MouseEventData::eButton button = MouseEventData::eButton::None;
		switch (btn)
		{
			case SDL_BUTTON_MASK(1): button = MouseEventData::eButton::Left; break;
			case SDL_BUTTON_MASK(2): button = MouseEventData::eButton::Middle; break;
			case SDL_BUTTON_MASK(3): button = MouseEventData::eButton::Right; break;
			default: button = MouseEventData::eButton::None; break;
		}
		MouseEventData mmd(*this, mx, my, button);
		return mmd;
	}

	void WindowCore::handle_events(void)
	{
		if (!isInitialized()) return;
		SDL_Event event;

		if (isBlockingEventsEnabled())
		{
			if (SDL_WaitEventTimeout(&event, 16))
				__handle_event(event);
		}
		else
		{
			while (SDL_PollEvent(&event))
				__handle_event(event);
		}
	}

	void WindowCore::__handle_event(SDL_Event& event)
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			close();
		}
		else if (event.type == SDL_EVENT_WINDOW_RESIZED)
		{
			WindowResizedData wrd(*this, m_windowWidth, m_windowHeight, 0, 0);
			SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
			wrd.new_width = m_windowWidth;
			wrd.new_height = m_windowHeight;
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
		}
		else if (event.type == SDL_EVENT_MOUSE_MOTION)
		{
			MouseEventData mmd = get_mouse_state();
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseMoved, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		{
			MouseEventData mmd = get_mouse_state();
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MousePressed, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			MouseEventData mmd = get_mouse_state();
			switch (event.button.button)
			{
				case SDL_BUTTON_MASK(1): mmd.button = MouseEventData::eButton::Left; break;
				case SDL_BUTTON_MASK(2): mmd.button = MouseEventData::eButton::Middle; break;
				case SDL_BUTTON_MASK(3): mmd.button = MouseEventData::eButton::Right; break;
				default: mmd.button = MouseEventData::eButton::None; break;
			}
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseReleased, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_KEY_DOWN)
		{
			KeyEventData ked(*this, (int32_t)event.key.key, 0, KeyEventData::eKeyEvent::Pressed);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyPressed, ostd::tSignalPriority::RealTime, ked);
		}
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			KeyEventData ked(*this, (int32_t)event.key.key, 0, KeyEventData::eKeyEvent::Released);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyReleased, ostd::tSignalPriority::RealTime, ked);
		}
		else if (event.type == SDL_EVENT_TEXT_INPUT)
		{
			KeyEventData ked(*this, 0, event.text.text[0], KeyEventData::eKeyEvent::Text);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::TextEntered, ostd::tSignalPriority::RealTime, ked);
		}
		__on_event(event);
	}




	void GraphicsWindow::__on_window_init(int32_t width, int32_t height, const ostd::String& title)
	{
		SDL_SetWindowResizable(m_window, false);

		m_fixedUpdateTImer.create(60.0, [this](double frameTime_s){
			this->onFixedUpdate(frameTime_s);
		});

		m_fpsUpdateTimer.create(1.0, [this](double frameTime_s){
			if (this->m_frameCount == 0) return;
			if (this->m_frameTimeAcc == 0) return;
			this->m_fps = (int32_t)(1.0f / (static_cast<double>(this->m_frameTimeAcc) / static_cast<double>(this->m_frameCount)));
			this->m_frameTimeAcc = 0;
			this->m_frameCount = 0;
		});

		setTypeName("ogfx::GraphicsWindow");
		onInitialize();
	}

	void GraphicsWindow::__on_window_close(void)
	{
		onClose();
	}

	void GraphicsWindow::__main_loop(void)
	{
		handle_events();
		SDL_SetRenderDrawColor(m_renderer, getClearColor().r, getClearColor().g, getClearColor().b, getClearColor().a);
		if (m_refreshScreen)
			SDL_RenderClear(m_renderer);
		m_fixedUpdateTImer.update();
		onUpdate();
		onRender();
		SDL_RenderPresent(m_renderer);
		m_frameTimeAcc += m_fpsUpdateClock.restart(ostd::eTimeUnits::Seconds);
		m_frameCount++;
		m_fpsUpdateTimer.update();
	}

	void GraphicsWindow::__on_event(SDL_Event& event)
	{
		onSDLEvent(event);
	}

	void GraphicsWindow::__on_window_destroy(void)
	{
		onDestroy();
	}




	namespace gui
	{
		void Window::__on_window_init(int32_t width, int32_t height, const ostd::String& title)
		{
			enableBlockingEvents();
			setTypeName("ogfx::gui::Window");
			m_gfx.init(*this);
			onInitialize();
		}

		void Window::__on_window_close(void)
		{
			onClose();
		}

		void Window::__main_loop(void)
		{
			while (isRunning())
			{
				handle_events();
				SDL_SetRenderDrawColor(m_renderer, getClearColor().r, getClearColor().g, getClearColor().b, getClearColor().a);
				SDL_RenderClear(m_renderer);
				m_gfx.drawString("Hello World", { 100, 100 }, { 255, 0, 0 });
				SDL_RenderPresent(m_renderer);
			}
		}

		void Window::__on_event(SDL_Event& event)
		{
			if (event.type == SDL_EVENT_KEY_UP)
			{
				if (event.key.key == SDLK_ESCAPE)
					close();
			}
			onSDLEvent(event);
		}

		void Window::__on_window_destroy(void)
		{
			onDestroy();
		}
	}
}
