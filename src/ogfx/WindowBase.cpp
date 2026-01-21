#include "WindowBase.hpp"
#include "Time.hpp"

namespace ogfx
{
	WindowBase::~WindowBase(void)
	{
		onDestroy();
		SDL_FreeCursor(m_cursor_IBeam);
		SDL_FreeCursor(m_cursor_Arrow);
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		// IMG_Quit();
		SDL_Quit();
		TTF_Quit();
	}

	void WindowBase::initialize(int32_t width, int32_t height, const ostd::String& windowTitle)
	{
		if (m_initialized) return;
		m_windowWidth = width;
		m_windowHeight = height;
		m_title = windowTitle;
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			printf( "SDL could not initialize! Error: %s\n", SDL_GetError() );
			exit(1);
		}
		// int imgFlags = IMG_INIT_PNG;
		// if (!(IMG_Init(imgFlags) & imgFlags))
		// {
		// 	printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
		// 	exit(2);
		// }
		m_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_windowWidth, m_windowHeight, SDL_WINDOW_RESIZABLE);
		SDL_SetWindowResizable(m_window, SDL_FALSE);
		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
		SDL_SetWindowMinimumSize(m_window, m_windowWidth, m_windowHeight);
		SDL_SetWindowTitle(m_window, m_title.c_str());

		m_cursor_Arrow = SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_ARROW);
		m_cursor_IBeam = SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_IBEAM);

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

		setTypeName("dragon::WindowBase");
		enableSignals(true);
		validate();

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
		SDL_SetWindowResizable(m_window, (enable ? SDL_TRUE : SDL_FALSE));
	}

	void WindowBase::__handle_events(void)
	{
		if (!m_initialized) return;
		auto l_getMouseState = [this](void) -> MouseEventData {
			int32_t mx = 0, my = 0;
			uint32_t btn = SDL_GetMouseState(&mx, &my);
			MouseEventData::eButton button = MouseEventData::eButton::None;
			switch (btn)
			{
				case SDL_BUTTON(1): button = MouseEventData::eButton::Left; break;
				case SDL_BUTTON(2): button = MouseEventData::eButton::Middle; break;
				case SDL_BUTTON(3): button = MouseEventData::eButton::Right; break;
				default: button = MouseEventData::eButton::None; break;
			}
			MouseEventData mmd(*this, mx, my, button);
			return mmd;
		};
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				m_running = false;
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowClosed, ostd::tSignalPriority::Normal, *this);
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					WindowResizedData wrd(*this, m_windowWidth, m_windowHeight, 0, 0);
					SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
					wrd.new_width = m_windowWidth;
					wrd.new_height = m_windowHeight;
					ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
				}
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				MouseEventData mmd = l_getMouseState();
				if (isMouseDragEventEnabled() && mmd.button != MouseEventData::eButton::None)
					ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseDragged, ostd::tSignalPriority::RealTime, mmd);
				else
					ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseMoved, ostd::tSignalPriority::RealTime, mmd);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				MouseEventData mmd = l_getMouseState();
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MousePressed, ostd::tSignalPriority::RealTime, mmd);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				MouseEventData mmd = l_getMouseState();
				switch (event.button.button)
				{
					case SDL_BUTTON(1): mmd.button = MouseEventData::eButton::Left; break;
					case SDL_BUTTON(2): mmd.button = MouseEventData::eButton::Middle; break;
					case SDL_BUTTON(3): mmd.button = MouseEventData::eButton::Right; break;
					default: mmd.button = MouseEventData::eButton::None; break;
				}
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseReleased, ostd::tSignalPriority::RealTime, mmd);
			}
			else if (event.type == SDL_KEYDOWN)
			{
				KeyEventData ked(*this, (int32_t)event.key.keysym.sym, 0, KeyEventData::eKeyEvent::Pressed);
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyPressed, ostd::tSignalPriority::RealTime, ked);
			}
			else if (event.type == SDL_KEYUP)
			{
				KeyEventData ked(*this, (int32_t)event.key.keysym.sym, 0, KeyEventData::eKeyEvent::Released);
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyReleased, ostd::tSignalPriority::RealTime, ked);
			}
			else if (event.type == SDL_TEXTINPUT)
			{
				KeyEventData ked(*this, 0, event.text.text[0], KeyEventData::eKeyEvent::Text);
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::TextEntered, ostd::tSignalPriority::RealTime, ked);
			}
		}
	}
}
