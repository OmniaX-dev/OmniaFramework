#include "Window.hpp"
#include "Logger.hpp"
#include "Signals.hpp"

namespace ogfx
{

	Window& Window::create(ostd::UI16Point size, ostd::String title, tContextSettings contextSettings)
	{
		sf::ContextSettings settings;
		settings.depthBits = contextSettings.depthBits;
		settings.stencilBits = contextSettings.stencilBits;
		settings.antialiasingLevel = contextSettings.antiAliasLevel;
		settings.majorVersion = contextSettings.GLMajorVersion;
		settings.minorVersion = contextSettings.GLMinorVersion;
		settings.attributeFlags = (contextSettings.GLProfile == tContextSettings::tGLProfile::Core ? settings.Core : settings.Default);

		m_sfml_window.create(sf::VideoMode(size.x, size.y), title, sf::Style::Default, settings);
		m_sfml_window.setActive(true);
		m_sfml_window.setKeyRepeatEnabled(false);
		enableVSync();

		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction)))
		{
			OX_FATAL("Failed to initialize GLAD.");
			return *this;
		}

		glViewport(0, 0, size.x, size.y);
		glEnable(GL_BLEND);
		
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		setTypeName("ogfx::Window"); 
		validate();
		__calc_win_deco_sizes();
		return *this;
	}

	void Window::handleEvents(void)
	{
		sf::Event event;
		while (m_sfml_window.pollEvent(event))
		{
			Event evt(event);
			if (event.type == sf::Event::Resized)
			{
				WindowSizeObj wsobj(event.size.width, event.size.height);
				SignalHandler::emitSignal(tBuiltinSignals::WindowResized, tSignalPriority::RealTime, wsobj);
			}
			else
				SignalHandler::emitSignal(tBuiltinSignals::OnGuiEvent, tSignalPriority::RealTime, evt);
		}
	}

	Window& Window::centerMouse(void)
	{
		m_disable_mouse_moved_event = true;
		sf::Vector2i mpos = m_sfml_window.getPosition();
		sf::Vector2i deco(getBorderSize(), getTitleBarSize());
		mpos += sf::Vector2i(m_sfml_window.getSize().x / 2, m_sfml_window.getSize().y / 2);
		sf::Mouse::setPosition(mpos + deco);
		return *this;
	}

	void Window::printOpenGLInfo(void)
	{
        OX_INFO("OpenGL Version: %s", glGetString(GL_VERSION));
        int nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        OX_INFO("Maximum number of vertex attributes supported: %d", nrAttributes);
        int nTexUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nTexUnits);
        OX_INFO("Maximum number of Texture Units (per Stage): %d", nTexUnits);
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nTexUnits);
        OX_INFO("Maximum number of Texture Units (Total): %d", nTexUnits);
	}

	void Window::__calc_win_deco_sizes(void)
	{
		if (isInvalid()) return;
		auto mousePos = sf::Mouse::getPosition();
		sf::Vector2i deco { 0, 0 };
		sf::Vector2i offset { 100, 100 };
		sf::Vector2i npos = m_sfml_window.getPosition() + offset;
		sf::Mouse::setPosition(npos);
		auto mposr = sf::Mouse::getPosition(m_sfml_window);
		deco = offset - mposr;
		sf::Mouse::setPosition(mousePos);
		m_win_deco_sizes = deco;
	}
}
