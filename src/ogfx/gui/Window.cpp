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
#include "utils/Keycodes.hpp"
#include <SDL3/SDL_events.h>

namespace ogfx
{
	const Uint32 REDRAW_EVENT = SDL_RegisterEvents(1);

	inline static const ostd::String DefaultThemeStyle = \
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

	void WindowCore::initialize(int32_t width, int32_t height, const ostd::String& title)
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
		SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_SetWindowTitle(m_window, m_title.c_str());
		SDL_StartTextInput(m_window);
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
		m_wout.setFontSize(22);
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

	void WindowCore::setSize(int32_t width, int32_t height)
	{
		if (!isInitialized()) return;
		SDL_SetWindowSize(m_window, width, height);
		ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::WindowResized, ostd::Signal::Priority::RealTime);
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
	        default:
	            SDL_SetCursor(m_cursor_Default);
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

	void WindowCore::setBlockingEventsRefreshFPS(uint32_t fps)
	{
		if (fps == 0 || fps > MaxBlockingEventsFPS)
		{
			setBlockingEventsRefreshFPS(DefaultBlockingEventsFPS);
			return;
		}
		m_blockingEventsDelay = static_cast<int>(std::floor((1.0 / (double)fps) * 1000));
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
			MouseEventData mmd = get_mouse_state();
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MouseMoved, ostd::Signal::Priority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_MOUSE_WHEEL)
		{
			MouseEventData mmd = get_mouse_state();
			if (event.wheel.y == -1)
				mmd.scroll = MouseEventData::eScrollDirection::Down;
			else if (event.wheel.y == 1)
				mmd.scroll = MouseEventData::eScrollDirection::Up;
			else
				mmd.scroll = MouseEventData::eScrollDirection::None;
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MouseScrolled, ostd::Signal::Priority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		{
			MouseEventData mmd = get_mouse_state();
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MousePressed, ostd::Signal::Priority::RealTime, mmd);
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
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::MouseReleased, ostd::Signal::Priority::RealTime, mmd);
		}
		else if (event.type == SDL_EVENT_KEY_DOWN)
		{
			KeyEventData ked(*this, (int32_t)event.key.key, 0, KeyEventData::eKeyEvent::Pressed);
			ostd::SignalHandler::emitSignal(ostd::BuiltinSignals::KeyPressed, ostd::Signal::Priority::RealTime, ked);
		}
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			KeyEventData ked(*this, (int32_t)event.key.key, 0, KeyEventData::eKeyEvent::Released);
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
		m_defaultStylesheetVariables["cursor_default"] = { ostd::String("").add(static_cast<int32_t>(eCursor::Default)), true };
		m_defaultStylesheetVariables["cursor_text"] = { ostd::String("").add(static_cast<int32_t>(eCursor::Text)), true };
		m_defaultStylesheetVariables["cursor_wait"] = { ostd::String("").add(static_cast<int32_t>(eCursor::Wait)), true };
		m_defaultStylesheetVariables["cursor_crosshair"] = { ostd::String("").add(static_cast<int32_t>(eCursor::Crosshair)), true };
		m_defaultStylesheetVariables["cursor_progress"] = { ostd::String("").add(static_cast<int32_t>(eCursor::Progress)), true };
		m_defaultStylesheetVariables["cursor_nwse_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::NWSE_Resize)), true };
		m_defaultStylesheetVariables["cursor_nesw_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::NESW_Resize)), true };
		m_defaultStylesheetVariables["cursor_ew_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::EW_Resize)), true };
		m_defaultStylesheetVariables["cursor_ns_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::NS_Resize)), true };
		m_defaultStylesheetVariables["cursor_move"] = { ostd::String("").add(static_cast<int32_t>(eCursor::Move)), true };
		m_defaultStylesheetVariables["cursor_no_allowed"] = { ostd::String("").add(static_cast<int32_t>(eCursor::NotAllowed)), true };
		m_defaultStylesheetVariables["cursor_pointer"] = { ostd::String("").add(static_cast<int32_t>(eCursor::Pointer)), true };
		m_defaultStylesheetVariables["cursor_nw_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::NW_Resize)), true };
		m_defaultStylesheetVariables["cursor_n_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::N_Resize)), true };
		m_defaultStylesheetVariables["cursor_ne_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::NE_Resize)), true };
		m_defaultStylesheetVariables["cursor_e_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::E_Resize)), true };
		m_defaultStylesheetVariables["cursor_se_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::SE_Resize)), true };
		m_defaultStylesheetVariables["cursor_s_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::S_Resize)), true };
		m_defaultStylesheetVariables["cursor_sw_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::SW_Resize)), true };
		m_defaultStylesheetVariables["cursor_w_resize"] = { ostd::String("").add(static_cast<int32_t>(eCursor::W_Resize)), true };

		// Colors
		m_defaultStylesheetVariables["color_transparent"]   = { "Color(" + ostd::Colors::Transparent.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_black"]         = { "Color(" + ostd::Colors::Black.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_white"]         = { "Color(" + ostd::Colors::White.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_gray"]          = { "Color(" + ostd::Colors::Gray.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_lightgray"]     = { "Color(" + ostd::Colors::LightGray.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkgray"]      = { "Color(" + ostd::Colors::DarkGray.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_red"]           = { "Color(" + ostd::Colors::Red.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_green"]         = { "Color(" + ostd::Colors::Green.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_blue"]          = { "Color(" + ostd::Colors::Blue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_yellow"]        = { "Color(" + ostd::Colors::Yellow.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_cyan"]          = { "Color(" + ostd::Colors::Cyan.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_magenta"]       = { "Color(" + ostd::Colors::Magenta.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_orange"]        = { "Color(" + ostd::Colors::Orange.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_purple"]        = { "Color(" + ostd::Colors::Purple.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_brown"]         = { "Color(" + ostd::Colors::Brown.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_pink"]          = { "Color(" + ostd::Colors::Pink.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_lime"]          = { "Color(" + ostd::Colors::Lime.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_olive"]         = { "Color(" + ostd::Colors::Olive.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_teal"]          = { "Color(" + ostd::Colors::Teal.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_navy"]          = { "Color(" + ostd::Colors::Navy.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_maroon"]        = { "Color(" + ostd::Colors::Maroon.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_indigo"]        = { "Color(" + ostd::Colors::Indigo.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_gold"]          = { "Color(" + ostd::Colors::Gold.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_silver"]        = { "Color(" + ostd::Colors::Silver.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_beige"]         = { "Color(" + ostd::Colors::Beige.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_coral"]         = { "Color(" + ostd::Colors::Coral.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_salmon"]        = { "Color(" + ostd::Colors::Salmon.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_chocolate"]     = { "Color(" + ostd::Colors::Chocolate.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_khaki"]         = { "Color(" + ostd::Colors::Khaki.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_lavender"]      = { "Color(" + ostd::Colors::Lavender.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_mint"]          = { "Color(" + ostd::Colors::Mint.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_skyblue"]       = { "Color(" + ostd::Colors::SkyBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_royalblue"]     = { "Color(" + ostd::Colors::RoyalBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_deepskyblue"]   = { "Color(" + ostd::Colors::DeepSkyBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_turquoise"]     = { "Color(" + ostd::Colors::Turquoise.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_aquamarine"]    = { "Color(" + ostd::Colors::Aquamarine.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_forestgreen"]   = { "Color(" + ostd::Colors::ForestGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_seagreen"]      = { "Color(" + ostd::Colors::SeaGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_springgreen"]   = { "Color(" + ostd::Colors::SpringGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_firebrick"]     = { "Color(" + ostd::Colors::Firebrick.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_crimson"]       = { "Color(" + ostd::Colors::Crimson.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_tomato"]        = { "Color(" + ostd::Colors::Tomato.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkorange"]    = { "Color(" + ostd::Colors::DarkOrange.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkred"]       = { "Color(" + ostd::Colors::DarkRed.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkblue"]      = { "Color(" + ostd::Colors::DarkBlue.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkgreen"]     = { "Color(" + ostd::Colors::DarkGreen.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkcyan"]      = { "Color(" + ostd::Colors::DarkCyan.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkmagenta"]   = { "Color(" + ostd::Colors::DarkMagenta.hexString(true, "#") + ")", true };
		m_defaultStylesheetVariables["color_darkyellow"]    = { "Color(" + ostd::Colors::DarkYellow.hexString(true, "#") + ")", true };
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
		void Window::addWidget(Widget& widget)
		{
			m_rootWidget.addChild(widget);
			setTheme(*m_guiTheme);
		}

		void Window::setTheme(const ostd::Stylesheet& theme)
		{
			m_guiTheme = &theme;
			m_rootWidget.__applyTheme(theme, true);
		}

		void Window::__on_window_init(int32_t width, int32_t height, const ostd::String& title)
		{
			enableBlockingEvents();
			setTypeName("ogfx::gui::Window");
			m_gfx.init(*this);
			loadDefaultTHeme();
			onInitialize();
			m_rootWidget.setSize(static_cast<float>(width), static_cast<float>(height));
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
				before_render();
				m_rootWidget.__update();
				m_rootWidget.__draw(m_gfx);
				onRedraw(m_gfx);
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
				evt.windowResized = &(ogfx::WindowResizedData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::WindowResized;
				m_rootWidget.__onWindowResized(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseMoved)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MouseMoved;
				m_rootWidget.__onMouseMoved(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseScrolled)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MouseScrolled;
				m_rootWidget.__onMouseScrolled(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MousePressed)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MousePressed;
				m_rootWidget.__onMousePressed(evt);
			}
			else if (signal.ID == ostd::BuiltinSignals::MouseReleased)
			{
				evt.mouse = &(ogfx::MouseEventData&)signal.userData;
				evt.__original_signal_id = ostd::BuiltinSignals::MouseReleased;
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
