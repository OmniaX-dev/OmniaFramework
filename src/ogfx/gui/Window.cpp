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
	along with OmniaFramewo6  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Window.hpp"
#include "../../ostd/utils/Time.hpp"
#include "utils/Keycodes.hpp"
#include <SDL3/SDL_events.h>

namespace ogfx
{
	const Uint32 REDRAW_EVENT = SDL_RegisterEvents(1);

	//TODO: Temporary
	inline static const String DefaultThemeStyle = \
		"window.backgroundColor = Color(#000000FF)\n" \
		"label.textColor = Color(#FFFFFFFF)\n" \
		"label.backgroundColor = Color(#00000000)\n" \
		"label.borderColor = Color(#FFFFFFFF)\n" \
		"label.fontSize = 20\n" \
		"label.borderRadius = 20\n" \
		"label.borderWidth = 2\n" \
		"label.showBackground = false\n" \
		"label.showBorder = false\n" \
		"label.padding = Rect(5, 5, 5, 5)\n";

	WindowCore::~WindowCore(void)
	{
		__on_window_destroy();

		SDL_DestroyCursor(m_cursor_Default);
		SDL_DestroyCursor(m_cursor_Text);
		SDL_DestroyCursor(m_cursor_Wait);
		SDL_DestroyCursor(m_cursor_Crosshair);
		SDL_DestroyCursor(m_cursor_Progress);
		SDL_DestroyCursor(m_cursor_NWSE_Resize);
		SDL_DestroyCursor(m_cursor_NESW_Resize);
		SDL_DestroyCursor(m_cursor_EW_Resize);
		SDL_DestroyCursor(m_cursor_NS_Resize);
		SDL_DestroyCursor(m_cursor_Move);
		SDL_DestroyCursor(m_cursor_NotAllowed);
		SDL_DestroyCursor(m_cursor_Pointer);
		SDL_DestroyCursor(m_cursor_NW_Resize);
		SDL_DestroyCursor(m_cursor_N_Resize);
		SDL_DestroyCursor(m_cursor_NE_Resize);
		SDL_DestroyCursor(m_cursor_E_Resize);
		SDL_DestroyCursor(m_cursor_SE_Resize);
		SDL_DestroyCursor(m_cursor_S_Resize);
		SDL_DestroyCursor(m_cursor_SW_Resize);
		SDL_DestroyCursor(m_cursor_W_Resize);

		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDLSysten::release();
	}

	void WindowCore::initialize(i32 width, i32 height, const String& title)
	{
		if (m_initialized) return;
		SDLSysten::acquire();

		__load_default_stylesheet_variables();
		DefaultTheme.loadFromString(DefaultThemeStyle, "memory://", true, getDefaultStylesheetVariableList());

		m_windowWidth = width;
		m_windowHeight = height;
		m_title = title;
		setBlockingEventsRefreshFPS(DefaultBlockingEventsFPS);

		m_window = SDL_CreateWindow("", m_windowWidth, m_windowHeight, SDL_WINDOW_RESIZABLE);
		m_renderer = SDL_CreateRenderer(m_window, nullptr);
		SDL_SetWindowMinimumSize(m_window, m_windowWidth, m_windowHeight);
		setPosition(WindowsPositionCenter);
		SDL_SetWindowTitle(m_window, m_title.c_str());
		SDL_StartTextInput(m_window);

		m_systemScale = SDL_GetWindowDisplayScale(m_window);
		setUserScale(1.0f);

		SDL_ShowWindow(m_window);

		m_cursor_Default      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
		m_cursor_Text         = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
		m_cursor_Wait         = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
		m_cursor_Crosshair    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
		m_cursor_Progress     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_PROGRESS);
		m_cursor_NWSE_Resize  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
		m_cursor_NESW_Resize  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
		m_cursor_EW_Resize    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
		m_cursor_NS_Resize    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
		m_cursor_Move         = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
		m_cursor_NotAllowed   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);
		m_cursor_Pointer      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
		m_cursor_NW_Resize    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NW_RESIZE);
		m_cursor_N_Resize     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_N_RESIZE);
		m_cursor_NE_Resize    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NE_RESIZE);
		m_cursor_E_Resize     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_E_RESIZE);
		m_cursor_SE_Resize    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SE_RESIZE);
		m_cursor_S_Resize     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_S_RESIZE);
		m_cursor_SW_Resize    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SW_RESIZE);
		m_cursor_W_Resize     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_W_RESIZE);

		m_wout.attachWindow(*this);
		// m_wout.setFontSize(22);
		m_wout.setConsoleMaxCharacters({ 1000, 1000 });
		m_wout.setConsolePosition({ 5, 5 });
		m_wout.setWrapMode(ogfx::GraphicsWindowOutputHandler::eWrapMode::NewLine);
		m_wout.setDefaultForegroundColor({ 255, 255, 255, 255 });

		m_initialized = true;
		m_running = true;

		setTypeName("ogfx::WindowCore");
		enableSignals(true);
		validate();
		setSize(m_windowWidth, m_windowHeight);

		connectSignal(ostd::BuiltinSignals::KeyPressed);
		connectSignal(ostd::BuiltinSignals::KeyReleased);
		connectSignal(ostd::BuiltinSignals::TextEntered);
		connectSignal(ostd::BuiltinSignals::MousePressed);
		connectSignal(ostd::BuiltinSignals::MouseReleased);
		connectSignal(ostd::BuiltinSignals::MouseMoved);
		connectSignal(ostd::BuiltinSignals::MouseScrolled);
		connectSignal(ostd::BuiltinSignals::OnGuiEvent);
		connectSignal(ostd::BuiltinSignals::WindowClosed);
		connectSignal(ostd::BuiltinSignals::WindowResized);
		connectSignal(ostd::BuiltinSignals::WindowFocused);
		connectSignal(ostd::BuiltinSignals::WindowLostFocus);
		connectSignal(ostd::BuiltinSignals::FileDragAndDropped);
		connectSignal(ostd::BuiltinSignals::TextDragAndDropped);

		setCursor(eCursor::Default);

		__on_window_init(width, height, title);
		setSize(m_windowWidth, m_windowHeight);
	}

	void WindowCore::mainLoop(void)
	{
		if (isInitialized())
			__main_loop();
	}

	void WindowCore::close(void)
	{
		__on_window_close();
		m_running = false;
		ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::WindowClosed, ostd::Signal::Priority::Normal, *this);
	}

	void WindowCore::setSize(i32 width, i32 height)
	{
		if (!isInitialized()) return;
		SDL_SetWindowSize(m_window, width, height);
		ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::WindowResized, ostd::Signal::Priority::RealTime);
	}

	void WindowCore::setTitle(const String& title)
	{
		if (!isInitialized()) return;
		m_title = title;
		SDL_SetWindowTitle(m_window, m_title.c_str());
	}

	void WindowCore::setCursor(eCursor cursor)
	{
		switch (cursor)
		{
			case eCursor::Default:
				SDL_SetCursor(m_cursor_Default);
			break;
			case eCursor::Text:
				SDL_SetCursor(m_cursor_Text);
			break;
			case eCursor::Wait:
				SDL_SetCursor(m_cursor_Wait);
			break;
			case eCursor::Crosshair:
				SDL_SetCursor(m_cursor_Crosshair);
			break;
			case eCursor::Progress:
				SDL_SetCursor(m_cursor_Progress);
			break;
			case eCursor::NWSE_Resize:
				SDL_SetCursor(m_cursor_NWSE_Resize);
			break;
			case eCursor::NESW_Resize:
				SDL_SetCursor(m_cursor_NESW_Resize);
			break;
			case eCursor::EW_Resize:
				SDL_SetCursor(m_cursor_EW_Resize);
			break;
			case eCursor::NS_Resize:
				SDL_SetCursor(m_cursor_NS_Resize);
			break;
			case eCursor::Move:
				SDL_SetCursor(m_cursor_Move);
			break;
			case eCursor::NotAllowed:
				SDL_SetCursor(m_cursor_NotAllowed);
			break;
			case eCursor::Pointer:
				SDL_SetCursor(m_cursor_Pointer);
			break;
			case eCursor::NW_Resize:
				SDL_SetCursor(m_cursor_NW_Resize);
			break;
			case eCursor::N_Resize:
				SDL_SetCursor(m_cursor_N_Resize);
			break;
			case eCursor::NE_Resize:
				SDL_SetCursor(m_cursor_NE_Resize);
			break;
			case eCursor::E_Resize:
				SDL_SetCursor(m_cursor_E_Resize);
			break;
			case eCursor::SE_Resize:
				SDL_SetCursor(m_cursor_SE_Resize);
			break;
			case eCursor::S_Resize:
				SDL_SetCursor(m_cursor_S_Resize);
			break;
			case eCursor::SW_Resize:
				SDL_SetCursor(m_cursor_SW_Resize);
			break;
			case eCursor::W_Resize:
				SDL_SetCursor(m_cursor_W_Resize);
			break;
			case eCursor::NoCursor:
			default:
				SDL_SetCursor(m_cursor_Default);
			break;
		}
	}

	void WindowCore::setUserScale(f32 scale)
	{
		m_userScale = std::clamp(scale, 0.1f, 10.0f); //TODO: Handle min and  max scale better
		f32 s = getScaleFactor();
		auto state = getWindowState();
		if (!state.maximized && !state.fullscreen)
			setSize(getWindowWidth() * s, getWindowHeight() * s);
		SDL_SetRenderScale(m_renderer, s, s);
	}

	void WindowCore::setPosition(const IPoint& pos)
	{
		IPoint p = pos;
		if (p.x < 0)
			p.x = SDL_WINDOWPOS_CENTERED;
		if (p.y < 0)
			p.y = SDL_WINDOWPOS_CENTERED;
		SDL_SetWindowPosition(m_window, p.x, p.y);
	}

	void WindowCore::setWindowState(eWindowState state)
	{
		switch (state)
		{
			case eWindowState::Default:
				SDL_RestoreWindow(m_window);
			break;
			case eWindowState::Maximized:
				SDL_MaximizeWindow(m_window);
			break;
			case eWindowState::Minimized:
				SDL_MinimizeWindow(m_window);
			break;
			default: break;
		}
	}

	WindowCore::State WindowCore::getWindowState(void) const
	{
		SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
		State state;
		state.fullscreen = flags & SDL_WINDOW_FULLSCREEN;
		state.borderless = flags & SDL_WINDOW_BORDERLESS;
		state.resizable = flags & SDL_WINDOW_RESIZABLE;
		state.minimized = flags & SDL_WINDOW_MINIMIZED;
		state.maximized = flags & SDL_WINDOW_MAXIMIZED;
		state.alwaysOnTop = flags & SDL_WINDOW_ALWAYS_ON_TOP;
		return state;
	}

	WindowCore::eCursor WindowCore::getCurosr(void) const
	{
		auto cur = SDL_GetCursor();
		if (cur == m_cursor_Default)     return eCursor::Default;
		if (cur == m_cursor_Text)        return eCursor::Text;
		if (cur == m_cursor_Wait)        return eCursor::Wait;
		if (cur == m_cursor_Crosshair)   return eCursor::Crosshair;
		if (cur == m_cursor_Progress)    return eCursor::Progress;
		if (cur == m_cursor_NWSE_Resize) return eCursor::NWSE_Resize;
		if (cur == m_cursor_NESW_Resize) return eCursor::NESW_Resize;
		if (cur == m_cursor_EW_Resize)   return eCursor::EW_Resize;
		if (cur == m_cursor_NS_Resize)   return eCursor::NS_Resize;
		if (cur == m_cursor_Move)        return eCursor::Move;
		if (cur == m_cursor_NotAllowed)  return eCursor::NotAllowed;
		if (cur == m_cursor_Pointer)     return eCursor::Pointer;

		if (cur == m_cursor_NW_Resize)   return eCursor::NW_Resize;
		if (cur == m_cursor_N_Resize)    return eCursor::N_Resize;
		if (cur == m_cursor_NE_Resize)   return eCursor::NE_Resize;
		if (cur == m_cursor_E_Resize)    return eCursor::E_Resize;
		if (cur == m_cursor_SE_Resize)   return eCursor::SE_Resize;
		if (cur == m_cursor_S_Resize)    return eCursor::S_Resize;
		if (cur == m_cursor_SW_Resize)   return eCursor::SW_Resize;
		if (cur == m_cursor_W_Resize)    return eCursor::W_Resize;

		return eCursor::Default;
	}

	void WindowCore::enableResizable(bool enable)
	{
		SDL_SetWindowResizable(m_window, enable);
		m_resizeable = enable;
	}

	void WindowCore::setIcon(const String& iconFilePath)
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

	void WindowCore::setBlockingEventsRefreshFPS(u32 fps)
	{
		if (fps == 0 || fps > MaxBlockingEventsFPS)
		{
			setBlockingEventsRefreshFPS(DefaultBlockingEventsFPS);
			return;
		}
		m_blockingEventsDelay = cast<i32>(std::floor((1.0 / (f64)fps) * 1000));
	}

	void WindowCore::requestRedraw(void)
	{
		SDL_Event e{};
		SDL_zero(e);
		e.type = REDRAW_EVENT;
		SDL_PushEvent(&e);
	}

	void WindowCore::handleSignal(ostd::Signal& signal)
	{
		__on_signal(signal);
	}

	stdvec<String> WindowCore::showOpenFileDialog(const stdvec<FileDialogFilter>& filterList, bool multiselect, const String& defaultPath) const
	{
		struct DialogResult {
			stdvec<String> paths;
			bool completed { false };
		};
		DialogResult result;

		__store_file_dialog_filters(filterList);

		SDL_ShowOpenFileDialog(
			[](void* userdata, const char* const* filelist, int filter) {
				auto* res = static_cast<DialogResult*>(userdata);
				if (filelist) {
					for (const char* const* f = filelist; *f; f++)
						res->paths.push_back(String(*f));
				}
				res->completed = true;
			},
			&result,
			m_window,
			m_sdlFilters.empty() ? nullptr : m_sdlFilters.data(),
			m_sdlFilters.size(),
			defaultPath.empty() ? nullptr : defaultPath.c_str(),
			multiselect
		);

		while (!result.completed) {
			SDL_PumpEvents();
			SDL_Delay(10);
		}

		return result.paths;
	}

	stdvec<String> WindowCore::showOpenFolderDialog(bool multiselect, const String& defaultPath) const
	{
		struct DialogResult {
			stdvec<String> paths;
			bool completed { false };
		};
		DialogResult result;

		SDL_ShowOpenFolderDialog(
			[](void* userdata, const char* const* filelist, int filter) {
				auto* res = static_cast<DialogResult*>(userdata);
				if (filelist) {
					for (const char* const* f = filelist; *f; f++)
						res->paths.push_back(String(*f));
				}
				res->completed = true;
			},
			&result,
			m_window,
			defaultPath.empty() ? nullptr : defaultPath.c_str(),
			multiselect
		);

		while (!result.completed) {
			SDL_PumpEvents();
			SDL_Delay(10);
		}

		return result.paths;
	}

	String WindowCore::showSaveFileDialog(const stdvec<FileDialogFilter>& filterList, const String& defaultPath) const
	{
		struct DialogResult {
			String path;
			bool completed { false };
		};
		DialogResult result;

		__store_file_dialog_filters(filterList);

		SDL_ShowSaveFileDialog(
			[](void* userdata, const char* const* filelist, int filter) {
				auto* res = static_cast<DialogResult*>(userdata);
				if (filelist && *filelist) {
					res->path = String(*filelist);
				}
				res->completed = true;
			},
			&result,
			m_window,
			m_sdlFilters.empty() ? nullptr : m_sdlFilters.data(),
			m_sdlFilters.size(),
			defaultPath.empty() ? nullptr : defaultPath.c_str()
		);

		while (!result.completed) {
			SDL_PumpEvents();
			SDL_Delay(10);
		}

		return result.path;
	}

	MouseEventData WindowCore::get_mouse_state(SDL_Event& event)
	{
		f32 mx = 0, my = 0, _mx = 0, _my = 0;
		u32 btn = SDL_GetMouseState(&_mx, &_my);
		f32 scale = getScaleFactor();
		switch (event.type)
		{
			case SDL_EVENT_MOUSE_MOTION:
				mx = event.motion.x / scale;
				my = event.motion.y / scale;
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				mx = event.button.x / scale;
				my = event.button.y / scale;
				btn = event.button.button;
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				mx = event.wheel.mouse_x / scale;
				my = event.wheel.mouse_y / scale;
				break;
			default:
				mx = _mx / scale;
				my = _my / scale;
				break;
		}
		MouseEventData::eButton button = MouseEventData::eButton::None;
		switch (event.button.button)
		{
			case SDL_BUTTON_LEFT: button = MouseEventData::eButton::Left; break;
			case SDL_BUTTON_MIDDLE: button = MouseEventData::eButton::Middle; break;
			case SDL_BUTTON_RIGHT: button = MouseEventData::eButton::Right; break;
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
			if (SDL_WaitEventTimeout(&event, m_blockingEventsDelay))
				__handle_event(event);
		}
		else
		{
			while (SDL_PollEvent(&event))
				__handle_event(event);
		}
	}

	void WindowCore::before_render(void)
	{
		SDL_SetRenderDrawColor(m_renderer, getClearColor().r, getClearColor().g, getClearColor().b, getClearColor().a);
		if (m_refreshScreen)
			SDL_RenderClear(m_renderer);
		wout().beginFrame();
	}

	void WindowCore::after_render(void)
	{
		SDL_RenderPresent(m_renderer);
		ostd::SignalHandler::handleDelegateSignals();
	}

	void WindowCore::__handle_event(SDL_Event& event)
	{
		if (event.type == REDRAW_EVENT)
		{
			//Doesn't need to do anything, the event exists just to make SDL_WaitEventTimeout() return early
		}
		else if (event.type == SDL_EVENT_QUIT)
		{
			close();
		}
		else if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED)
		{
			m_systemScale = SDL_GetWindowDisplayScale(m_window);
			updateScalleFactor();
		}
		else if (event.type == SDL_EVENT_DROP_FILE)
		{
			DropEventData ded(*this, DropEventData::eDropType::File);
			ded.textOrFilePath = event.drop.data;
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::FileDragAndDropped, ostd::Signal::Priority::RealTime, ded);
		}
		else if (event.type == SDL_EVENT_DROP_TEXT)
		{
			DropEventData ded(*this, DropEventData::eDropType::Text);
			ded.textOrFilePath = event.drop.data;
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::FileDragAndDropped, ostd::Signal::Priority::RealTime, ded);
		}
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
		{
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::WindowFocused, ostd::Signal::Priority::RealTime, *this);
		}
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
		{
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::WindowLostFocus, ostd::Signal::Priority::RealTime, *this);
		}
		else if (event.type == SDL_EVENT_WINDOW_RESIZED)
		{
			WindowResizedData wrd(*this, m_windowWidth, m_windowHeight, 0, 0);
			SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
			wrd.new_width = m_windowWidth;
			wrd.new_height = m_windowHeight;
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::WindowResized, ostd::Signal::Priority::RealTime, wrd);
		}
		else if (event.type == SDL_EVENT_MOUSE_MOTION)
		{
			MouseEventData mmd = get_mouse_state(event);
			m_mousePosition = { mmd.position_x, mmd.position_y };
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MouseMoved, ostd::Signal::Priority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_MOUSE_WHEEL)
		{
			MouseEventData mmd = get_mouse_state(event);
			f32 deltaY = event.wheel.y;
			f32 deltaX = m_invertHorizontalScroll ? -event.wheel.x : event.wheel.x;
			if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
				deltaY = -deltaY;
			if (deltaY < 0)
			{
				mmd.scroll = MouseEventData::eScrollDirection::Down;
				mmd.scrollAmount = { 0.0f, deltaY };
			}
			else if (deltaY > 0)
			{
				mmd.scroll = MouseEventData::eScrollDirection::Up;
				mmd.scrollAmount = { 0.0f, deltaY };
			}
			else if (deltaX < 0)
			{
				mmd.scroll = MouseEventData::eScrollDirection::Right;
				mmd.scrollAmount = { deltaX, 0.0f };
			}
			else if (deltaX > 0)
			{
				mmd.scroll = MouseEventData::eScrollDirection::Left;
				mmd.scrollAmount = { deltaX, 0.0f };
			}
			else
			{
				mmd.scroll = MouseEventData::eScrollDirection::None;
				mmd.scrollAmount = { 0.0f, 0.0f };
			}

			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MouseScrolled, ostd::Signal::Priority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		{
			MouseEventData mmd = get_mouse_state(event);
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MousePressed, ostd::Signal::Priority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			MouseEventData mmd = get_mouse_state(event);
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MouseReleased, ostd::Signal::Priority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_KEY_DOWN)
		{
			KeyEventData ked(*this, (i32)event.key.key, 0, KeyEventData::eKeyEvent::Pressed);
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::KeyPressed, ostd::Signal::Priority::RealTime, ked);
		}
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			KeyEventData ked(*this, (i32)event.key.key, 0, KeyEventData::eKeyEvent::Released);
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::KeyReleased, ostd::Signal::Priority::RealTime, ked);
		}
		else if (event.type == SDL_EVENT_TEXT_INPUT)
		{
			KeyEventData ked(*this, 0, event.text.text[0], KeyEventData::eKeyEvent::Text);
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::TextEntered, ostd::Signal::Priority::RealTime, ked);
		}
		__on_event(event);
	}

	void WindowCore::__load_default_stylesheet_variables(void)
	{
		// Cursors
		m_defaultStylesheetVariables["cursor_default"] = { String("").add(cast<i32>(eCursor::Default)), true };
		m_defaultStylesheetVariables["cursor_text"] = { String("").add(cast<i32>(eCursor::Text)), true };
		m_defaultStylesheetVariables["cursor_wait"] = { String("").add(cast<i32>(eCursor::Wait)), true };
		m_defaultStylesheetVariables["cursor_crosshair"] = { String("").add(cast<i32>(eCursor::Crosshair)), true };
		m_defaultStylesheetVariables["cursor_progress"] = { String("").add(cast<i32>(eCursor::Progress)), true };
		m_defaultStylesheetVariables["cursor_nwse_resize"] = { String("").add(cast<i32>(eCursor::NWSE_Resize)), true };
		m_defaultStylesheetVariables["cursor_nesw_resize"] = { String("").add(cast<i32>(eCursor::NESW_Resize)), true };
		m_defaultStylesheetVariables["cursor_ew_resize"] = { String("").add(cast<i32>(eCursor::EW_Resize)), true };
		m_defaultStylesheetVariables["cursor_ns_resize"] = { String("").add(cast<i32>(eCursor::NS_Resize)), true };
		m_defaultStylesheetVariables["cursor_move"] = { String("").add(cast<i32>(eCursor::Move)), true };
		m_defaultStylesheetVariables["cursor_no_allowed"] = { String("").add(cast<i32>(eCursor::NotAllowed)), true };
		m_defaultStylesheetVariables["cursor_pointer"] = { String("").add(cast<i32>(eCursor::Pointer)), true };
		m_defaultStylesheetVariables["cursor_nw_resize"] = { String("").add(cast<i32>(eCursor::NW_Resize)), true };
		m_defaultStylesheetVariables["cursor_n_resize"] = { String("").add(cast<i32>(eCursor::N_Resize)), true };
		m_defaultStylesheetVariables["cursor_ne_resize"] = { String("").add(cast<i32>(eCursor::NE_Resize)), true };
		m_defaultStylesheetVariables["cursor_e_resize"] = { String("").add(cast<i32>(eCursor::E_Resize)), true };
		m_defaultStylesheetVariables["cursor_se_resize"] = { String("").add(cast<i32>(eCursor::SE_Resize)), true };
		m_defaultStylesheetVariables["cursor_s_resize"] = { String("").add(cast<i32>(eCursor::S_Resize)), true };
		m_defaultStylesheetVariables["cursor_sw_resize"] = { String("").add(cast<i32>(eCursor::SW_Resize)), true };
		m_defaultStylesheetVariables["cursor_w_resize"] = { String("").add(cast<i32>(eCursor::W_Resize)), true };
		m_defaultStylesheetVariables["cursor_inherit"] = { String("").add(cast<i32>(eCursor::NoCursor)), true };

		// Colors
		m_defaultStylesheetVariables["color_transparent"]   = { "Color(" + Colors::Transparent.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_black"]         = { "Color(" + Colors::Black.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_white"]         = { "Color(" + Colors::White.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_gray"]          = { "Color(" + Colors::Gray.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_lightgray"]     = { "Color(" + Colors::LightGray.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkgray"]      = { "Color(" + Colors::DarkGray.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_red"]           = { "Color(" + Colors::Red.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_green"]         = { "Color(" + Colors::Green.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_blue"]          = { "Color(" + Colors::Blue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_yellow"]        = { "Color(" + Colors::Yellow.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_cyan"]          = { "Color(" + Colors::Cyan.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_magenta"]       = { "Color(" + Colors::Magenta.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_orange"]        = { "Color(" + Colors::Orange.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_purple"]        = { "Color(" + Colors::Purple.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_brown"]         = { "Color(" + Colors::Brown.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_pink"]          = { "Color(" + Colors::Pink.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_lime"]          = { "Color(" + Colors::Lime.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_olive"]         = { "Color(" + Colors::Olive.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_teal"]          = { "Color(" + Colors::Teal.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_navy"]          = { "Color(" + Colors::Navy.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_maroon"]        = { "Color(" + Colors::Maroon.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_indigo"]        = { "Color(" + Colors::Indigo.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_gold"]          = { "Color(" + Colors::Gold.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_silver"]        = { "Color(" + Colors::Silver.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_beige"]         = { "Color(" + Colors::Beige.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_coral"]         = { "Color(" + Colors::Coral.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_salmon"]        = { "Color(" + Colors::Salmon.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_chocolate"]     = { "Color(" + Colors::Chocolate.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_khaki"]         = { "Color(" + Colors::Khaki.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_lavender"]      = { "Color(" + Colors::Lavender.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_mint"]          = { "Color(" + Colors::Mint.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_skyblue"]       = { "Color(" + Colors::SkyBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_royalblue"]     = { "Color(" + Colors::RoyalBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_deepskyblue"]   = { "Color(" + Colors::DeepSkyBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_turquoise"]     = { "Color(" + Colors::Turquoise.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_aquamarine"]    = { "Color(" + Colors::Aquamarine.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_forestgreen"]   = { "Color(" + Colors::ForestGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_seagreen"]      = { "Color(" + Colors::SeaGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_springgreen"]   = { "Color(" + Colors::SpringGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_firebrick"]     = { "Color(" + Colors::Firebrick.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_crimson"]       = { "Color(" + Colors::Crimson.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_tomato"]        = { "Color(" + Colors::Tomato.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkorange"]    = { "Color(" + Colors::DarkOrange.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkred"]       = { "Color(" + Colors::DarkRed.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkblue"]      = { "Color(" + Colors::DarkBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkgreen"]     = { "Color(" + Colors::DarkGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkcyan"]      = { "Color(" + Colors::DarkCyan.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkmagenta"]   = { "Color(" + Colors::DarkMagenta.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkyellow"]    = { "Color(" + Colors::DarkYellow.hexString(true, "#") + ")", true };

		//Text Align
		m_defaultStylesheetVariables["text_center"]   = { String("").add(cast<i32>(eTextAlign::Center)), true };
		m_defaultStylesheetVariables["text_left"]   = { String("").add(cast<i32>(eTextAlign::Left)), true };
	}

	void WindowCore::__store_file_dialog_filters(const stdvec<FileDialogFilter>& filters) const
	{
		m_patternStorage.clear();
		m_sdlFilters.clear();
		m_patternStorage.reserve(filters.size());
		m_sdlFilters.reserve(filters.size());

		for (const auto& f : filters)
		{
			// Join extensions with semicolons: {"png", "jpg"} -> "png;jpg"
			String pattern;
			for (size_t i = 0; i < f.extensionList.size(); i++)
			{
				if (i > 0) pattern.add(";");
				pattern.add(f.extensionList[i]);
			}
			m_patternStorage.push_back(pattern);

			// SDL filter points at strings that must stay alive
			m_sdlFilters.push_back({
				f.description.c_str(),
				m_patternStorage.back().c_str()
			});
		}
	}





	void GraphicsWindow::__on_window_init(i32 width, i32 height, const String& title)
	{
		SDL_SetWindowResizable(m_window, false);

		m_fixedUpdateTImer.create(60.0, [this](f64 frameTime_s){
			this->onFixedUpdate(frameTime_s);
		});

		m_fpsUpdateTimer.create(1.0, [this](f64 frameTime_s){
			if (this->m_frameCount == 0) return;
			if (this->m_frameTimeAcc == 0) return;
			this->m_fps = (i32)(1.0f / (cast<f64>(this->m_frameTimeAcc) / cast<f64>(this->m_frameCount)));
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
		before_render();
		m_fixedUpdateTImer.update();
		onUpdate();
		onRender();
		after_render();
		m_frameTimeAcc += m_fpsUpdateClock.restart(ostd::eTimeUnits::Seconds);
		m_frameCount++;
		m_fpsUpdateTimer.update();
	}

	void GraphicsWindow::__on_signal(ostd::Signal& signal)
	{
		onSignal(signal);
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
		void Window::addWidget(Widget& widget, const Vec2& position)
		{
			m_rootWidget.addWidget(widget, position);
			setTheme(*m_guiTheme);
		}

		void Window::setTheme(const ostd::Stylesheet& theme)
		{
			m_guiTheme = &theme;
			m_rootWidget.__applyTheme(theme, true);
			m_rootWidget.reloadTheme(true);
			m_cmenu.applyTheme(theme);
			m_menubar.applyTheme(theme);
		}

		void Window::__on_window_init(i32 width, i32 height, const String& title)
		{
			enableBlockingEvents();
			setTypeName("ogfx::gui::Window");
			m_gfx.init(*this);
			loadDefaultTHeme();

			m_fixedUpdateTimer.create(60.0, [this](f64 dt) {
				__on_fixed_update();
			});
			m_lastFrameTime = ostd::StepTimer::Clock::now();
			onInitialize();
			m_rootWidget.setSize(cast<f32>(width), cast<f32>(height));
		}

		void Window::__on_window_close(void)
		{
			onClose();
		}

		void Window::__main_loop(void)
		{
			while (isRunning())
			{
				auto now = ostd::StepTimer::Clock::now();
				f64 delta = std::chrono::duration<f64>(now - m_lastFrameTime).count();
				m_lastFrameTime = now;

				if (delta > 0.25)
					delta = 0.25;

				m_fixedUpdateTimer.update();
				__on_update(delta);

				handle_events();
				before_render();
				m_rootWidget.__update();
				m_rootWidget.__draw(m_gfx);
				onRedraw(m_gfx);
				if (m_menubar.isVisible())
					m_menubar.draw(m_gfx);
				if (m_cmenu.isVisible())
				{
					stopTooltipTimer();
					m_cmenu.draw(m_gfx);
				}
				else if (isTooltipShown())
				{
					auto textSize = m_gfx.getStringDimensions(getTooltipText(), m_rootWidget.getTooltipFontSize());
					Rectangle textBounds = { getMousePosition(), textSize };
					textBounds += Rectangle { 0, 0, 30, 10 };
					m_gfx.outlinedRect(textBounds, m_rootWidget.getTooltipBackgroundColor(), m_rootWidget.getTooltipBorderColor(), m_rootWidget.getTooltipBorderWidth());
					m_gfx.drawCenteredString(getTooltipText(), textBounds, m_rootWidget.getTooltipTextColor(), m_rootWidget.getTooltipFontSize());
				}
				m_gfx.endFrame();
				after_render();
			}
		}

		void Window::__on_signal(ostd::Signal& signal)
		{
			Event evt(*this);
			evt.__original_signal_id = ostd::BuiltinSignals::NoSignal;
			if (signal.ID == ostd::BuiltinSignals::WindowClosed)
			{
				evt.__original_signal_id = ostd::BuiltinSignals::WindowClosed;
				m_rootWidget.__onWindowClosed(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::WindowFocused)
			{
				evt.__original_signal_id = ostd::BuiltinSignals::WindowFocused;
				m_rootWidget.__onWindowFocused(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::WindowLostFocus)
			{
				if (m_cmenu.isVisible())
					m_cmenu.hide();
				evt.__original_signal_id = ostd::BuiltinSignals::WindowLostFocus;
				m_rootWidget.__onWindowFocusLost(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::FileDragAndDropped)
			{
				// auto& ud = (ogfx::DropEventData&)signal.userData;
				// evt.drop.dropType = ud.dropType;
				// evt.drop.userObject = ud.userObject;
				// evt.drop.textOrFilePath = ud.textOrFilePath;
				// evt.__original_signal_id = ostd::BuiltinSignals::FileDragAndDropped;
				// m_rootWidget.__onMouseReleased(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::TextDragAndDropped)
			{
				// auto& ud = (ogfx::DropEventData&)signal.userData;
				// evt.drop.dropType = ud.dropType;
				// evt.drop.userObject = ud.userObject;
				// evt.drop.textOrFilePath = ud.textOrFilePath;
				// evt.__original_signal_id = ostd::BuiltinSignals::TextDragAndDropped;
				// m_rootWidget.__onMouseReleased(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::WindowResized)
			{
				if (m_cmenu.isVisible())
					m_cmenu.hide();
				evt.windowResized = &(ogfx::WindowResizedData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::WindowResized;
				if (m_menubar.isVisible())
					m_menubar.onWindowResized(evt);
				m_rootWidget.__onWindowResized(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseMoved)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MouseMoved;
				if (m_menubar.isVisible())
					m_menubar.onMouseMoved(evt);
				if (m_cmenu.isVisible())
					m_cmenu.onMouseMoved(evt);
				m_rootWidget.__onMouseMoved(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseScrolled)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MouseScrolled;
				if (m_cmenu.isVisible())
					m_cmenu.onMouseScrolled(evt);
				m_rootWidget.__onMouseScrolled(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MousePressed)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MousePressed;
				if (m_cmenu.isVisible())
					m_cmenu.onMousePressed(evt);
				if (m_menubar.isVisible())
					m_menubar.onMousePressed(evt);
				m_rootWidget.__onMousePressed(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseReleased)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MouseReleased;
				if (m_cmenu.isVisible())
					m_cmenu.onMouseReleased(evt);
				m_rootWidget.__onMouseReleased(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::KeyPressed)
			{
				evt.keyboard = &(ogfx::KeyEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::KeyPressed;
				m_rootWidget.__onKeyPressed(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::KeyReleased)
			{
				evt.keyboard = &(ogfx::KeyEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::KeyReleased;
				m_rootWidget.__onKeyReleased(evt);
				if (evt.keyboard->keyCode == KeyCode::Escape)
					close();
			}
			else if (signal.ID == ostd::BuiltinSignals::TextEntered)
			{
				evt.keyboard = &(ogfx::KeyEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::TextEntered;
				m_rootWidget.__onTextEntered(evt);
			}
			onSignal(signal);
		}

		void Window::__on_update(f64 delta)
		{
			m_cmenu.update();
			if (m_menubar.isVisible())
				m_menubar.update();
			onUpdate(delta);
		}

		void Window::__on_fixed_update(void)
		{
			onFixedUpdate();
		}

		void Window::__on_event(SDL_Event& event)
		{
			onSDLEvent(event);
		}

		void Window::__on_window_destroy(void)
		{
			onDestroy();
		}
	}
}
