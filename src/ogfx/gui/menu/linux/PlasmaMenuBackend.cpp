#include "PlasmaMenuBackend.hpp"
#include "X11AppMenuRegistrar.hpp"
#include "WaylandAppMenuBinder.hpp"

namespace ogfx
{
	namespace gui
	{
		PlasmaMenuBackend::PlasmaMenuBackend(void)
		{
		}

		PlasmaMenuBackend::~PlasmaMenuBackend(void)
		{
			detach();
		}

		bool PlasmaMenuBackend::isAvailable(WindowCore& window)
		{
			// The real compositor/registrar-presence check happens in attach() (it needs a live
			// DBus connection to probe), so this stays a cheap, side-effect-free check of
			// whether we're on a session type we know how to handle at all.
			auto sessionType = NativeWindowHandle::getSessionType();
			return sessionType == NativeWindowHandle::eSessionType::X11
				|| sessionType == NativeWindowHandle::eSessionType::Wayland;
		}

		bool PlasmaMenuBackend::attach(WindowCore& window, std::function<void(i32)> activateCallback)
		{
			if (!m_server.connect())
				return false;
			m_server.setActivateCallback(activateCallback);

			auto sessionType = NativeWindowHandle::getSessionType();
			if (sessionType == NativeWindowHandle::eSessionType::X11)
			{
				if (!X11AppMenuRegistrar::isAvailable(m_server.getConnection()))
				{
					m_server.disconnect();
					return false;
				}
				m_x11WindowId = NativeWindowHandle::getX11WindowId(window);
				m_x11Registered = X11AppMenuRegistrar::registerWindow(m_server.getConnection(), m_x11WindowId, m_server.getServiceName(), m_server.getObjectPath());
				if (!m_x11Registered)
				{
					m_server.disconnect();
					return false;
				}
				return true;
			}

			if (sessionType == NativeWindowHandle::eSessionType::Wayland)
			{
				void* display = NativeWindowHandle::getWaylandDisplay(window);
				void* surface = NativeWindowHandle::getWaylandSurface(window);
				m_waylandBinder = std::make_unique<WaylandAppMenuBinder>();
				if (!m_waylandBinder->bind(display, surface))
				{
					m_waylandBinder.reset();
					m_server.disconnect();
					return false;
				}
				m_waylandBinder->setAddress(m_server.getServiceName(), m_server.getObjectPath());
				return true;
			}

			m_server.disconnect();
			return false;
		}

		void PlasmaMenuBackend::setMenu(const stdvec<ContextMenu::Entry>& topLevel)
		{
			m_server.setMenu(topLevel);
		}

		void PlasmaMenuBackend::detach(void)
		{
			if (m_x11Registered)
			{
				X11AppMenuRegistrar::unregisterWindow(m_server.getConnection(), m_x11WindowId);
				m_x11Registered = false;
			}
			if (m_waylandBinder)
			{
				m_waylandBinder->release();
				m_waylandBinder.reset();
			}
			m_server.disconnect();
		}

		void PlasmaMenuBackend::update(void)
		{
			m_server.update();
		}
	}
}
