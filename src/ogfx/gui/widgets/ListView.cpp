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
		void ListView::Item::setText(const String& text)
		{
			if (m_text == text)
				return;
			m_text = text;
			update_dimensions();
			if (m_parent) m_parent->m_extentsDirty = true;
		}

		void ListView::Item::setFontSize(i32 fontSize)
		{
			if (m_fontSize == fontSize)
				return;
			m_fontSize = fontSize;
			update_dimensions();
			if (m_parent) m_parent->m_extentsDirty = true;
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
			setTypeName("ogfx::gui::ListView");
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
			const auto& bounds = getGlobalBounds();
			const auto& content = getContentExtents();
			const f32 scrollY = -getScrollOffset().y;
			const f32 visibleH = getContentBounds().h;
			const f32 visibleStart = scrollY;
			const f32 visibleEnd  = scrollY + visibleH;
			Color textColor;

			f32 y = 0;
			i32 startIdx = 0;
			for (auto& item : m_list)
			{
				if (!item.isValid()) { startIdx++; continue; }
				f32 itemH = item.getDimensions().y;
				if (y + itemH > visibleStart) break;
				y += itemH;
				startIdx++;
			}

			for (i32 i = startIdx; i < (i32)m_list.size(); i++)
			{
				auto& item = m_list[i];
				if (!item.isValid()) continue;
				f32 itemH = item.getDimensions().y;
				if (y > visibleEnd) break;
				f32 lineW = content.w;
				if (content.w < bounds.w)
					lineW = bounds.w;
				if (item.isSelected())
				{
					textColor = item.getSelectedTextColor();
					gfx.fillRect({ Vec2 { bounds.x, bounds.y + y + item.getPadding().h } + getScrollOffset(), { lineW, itemH } }, item.getSelectedColor());
				}
				else
					textColor = item.getTextColor();
				gfx.drawLine({ Vec2 { bounds.x, bounds.y + y + itemH + item.getPadding().h } + getScrollOffset(), Vec2 { bounds.x + lineW, bounds.y + y + itemH + item.getPadding().h } + getScrollOffset() }, getSeparatorLineColor(), 1);
				gfx.drawString(item, bounds.getPosition() + Vec2 { 0, y } + getScrollOffset() + item.getPadding().getPosition(), textColor, item.getFontSize());
				y += itemH;
			}
		}

		void ListView::afterDraw(ogfx::BasicRenderer2D& gfx)
		{
			drawScrollbars(gfx);
		}

		void ListView::onMouseReleased(const Event& event)
		{
			if (!isMouseInside())
				return;
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
				if (localY >= y && localY < y + itemH)
				{
					bool wasSelected = item.isSelected();
					item.set_selected(m_selectedList);
					if (!wasSelected && callback_onSelectionChanged)
						callback_onSelectionChanged(m_selectedList);
					event.handle();
					break;
				}
				y += itemH;
			}
		}

		Rectangle ListView::getContentExtents(void) const
		{

			if (!m_extentsDirty)
				return m_cachedExtents;
			f32 maxX = 0, maxY = 0;
			for (auto& item : m_list)
			{
				if (!item.isValid()) continue;
				Vec2 size = item.getDimensions();
				maxX = std::max(maxX, size.x);
				maxY += size.y;
			}
			m_cachedExtents = { 0, 0, maxX, maxY };
			m_extentsDirty = false;
			return m_cachedExtents;
		}

		ListView::Item& ListView::getLine(const String& text)
		{
			for (auto& item : m_list)
				if (item.getText() == text)
					return item;
			return InvalidItem;
		}

		ListView::Item& ListView::getLine(u32 index)
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
			if (m_list.size() > 0)
				m_list[0].set_selected(m_selectedList);
			m_extentsDirty = true;
			return m_list.back();
		}

		bool ListView::removeLine(const String& text)
		{
			auto it = std::find_if(m_list.begin(), m_list.end(), [&](const Item& item) {
				return item.getText() == text;
			});
			if (it != m_list.end())
			{
				m_list.erase(it);
				return true;
			}
			return false;
		}

		bool ListView::removeLine(u32 index)
		{
			if (!hasLine(index))
				return false;
			m_list.erase(m_list.begin() + index);
			return true;
		}

		bool ListView::hasLine(const String& text)
		{
			for (auto& item : m_list)
				if (item.getText() == text)
					return true;
			return false;
		}

		bool ListView::hasLine(u32 index)
		{
			return m_list.size() > index;
		}
	}
}
