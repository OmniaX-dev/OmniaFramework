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
					size.x = s.x;
				size.y += s.y;
				entry.update_size(gfx, fontSize);
			}
		}

		ContextMenu& ContextMenu::create(void)
		{
			setSize(130, 300);
			return *this;
		}

		void ContextMenu::draw(BasicRenderer2D& gfx)
		{
			gfx.outlinedRect(*this, Colors::Crimson, Colors::Black, 2);
			f32 y = 0;
			for (auto& entry : m_data.entries)
			{
				Vec2 entryPos = getPosition() + Vec2 { m_padding.x, y + m_padding.y };
				Rectangle rect = { entryPos - Vec2 { m_padding.x, 0 }, getw(), m_entryHeight };
				if (rect.contains(m_mousePos, true))
					gfx.outlinedRect(rect, Colors::Chocolate, Colors::White, 1);
				gfx.drawString(entry.text, entryPos, Colors::Black, getFontSize());
				y += m_entryHeight;
			}
		}

		void ContextMenu::onMouseReleased(const Event& event)
		{
			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };
			if (contains(mousePos, true))
			{

			}
			else
			{
				hide();
			}
			event.handle();
		}

		void ContextMenu::onMouseMoved(const Event& event)
		{
			Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };
			if (contains(mousePos, true))
			{
				m_mousePos = mousePos;
			}
			event.handle();
		}

		void ContextMenu::show(void)
		{
			m_visible = true;
		}

		void ContextMenu::show(const Vec2& pos)
		{
			setPosition(pos);
			m_visible = true;
		}

		void ContextMenu::hide(void)
		{
			m_visible = false;
		}

		void ContextMenu::setInstance(const Instance& instance)
		{
			m_data = instance;
			update_size();
		}

		void ContextMenu::setFontSize(i32 size)
		{
			m_fontSize = size;
			update_size();
		}

		void ContextMenu::update_size(void)
		{
			setSize(0, 0);
			auto& gfx = m_window.getGFX();
			for (auto& entry : m_data.entries)
			{
				auto s = gfx.getStringDimensions(entry.text, getFontSize());
				if (s.x > getw())
					setw(s.x);
				addh(s.y);
				m_entryHeight = s.y;
				entry.update_size(gfx, getFontSize());
			}
			addSize({ m_padding.x + m_padding.w, m_padding.y + m_padding.h });
		}
	}
}
