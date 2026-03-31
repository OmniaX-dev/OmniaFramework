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

#include "WindowBase.hpp"
#include "../../ostd/utils/Time.hpp"
#include <SDL2/SDL_render.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>

namespace ogfx
{
	WindowBase::~WindowBase(void)
	{
		onDestroy();
		SDL_DestroyCursor(m_cursor_IBeam);
		SDL_DestroyCursor(m_cursor_Arrow);
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		TTF_Quit();
	}

	void WindowBase::initialize(int32_t width, int32_t height, const ostd::String& windowTitle)
	{
		if (m_initialized) return;
		m_windowWidth = width;
		m_windowHeight = height;
		m_title = windowTitle;
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			printf( "SDL could not initialize! Error: %s\n", SDL_GetError() );
			exit(1);
		}
		m_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_RESIZABLE);
		SDL_SetWindowResizable(m_window, false);
		m_renderer = SDL_CreateRenderer(m_window, nullptr);
		SDL_SetWindowMinimumSize(m_window, m_windowWidth, m_windowHeight);
		SDL_SetWindowTitle(m_window, m_title.c_str());
		SDL_StartTextInput(m_window);

		m_cursor_Arrow = SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_DEFAULT);
		m_cursor_IBeam = SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_TEXT);

		m_initialized = true;
		m_running = true;

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

		setTypeName("ogfx::WindowBase");
		enableSignals(true);
		validate();
		setSize(m_windowWidth, m_windowHeight);

		onInitialize();
	}

	void WindowBase::close(void)
	{
		m_running = false;
		onClose();
		ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowClosed, ostd::tSignalPriority::RealTime, *this);
	}

	void WindowBase::update(void)
	{
		if (!m_initialized) return;
		__handle_events();
		SDL_SetRenderDrawColor(m_renderer, m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
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

	void WindowBase::setSize(int32_t width, int32_t height)
	{
		if (!isInitialized()) return;
		SDL_SetWindowSize(m_window, width, height);
		ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime);
	}

	void WindowBase::setTitle(const ostd::String& title)
	{
		if (!isInitialized()) return;
		m_title = title;
		SDL_SetWindowTitle(m_window, m_title.c_str());
	}

	void WindowBase::setCursor(eCursor cursor)
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

	void WindowBase::enableResizable(bool enable)
	{
		SDL_SetWindowResizable(m_window, enable);
	}

	void WindowBase::setIcon(const ostd::String& iconFilePath)
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

	void WindowBase::__handle_events(void)
	{
		if (!m_initialized) return;
		auto l_getMouseState = [this](void) -> MouseEventData {
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
		};
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				m_running = false;
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowClosed, ostd::tSignalPriority::Normal, *this);
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
				MouseEventData mmd = l_getMouseState();
				if (isMouseDragEventEnabled() && mmd.button != MouseEventData::eButton::None)
					ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseDragged, ostd::tSignalPriority::RealTime, mmd);
				else
					ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseMoved, ostd::tSignalPriority::RealTime, mmd);
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
			{
				MouseEventData mmd = l_getMouseState();
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MousePressed, ostd::tSignalPriority::RealTime, mmd);
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			{
				MouseEventData mmd = l_getMouseState();
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
		}
	}
}
