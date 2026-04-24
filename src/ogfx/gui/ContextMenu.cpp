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

#include "ContextMenu.hpp"
#include "../render/BasicRenderer.hpp"
#include "Window.hpp"

namespace ogfx
{
	namespace gui
	{
		void ContextMenu::Entry::update_size(BasicRenderer2D& gfx, i32 fontSize)
		{
			size = { 0, 0 };
			for (auto& entry : submenus)
			{
				auto s = gfx.getStringDimensions(entry.text, fontSize);
				if (s.x > size.x)
					size.x = s.x + (entry.submenus.size() > 0 ? 40 : 0);
				size.y += s.y;
				entry.update_size(gfx, fontSize);
			}
		}





		ContextMenu& ContextMenu::create(void)
		{
			return *this;
		}

		void ContextMenu::applyTheme(const ostd::Stylesheet& theme)
		{
			setPadding(theme.get<Rectangle>("context.padding", getPadding(), {}, {}));
			setItemSpacing(theme.get<f32>("context.itemSpacing", getItemSpacing(), {}, {}));
			setFontSize(theme.get<i32>("context.fontSize", getFontSize(), {}, {}));
			setBackgroundColor(theme.get<Color>("context.backgroundColor", getBackgroundColor(), {}, {}));
			setSelectionColor(theme.get<Color>("context.selectionColor", getSelectionColor(), {}, {}));
			setSelectionTextColor(theme.get<Color>("context.selectionTextColor", getSelectionTextColor(), {}, {}));
			setSeparatorLineColor(theme.get<Color>("context.separatorLineColor", getSeparatorLineColor(), {}, {}));
			setTextColor(theme.get<Color>("context.textColor", getTextColor(), {}, {}));
			setSubmenuIndicatorColor(theme.get<Color>("context.submenuIndicatorColor", getSubmenuIndicatorColor(), {}, {}));
			setBorderColor(theme.get<Color>("context.borderColor", getBorderColor(), {}, {}));
			enableSelectionGradient(theme.get<bool>("context.useSelectionGradient", isSelectionGradientEnabled(), {}, {}));
			setSelectionGradient(theme.get<ColorGradient>("context.selectionGradient", getSelectionGradient(), {}, {}));
		}

		void ContextMenu::draw(BasicRenderer2D& gfx)
		{
			if (!m_visible) return;
			for (const auto& panel : m_panels)
				draw_panel(gfx, panel);
		}

		void ContextMenu::update(void)
		{
			if (!m_visible) return;

			if (m_pendingOpenPanelDepth >= 0 && m_hoverOpenTimerActive && m_hoverOpenTimer.read() >= SubmenuOpenDelayMs)
			{
				Panel& parent = m_panels[m_pendingOpenPanelDepth];
				i32 entryIdx = m_pendingOpenEntryIndex;
				Entry& e = (*parent.entries)[entryIdx];

				pop_to_depth(m_pendingOpenPanelDepth);

				Rectangle r = entry_rect(parent, entryIdx);
				Vec2 anchor { parent.position.x + parent.size.x, r.y };
				push_panel(e.submenus, anchor, false);
				m_panels[m_pendingOpenPanelDepth].openedSubmenuIndex = entryIdx;

				m_hoverOpenTimer.endCount();
				m_hoverOpenTimerActive = false;
				m_pendingOpenPanelDepth = -1;
				m_pendingOpenEntryIndex = -1;
			}

			if (m_hoverCloseTimerActive && m_hoverCloseTimer.read() >= SubmenuCloseDelayMs)
			{
				i32 keepDepth = -1;
				for (i32 i = 0; i < (i32)m_panels.size(); ++i)
				{
					Rectangle r { m_panels[i].position, m_panels[i].size };
					if (r.contains(m_mousePos, true)) keepDepth = i;
				}
				if (keepDepth >= 0)
				{
					pop_to_depth(keepDepth);
					m_panels[keepDepth].openedSubmenuIndex = -1;
				}
				m_hoverCloseTimer.endCount();
				m_hoverCloseTimerActive = false;
			}
		}

		void ContextMenu::onMouseReleased(const Event& event)
		{
			if (!m_visible) return;
			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };

			for (i32 i = (i32)m_panels.size() - 1; i >= 0; --i)
			{
				i32 idx = entry_index_at(m_panels[i], mousePos);
				if (idx >= 0)
				{
					Entry& e = (*m_panels[i].entries)[idx];
					if (e.submenus.empty())
					{
						if (m_data.onActivate && e.id >= 0)
							m_data.onActivate(e);
						hide();
						event.handle();
						hide();
					}
					else
					{
						pop_to_depth(i);
						Rectangle r = entry_rect(m_panels[i], idx);
						Vec2 anchor { m_panels[i].position.x + m_panels[i].size.x, r.y };
						push_panel(e.submenus, anchor, false);
						m_panels[i].openedSubmenuIndex = idx;
					}
					event.handle();
					return;
				}
			}

			hide();
			event.handle();
		}

		void ContextMenu::onMouseMoved(const Event& event)
		{
			if (!m_visible) return;
			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };
			m_mousePos = mousePos;

			i32 hitPanel = -1;
			i32 hitEntry = -1;
			for (i32 i = (i32)m_panels.size() - 1; i >= 0; --i)
			{
				i32 idx = entry_index_at(m_panels[i], mousePos);
				if (idx >= 0) { hitPanel = i; hitEntry = idx; break; }
				Rectangle panelRect { m_panels[i].position, m_panels[i].size };
				if (panelRect.contains(mousePos, true)) { hitPanel = i; hitEntry = -1; break; }
			}

			for (i32 i = 0; i < (i32)m_panels.size(); ++i)
				m_panels[i].hoveredIndex = (i == hitPanel ? hitEntry : -1);

			if (hitPanel < 0)
			{
				if (!m_hoverCloseTimerActive)
				{
					m_hoverCloseTimer.startCount(ostd::eTimeUnits::Milliseconds);
					m_hoverCloseTimerActive = true;
				}
				if (m_hoverOpenTimerActive)
				{
					m_hoverOpenTimer.endCount();
					m_hoverOpenTimerActive = false;
				}
				m_pendingOpenPanelDepth = -1;
				event.handle();
				return;
			}

			if ((i32)m_panels.size() - 1 > hitPanel)
			{
				if (hitEntry != m_panels[hitPanel].openedSubmenuIndex)
				{
					if (!m_hoverCloseTimerActive)
					{
						m_hoverCloseTimer.startCount(ostd::eTimeUnits::Milliseconds);
						m_hoverCloseTimerActive = true;
					}
				}
				else
				{
					if (m_hoverCloseTimerActive)
					{
						m_hoverCloseTimer.endCount();
						m_hoverCloseTimerActive = false;
					}
				}
			}
			else
			{
				if (m_hoverCloseTimerActive)
				{
					m_hoverCloseTimer.endCount();
					m_hoverCloseTimerActive = false;
				}
			}

			if (hitEntry >= 0)
			{
				Entry& e = (*m_panels[hitPanel].entries)[hitEntry];
				bool hasChildren = e.submenus.size() > 0;
				bool alreadyOpen = (m_panels[hitPanel].openedSubmenuIndex == hitEntry) && ((i32)m_panels.size() - 1 > hitPanel);

				if (hasChildren && !alreadyOpen)
				{
					if (m_pendingOpenPanelDepth != hitPanel || m_pendingOpenEntryIndex != hitEntry)
					{
						m_pendingOpenPanelDepth = hitPanel;
						m_pendingOpenEntryIndex = hitEntry;
						m_hoverOpenTimer.startCount(ostd::eTimeUnits::Milliseconds);
						m_hoverOpenTimerActive = true;
					}
				}
				else
				{
					if (m_hoverOpenTimerActive)
					{
						m_hoverOpenTimer.endCount();
						m_hoverOpenTimerActive = false;
					}
					m_pendingOpenPanelDepth = -1;
				}
			}

			event.handle();
		}

		void ContextMenu::onMousePressed(const Event& event)
		{
			event.handle();
		}

		void ContextMenu::onMouseScrolled(const Event& event)
		{
			event.handle();
		}

		void ContextMenu::show(const Vec2& pos)
		{
			m_panels.clear();
			push_panel(m_data.entries, pos, false);
			m_visible = true;
		}

		void ContextMenu::hide(void)
		{
			m_panels.clear();
			m_hoverOpenTimer.endCount();
			m_hoverCloseTimer.endCount();
			m_hoverOpenTimerActive = false;
			m_hoverCloseTimerActive = false;
			m_pendingOpenPanelDepth = -1;
			m_pendingOpenEntryIndex = -1;
			m_visible = false;
		}

		void ContextMenu::setInstance(const Instance& instance)
		{
			m_data = instance;
			m_panels.clear();
			m_pendingOpenPanelDepth = -1;
			m_pendingOpenEntryIndex = -1;
			m_hoverOpenTimer.endCount();
			m_hoverCloseTimer.endCount();
			if (m_visible)
				push_panel(m_data.entries, getPosition(), false);
		}

		void ContextMenu::setFontSize(i32 size)
		{
			m_fontSize = size;
			relayout_panels();
		}

		void ContextMenu::push_panel(stdvec<Entry>& entries, const Vec2& anchorTopLeft, bool flipLeft)
		{
			Panel p;
			p.entries = &entries;
			p.position = anchorTopLeft;
			compute_panel_size(p);

			f32 windowW = (f32)m_window.getWindowWidth();
			f32 windowH = (f32)m_window.getWindowHeight();
			if (flipLeft && !m_panels.empty())
			{
				// anchorTopLeft was computed assuming "open right"; flip to open left
				p.position.x = m_panels.back().position.x - p.size.x;
			}
			else if (p.position.x + p.size.x > windowW && !m_panels.empty())
			{
				p.position.x = m_panels.back().position.x - p.size.x;
			}

			if (p.position.y + p.size.y > windowH)
				p.position.y = windowH - p.size.y;
			if (p.position.y < 0) p.position.y = 0;
			if (p.position.x < 0) p.position.x = 0;

			m_panels.push_back(p);
		}

		void ContextMenu::compute_panel_size(Panel& panel)
		{
			panel.size = { 0, 0 };
			auto& gfx = m_window.getGFX();
			for (auto& entry : *panel.entries)
			{
				auto s = gfx.getStringDimensions(entry.text, getFontSize());
				f32 extra = (entry.submenus.size() > 0 ? s.y : 0);
				if (s.x + extra > panel.size.x)
					panel.size.x = s.x + extra;
				panel.size.y += s.y + m_spacing;
				panel.entryHeight = s.y + m_spacing;
			}
			panel.size.x += m_padding.x + m_padding.w;
			panel.size.y += m_padding.y + m_padding.h;
		}

		void ContextMenu::pop_to_depth(size_t depth)
		{
			if (depth + 1 < m_panels.size())
				m_panels.resize(depth + 1);
		}

		Rectangle ContextMenu::entry_rect(const Panel& panel, i32 index) const
		{
			return {
				panel.position.x,
				panel.position.y + m_padding.y + (index * panel.entryHeight),
				panel.size.x,
				panel.entryHeight
			};
		}

		i32 ContextMenu::entry_index_at(const Panel& panel, const Vec2& mousePos) const
		{
			Rectangle panelRect { panel.position, panel.size };
			if (!panelRect.contains(mousePos, true)) return -1;
			for (i32 i = 0; i < (i32)panel.entries->size(); ++i)
			{
				if (entry_rect(panel, i).contains(mousePos, true))
					return i;
			}
			return -1;
		}

		void ContextMenu::relayout_panels(void)
		{
			for (auto& panel : m_panels)
				compute_panel_size(panel);
		}

		void ContextMenu::draw_panel(BasicRenderer2D& gfx, const Panel& panel)
		{
			gfx.fillRect({ panel.position, panel.size }, getBackgroundColor());
			const f32 triPad = 14;
			f32 y = 0;
			i32 i = 0;
			auto& entries = *panel.entries;
			for (auto& entry : entries)
			{
				Vec2 entryPos = panel.position + Vec2 { m_padding.x, y + m_padding.y };
				Rectangle rect = { entryPos - Vec2 { m_padding.x, 0 }, panel.size.x, panel.entryHeight };

				bool highlighted = (panel.hoveredIndex == i)
								|| (panel.openedSubmenuIndex == i);

				if (highlighted)
				{
					if (isSelectionGradientEnabled())
						gfx.fillGradientRect(rect, m_selectionGradient);
					else
						gfx.fillRect(rect, getSelectionColor());
					gfx.drawRect(rect, getSeparatorLineColor(), 1, false, false, i != entries.size() , false);
					gfx.drawVCenteredString(entry.text, { entryPos, panel.size.x, panel.entryHeight }, getSelectionTextColor(), getFontSize());
				}
				else
				{
					gfx.drawRect(rect, getSeparatorLineColor(), 1, false, false, i != entries.size() , false);
					gfx.drawVCenteredString(entry.text, { entryPos, panel.size.x, panel.entryHeight }, getTextColor(), getFontSize());
				}
				if (entry.submenus.size() > 0)
				{
					Triangle tri {
						{ rect.x + rect.w - panel.entryHeight + triPad, rect.y + (triPad * 0.5f) },
						{ rect.x + rect.w - (triPad * 0.5f), rect.y + (rect.h * 0.5f) },
						{ rect.x + rect.w - panel.entryHeight + triPad, rect.y + rect.h - (triPad * 0.5f) }
					};
					gfx.fillTriangle(tri, getSubmenuIndicatorColor());
				}
				y += panel.entryHeight;
				i++;
			}
			gfx.drawRect({ panel.position, panel.size }, getBorderColor(), 2);
		}
	}
}
