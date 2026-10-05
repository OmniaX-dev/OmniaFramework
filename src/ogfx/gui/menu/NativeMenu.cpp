#include "NativeMenu.hpp"
#include "NoopMenuBackend.hpp"
#ifdef __APPLE__
#include "macos/CocoaMenuBackend.hpp"
#endif
#ifdef __linux__
#include "linux/PlasmaMenuBackend.hpp"
#endif

namespace ogfx
{
	namespace gui
	{
		void NativeMenu::register_candidates(void)
		{
			// NoopMenuBackend is always last, so anything unsupported falls through to it and
			// the in-widget MenuBar keeps working exactly as it does today.
			#ifdef __linux__
			m_candidates.push_back(std::make_unique<PlasmaMenuBackend>());
			#endif
			#ifdef __APPLE__
			m_candidates.push_back(std::make_unique<CocoaMenuBackend>());
			#endif
			m_candidates.push_back(std::make_unique<NoopMenuBackend>());
		}

		bool NativeMenu::enable(void)
		{
			for (auto& candidate : m_candidates)
			{
				if (!candidate->isAvailable(m_window))
					continue;
				if (candidate->attach(m_window, [this](i32 id) { activate(id); }))
				{
					m_backend = candidate.get();
					return true;
				}
			}
			m_backend = nullptr;
			return false;
		}

		void NativeMenu::disable(void)
		{
			if (m_backend)
				m_backend->detach();
			m_backend = nullptr;
		}

		void NativeMenu::activate(i32 id)
		{
			auto it = m_activationMap.find(id);
			if (it != m_activationMap.end() && it->second)
				it->second();
		}

		void NativeMenu::assign_ids_and_bind(stdvec<ContextMenu::Entry>& entries, const ContextMenu::Instance::Callback& onActivate)
		{
			for (auto& e : entries)
			{
				if (e.isSeparator)
					continue;

				// Mirrors ContextMenu::onMouseReleased's own gate (id >= 0 required to fire
				// onActivate) - an entry the app left at the default id=-1 stays inert here too,
				// even though it still needs SOME unique id for the backend's own bookkeeping.
				const bool wasActivatable = (e.id >= 0);
				if (e.id < 0)
					e.id = m_nextSyntheticId--;

				if (!e.submenus.empty())
				{
					assign_ids_and_bind(e.submenus, onActivate);
					continue; // parents aren't individually activated even if wasActivatable
				}

				if (wasActivatable)
				{
					ContextMenu::Entry entryCopy = e;
					m_activationMap[e.id] = [onActivate, entryCopy]() { if (onActivate) onActivate(entryCopy); };
				}
			}
		}

		void NativeMenu::flatten(const stdvec<MenuBar::Menu>& menus, stdvec<ContextMenu::Entry>& outTopLevel)
		{
			m_activationMap.clear();
			m_nextSyntheticId = -2;
			outTopLevel.clear();
			outTopLevel.reserve(menus.size());

			for (auto& menu : menus)
			{
				ContextMenu::Entry top(menu.label);
				top.submenus = menu.instance.entries;
				top.id = m_nextSyntheticId--; // top-level labels are never individually activatable
				assign_ids_and_bind(top.submenus, menu.instance.onActivate);
				outTopLevel.push_back(std::move(top));
			}
		}

		void NativeMenu::setMenu(const stdvec<MenuBar::Menu>& menus)
		{
			stdvec<ContextMenu::Entry> topLevel;
			flatten(menus, topLevel);
			if (m_backend)
				m_backend->setMenu(topLevel);
		}
	}
}
