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
#include "../../../ostd/io/Memory.hpp"

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

			void ListView::Item::set_selected(stdvec<Item*>& selectionList)
			{
				for (auto& sel : selectionList)
					sel->m_selected = false;
				selectionList.clear();
				selectionList.push_back(this);
				m_selected = true;
			}

			void ListView::Item::add_selected(stdvec<Item*>& selectionList)
			{
				if (STDVEC_CONTAINS(selectionList, this))
					return;
				selectionList.push_back(this);
				m_selected = true;
			}

			void ListView::Item::remove_selected(stdvec<Item*>& selectionList)
			{
				if (!STDVEC_CONTAINS(selectionList, this))
					return;
				STDVEC_REMOVE(selectionList, this);
				m_selected = false;
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
				return *this;
			}

			void ListView::applyTheme(const ostd::Stylesheet& theme)
			{
				setSeparatorLineColor(getThemeValue<Color>(theme, "separatorLineColor", getSeparatorLineColor()));
				enableShowSeparatorLine(getThemeValue<bool>(theme, "showSeparatorLine", isShowSeparatorLineEnabled()));
				setDefaultLineTextColor(getThemeValue<Color>(theme, "item.defaultTextColor", getDefaultLineTextColor()));
				setDefaultLineFontSize(getThemeValue<i32>(theme, "item.defaultFontSize", getDefaultLineFontSize()));
				setDefaultLinePadding(getThemeValue<Rectangle>(theme, "item.defaultPadding", getDefaultLinePadding()));
				setDefaultLineSelectionColor(getThemeValue<Color>(theme, "item.defaultSelectionColor", getDefaultLineSelectionColor()));
				setDefaultLineSelectionTextColor(getThemeValue<Color>(theme, "item.defaultSelectionTextColor", getDefaultLineSelectionTextColor()));
			}

			void ListView::onDraw(ogfx::BasicRenderer2D& gfx)
			{
				// f32 y = 0;
				const auto& bounds = getGlobalBounds();
				const auto& content = getContentExtents();
				Color textColor;
				// Compute the visible Y window in content space
				const f32 scrollY      = -getScrollOffset().y;           // positive offset into content
				const f32 visibleH     =  getContentBounds().h;
				const f32 visibleStart =  scrollY;
				const f32 visibleEnd   =  scrollY + visibleH;

				// Find the starting item and its Y via a linear scan...
				f32 y = 0;
				i32 startIdx = 0;
				for (auto& item : m_list)
				{
					if (!item.isValid()) { startIdx++; continue; }
					f32 itemH = item.getDimensions().y;
					if (y + itemH > visibleStart) break;   // this item is the first one in view
					y += itemH;
					startIdx++;
				}

				// ...then process only items from startIdx until we exceed visibleEnd
				for (i32 i = startIdx; i < (i32)m_list.size(); i++)
				{
					auto& item = m_list[i];
					if (!item.isValid()) continue;
					f32 itemH = item.getDimensions().y;
					if (y > visibleEnd) break;             // past the visible window, stop
					if (item.isSelected())
					{
						textColor = item.getSelectedTextColor();
						gfx.fillRect({ Vec2 { bounds.x, bounds.y + y + 4 } + getScrollOffset(), { content.w, itemH } }, item.getSelectedColor());
					}
					else
						textColor = item.getTextColor();
					gfx.drawLine({ Vec2 { bounds.x, bounds.y + y + itemH + 4 } + getScrollOffset(), Vec2 { bounds.x + content.w, bounds.y + y + itemH + 4 } + getScrollOffset() }, Colors::Black, 1);
					gfx.drawString(item, bounds.getPosition() + Vec2 { 0, y } + getScrollOffset() + item.getPadding().getPosition(), textColor, item.getFontSize());
					// draw or hit-test item at content Y = y
					y += itemH;
				}
				// for (auto& item : m_list)
				// {
				//     if (!item.isValid()) continue;
				//     const auto& size = item.getDimensions();
				//     if (item.isSelected())
				//     {
				//         textColor = item.getSelectedTextColor();
				//         gfx.fillRect({ Vec2 { bounds.x, bounds.y + y + 4 } + getScrollOffset(), { content.w, size.y } }, item.getSelectedColor());
				//     }
				//     else
				//         textColor = item.getTextColor();
				//     gfx.drawLine({ Vec2 { bounds.x, bounds.y + y + size.y + 4 } + getScrollOffset(), Vec2 { bounds.x + content.w, bounds.y + y + size.y + 4 } + getScrollOffset() }, Colors::Black, 1);
				//     gfx.drawString(item, bounds.getPosition() + Vec2 { 0, y } + getScrollOffset() + item.getPadding().getPosition(), textColor, item.getFontSize());
				//     y += size.y;
				// }
			}

			void ListView::afterDraw(ogfx::BasicRenderer2D& gfx)
			{
				drawScrollbars(gfx);
			}

			void ListView::onMouseReleased(const Event& event)
			{
				if (event.mouse->button != ogfx::MouseEventData::eButton::Left)
					return;
				if (isMouseInsideAnyScrollbar())
					return;

				const Vec2  mousePos   { event.mouse->position_x, event.mouse->position_y };
				const Vec2  origin     = getGlobalBounds().getPosition();
				const Vec2  scroll     = getScrollOffset();
				const f32   localY     = mousePos.y - origin.y - scroll.y;

				f32 y = 0;
				for (auto& item : m_list)
				{
					if (!item.isValid()) continue;
					const f32 itemH = item.getDimensions().y;
					if (!item.isSelected() && localY >= y && localY < y + itemH)
					{
						item.set_selected(m_selectedList);
						if (callback_onSelectionChanged)
							callback_onSelectionChanged(m_selectedList);
						event.handle();
						break;
					}
					y += itemH;
				}
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

			ListView::Item& ListView::getItem(const String& text)
			{
				for (auto& item : m_list)
					if (item.getText() == text)
						return item;
				return InvalidItem;
			}

			ListView::Item& ListView::getItem(u32 index)
			{
				if (index < m_list.size())
					return m_list[index];
				return InvalidItem;
			}

			ListView::Item& ListView::addLine(const String& text)
			{
				Item item { *this };
				item.setTextColor(m_defaultLinetextColor);
				item.setPadding(m_defaultLinepadding);
				item.setFontSize(m_defaultLinefontSize);
				item.setSelectedColor(m_defaultSelectionColor);
				item.setSelectedTextColor(m_defaultSelectionTextColor);
				item.setText(text);
				m_list.push_back(item);
				if (m_list.size() == 1)
					m_list.back().set_selected(m_selectedList);
				return m_list.back();
			}
		}
	}
}
