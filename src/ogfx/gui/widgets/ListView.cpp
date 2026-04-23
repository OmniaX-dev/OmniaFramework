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

#include "ListView.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../Window.hpp"
#include "../../../ostd/math/Random.hpp"

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			void ListView::Item::setText(const String& text)
			{
				if (m_text == text)
					return;
				m_text = text;
				update_dimensions();
			}

			void ListView::Item::setFontSize(i32 fontSize)
			{
				if (m_fontSize == fontSize)
					return;
				m_fontSize = fontSize;
				update_dimensions();
			}

			void ListView::Item::update_dimensions(void)
			{
				if (!isValid())
					return;
				m_dimensions = m_parent->getWindow().getGFX().getStringDimensions(m_text, m_fontSize);
				m_dimensions += Vec2 { m_padding.x + m_padding.w, m_padding.y + m_padding.h };
			}




			ListView& ListView::create(void)
			{
				setPadding({ 0, 0, 0, 0 });
				setTypeName("ogfx::gui::widgets::ListView");
				enableStopEvents();
				enableBackground();
				enableBorder();
				setBackgroundColor({ 160, 160, 160 });
				setBorderColor({ 50, 50, 50 });
				setStylesheetCategoryName("list");
				validate();
				for (i32 i = 0; i < 30; i++)
				{
					Item k { *this };
					m_list.push_back(k);
				}
				return *this;
			}

			void ListView::applyTheme(const ostd::Stylesheet& theme)
			{
			}

			void ListView::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				f32 y = 0;
				const auto& bounds = getGlobalBounds();
				for (auto& item : m_list)
				{
					if (!item.isValid()) continue;
					const auto& size = item.getDimensions();
					gfx.drawString(item, bounds.getPosition() + Vec2 { 0, y } + getScrollOffset() + item.getPadding().getPosition(), item.getTextColor(), item.getFontSize());
					y += size.y;
				}
			}

			void ListView::afterDraw(ogfx::BasicRenderer2D& gfx)
			{
				drawScrollbars(gfx);
			}

			Rectangle ListView::getContentExtents(void) const
			{
				f32 maxX = 0, maxY = 0;
				for (auto& item : m_list)
				{
					if (!item.isValid()) continue;
					Vec2 size = item.getDimensions();
					maxX = std::max(maxX, size.x);
					maxY += size.y;
				}
				return { 0, 0, maxX, maxY };
			}

			void ListView::refresh(void)
			{
				for (auto& item : m_list)
				{
					item.setText(ostd::Random::getString(ostd::Random::getui8(0, 40)));
				}

				m_list[10].setFontSize(40);
				m_list[16].setTextColor(Colors::Crimson);
			}
		}
	}
}
