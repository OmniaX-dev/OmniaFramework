#include "MenuBar.hpp"
#include "../render/BasicRenderer.hpp"
#include "Window.hpp"

namespace ogfx
{
	namespace gui
	{
		MenuBar& MenuBar::create(void)
		{
			setSize(m_window.getWindowWidth(), m_height);
			setPosition(0, 0);
			return *this;
		}

		MenuBar& MenuBar::addMenu(const String& label, const ContextMenu::Instance& instance)
		{
			m_slots.push_back({ label, instance, {} });
			recompute_layout();
			return *this;
		}

		MenuBar& MenuBar::addMenu(const Menu& menu)
		{
			return addMenu(menu.label, menu.instance);
		}

		void MenuBar::clearMenus(void)
		{
			close_menu();
			m_slots.clear();
		}

		void MenuBar::onWindowResized(const Event& event)
		{
			setSize(m_window.getWindowWidth(), m_height);
			setPosition(0, 0);
			recompute_layout();
		}

		void MenuBar::applyTheme(const ostd::Stylesheet& theme)
		{
			setHeight(theme.get<f32>("menubar.height", getHeight(), {}, {}));
			setItemPadding(theme.get<Rectangle>("menubar.itemPadding", getItemPadding(), {}, {}));
			setFontSize(theme.get<i32>("menubar.fontSize", getFontSize(), {}, {}));
			setBackgroundColor(theme.get<Color>("menubar.backgroundColor", getBackgroundColor(), {}, {}));
			setTextColor(theme.get<Color>("menubar.textColor", getTextColor(), {}, {}));
			setSelectionColor(theme.get<Color>("menubar.selectionColor", getSelectionColor(), {}, {}));
			setSelectionTextColor(theme.get<Color>("menubar.selectionTextColor", getSelectionTextColor(), {}, {}));
			setBorderColor(theme.get<Color>("menubar.borderColor", getBorderColor(), {}, {}));
			setSize(m_window.getWindowWidth(), m_height);
			recompute_layout();
		}

		void MenuBar::recompute_layout(void)
		{
			auto& gfx = m_window.getGFX();
			f32 x = m_itemPadding.x;
			for (auto& slot : m_slots)
			{
				Vec2 textSize = gfx.getStringDimensions(slot.label, m_fontSize);
				f32 slotW = textSize.x + m_itemPadding.x + m_itemPadding.w;
				slot.rect = { x, 0, slotW, m_height };
				x += slotW;
			}
		}

		i32 MenuBar::slot_index_at(const Vec2& mousePos) const
		{
			for (i32 i = 0; i < (i32)m_slots.size(); ++i)
			{
				if (m_slots[i].rect.contains(mousePos, true))
					return i;
			}
			return -1;
		}

		void MenuBar::open_menu(i32 index)
		{
			if (index < 0 || index >= (i32)m_slots.size()) return;

			const Slot& slot = m_slots[index];
			// Anchor menu just below the label
			Vec2 anchor { slot.rect.x, slot.rect.y + slot.rect.h };

			m_window.setContextMenu(slot.instance);
			m_window.showContextMenu(anchor);

			m_openIndex = index;
			m_active = true;
		}

		void MenuBar::close_menu(void)
		{
			if (m_window.isContextMenuVisible())
				m_window.hideContextMenu();
			m_openIndex = -1;
			m_active = false;
		}

		void MenuBar::draw(BasicRenderer2D& gfx)
		{
			if (!m_visible) return;

			gfx.fillRect(*this, m_backgroundColor);

			for (i32 i = 0; i < (i32)m_slots.size(); ++i)
			{
				const Slot& slot = m_slots[i];
				bool highlighted = (i == m_hoveredIndex) || (i == m_openIndex);

				if (highlighted)
				{
					gfx.fillRect(slot.rect, m_selectionColor);
					gfx.drawCenteredString(slot.label, slot.rect, m_selectionTextColor, m_fontSize);
				}
				else
				{
					gfx.drawCenteredString(slot.label, slot.rect, m_textColor, m_fontSize);
				}
			}

			// Bottom border line
			gfx.drawLine({ { getx(), gety() + m_height }, { getx() + getw(), gety() + m_height } }, m_borderColor);
		}

		void MenuBar::update(void)
		{
			if (!m_visible) return;

			// If we're in active mode but the context menu got dismissed
			// (item clicked, click-outside, escape, etc.), drop out of active mode.
			if (m_active && !m_window.isContextMenuVisible())
			{
				m_openIndex = -1;
				m_active = false;
			}
		}

		void MenuBar::onMousePressed(const Event& event)
		{
			if (!m_visible) return;

			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };
			i32 idx = slot_index_at(mousePos);

			if (idx < 0)
			{
				// Press outside the bar — let the context menu's own click-outside
				// handling close it; we'll catch up via update().
				return;
			}

			if (m_openIndex == idx)
			{
				// Clicking the open menu's own label closes it (toggle).
				close_menu();
			}
			else
			{
				open_menu(idx);
			}
			event.handle();
		}

		void MenuBar::onMouseMoved(const Event& event)
		{
			if (!m_visible) return;

			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };
			m_hoveredIndex = slot_index_at(mousePos);

			// Hover-switching: in active mode, moving onto a different label
			// instantly switches the open menu.
			if (m_active && m_hoveredIndex >= 0 && m_hoveredIndex != m_openIndex)
			{
				open_menu(m_hoveredIndex);
				event.handle();
			}
		}
	}
}
