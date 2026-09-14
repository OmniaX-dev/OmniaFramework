/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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

#include "DetailList.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../../../ostd/io/Memory.hpp"
#include "../../../ostd/io/Logger.hpp"
#include <algorithm>

namespace ogfx
{
	namespace gui
	{
		// ============================== Row ==============================
		DetailList::Cell DetailList::Row::getCell(u32 col) const
		{
			if (col < m_cells.size())
				return m_cells[col];
			return Cell { String("") };
		}

		String DetailList::Row::getString(u32 col) const
		{
			if (col < m_cells.size())
			{
				if (auto p = std::get_if<String>(&m_cells[col]))
					return *p;
			}
			return "";
		}

		i64 DetailList::Row::getI64(u32 col) const
		{
			if (col < m_cells.size())
			{
				if (auto p = std::get_if<i64>(&m_cells[col]))
					return *p;
			}
			return 0;
		}

		u64 DetailList::Row::getU64(u32 col) const
		{
			if (col < m_cells.size())
			{
				if (auto p = std::get_if<u64>(&m_cells[col]))
					return *p;
			}
			return 0;
		}

		f64 DetailList::Row::getF64(u32 col) const
		{
			if (col < m_cells.size())
			{
				if (auto p = std::get_if<f64>(&m_cells[col]))
					return *p;
			}
			return 0.0;
		}

		bool DetailList::Row::setCell(u32 col, const Cell& value)
		{
			if (m_owner == nullptr || col >= m_owner->m_columns.size() || col >= m_cells.size())
				return false;
			if (!DetailList::cell_matches_type(value, m_owner->m_columns[col].type))
			{
				OX_WARN("DetailList: setCell() type mismatch on column %d (\"%s\").", (i32)col, m_owner->m_columns[col].name.c_str());
				return false;
			}
			m_cells[col] = value;
			if ((i32)col == m_owner->m_sortColumnIndex)
				m_owner->m_sortDirty = true;
			return true;
		}

		String DetailList::Row::cellToString(u32 col) const
		{
			if (m_owner == nullptr || col >= m_cells.size() || col >= m_owner->m_columns.size())
				return "";
			const auto& c = m_owner->m_columns[col];
			switch (c.type)
			{
				case eColumnType::String: return std::get<String>(m_cells[col]);
				case eColumnType::I64:    return String("").add(std::get<i64>(m_cells[col]));
				case eColumnType::U64:    return String("").add(std::get<u64>(m_cells[col]));
				case eColumnType::F64:    return String("").add(std::get<f64>(m_cells[col]), c.precision);
			}
			return "";
		}

		void DetailList::Row::set_selected(stdvec<Row*>& selectionList)
		{
			for (auto* sel : selectionList)
				sel->m_selected = false;
			selectionList.clear();
			selectionList.push_back(this);
			m_selected = true;
		}

		// ===================================================================




		// ============================ DetailList ============================
		DetailList& DetailList::create(void)
		{
			setPadding({ 0, 0, 0, 0 });
			setTypeName("ogfx::gui::DetailList");
			enableStopEvents();
			enableBackground();
			enableBorder();
			setBackgroundColor({ 160, 160, 160 });
			setBorderColor({ 50, 50, 50 });
			setStylesheetCategoryName("detailList");
			setContentOffset({ 0, m_headerHeight });
			updateScrollbarsSize();
			reloadTheme();
			validate();
			return *this;
		}

		void DetailList::setHeaderHeight(f32 height)
		{
			m_headerHeight = std::max(height, 0.0f);
			setContentOffset({ 0, m_headerHeight });
			updateScrollbarsSize();
		}

		void DetailList::applyTheme(const ostd::Stylesheet& theme)
		{
			setBackgroundColor2(getThemeValue<Color>(theme, "backgroundColor2", getBackgroundColor2()));
			enableAlternatingBackground(getThemeValue<bool>(theme, "showAlternatingBackground", isAlternatingBackgroundEnabled()));
			setSeparatorLineColor(getThemeValue<Color>(theme, "separatorLineColor", getSeparatorLineColor()));
			enableShowSeparatorLine(getThemeValue<bool>(theme, "showSeparatorLine", isShowSeparatorLineEnabled()));
			enableShowColumnSeparators(getThemeValue<bool>(theme, "showColumnSeparators", isShowColumnSeparatorsEnabled()));
			setCellPadding(getThemeValue<Rectangle>(theme, "cellPadding", getCellPadding()));
			setSelectionColor(getThemeValue<Color>(theme, "selectionColor", getSelectionColor()));
			setSelectionTextColor(getThemeValue<Color>(theme, "selectionTextColor", getSelectionTextColor()));

			setHeaderHeight(getThemeValue<f32>(theme, "header.height", getHeaderHeight()));
			setHeaderBackgroundColor(getThemeValue<Color>(theme, "header.backgroundColor", getHeaderBackgroundColor()));
			setHeaderTextColor(getThemeValue<Color>(theme, "header.textColor", getHeaderTextColor()));
			setHeaderBorderColor(getThemeValue<Color>(theme, "header.borderColor", getHeaderBorderColor()));
			setHeaderBorderWidth(getThemeValue<i32>(theme, "header.borderWidth", getHeaderBorderWidth()));
			setHeaderSortArrowColor(getThemeValue<Color>(theme, "header.sortArrowColor", getHeaderSortArrowColor()));
			setHeaderSortArrowSize(getThemeValue<f32>(theme, "header.sortArrowSize", getHeaderSortArrowSize()));
			setHeaderSortArrowPadding(getThemeValue<f32>(theme, "header.sortArrowPadding", getHeaderSortArrowPadding()));
		}

		f32 DetailList::row_height(void) const
		{
			return (f32)getFontSize() + m_cellPadding.top() + m_cellPadding.bottom();
		}

		f32 DetailList::get_last_column_stretch(void) const
		{
			if (m_columns.empty())
				return 0.0f;
			f32 totalWidth = 0.0f;
			for (auto& c : m_columns)
				totalWidth += c.width;
			const f32 available = getContentBounds().w - getVScrollbarSize();
			return (totalWidth < available) ? (available - totalWidth) : 0.0f;
		}

		void DetailList::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			ensure_sorted();
			draw_rows(gfx);
		}

		void DetailList::afterDraw(ogfx::BasicRenderer2D& gfx)
		{
			// Drawn after the row content and after children, so the header always sits on
			// top and never scrolls, mirroring how Panel layers its titlebar.
			draw_header(gfx);
			drawScrollbars(gfx);
		}

		void DetailList::draw_rows(ogfx::BasicRenderer2D& gfx)
		{
			const auto bounds = getGlobalBounds();
			const f32 rowH = row_height();
			const f32 scrollY = -getScrollOffset().y;
			const f32 visibleH = getContentBounds().h;
			const f32 visibleEnd = scrollY + visibleH;
			const f32 contentW = std::max(getContentExtents().w, getContentBounds().w);
			const f32 lastColumnStretch = get_last_column_stretch();

			i32 startIdx = (rowH > 0) ? (i32)std::max(0.0f, std::floor(scrollY / rowH)) : 0;
			f32 y = (f32)startIdx * rowH;

			for (i32 i = startIdx; i < (i32)m_displayOrder.size(); i++)
			{
				if (y > visibleEnd)
					break;
				Row& row = *m_displayOrder[i];

				Rectangle lineRect {
					Vec2 { bounds.x, bounds.y + m_headerHeight + y } + getScrollOffset(),
					{ contentW, rowH }
				};

				Color textColor;
				if (row.isSelected())
				{
					textColor = getSelectionTextColor();
					gfx.fillRect(lineRect, getSelectionColor());
				}
				else
				{
					Color rowColor = row.getTextColor();
					textColor = (rowColor == Colors::Transparent) ? getTextColor() : rowColor;
					if (isAlternatingBackgroundEnabled())
						gfx.fillRect(lineRect, (i % 2 == 0) ? getBackgroundColor() : getBackgroundColor2());
				}

				f32 cx = lineRect.x;
				for (u32 c = 0; c < m_columns.size(); c++)
				{
					const auto& col = m_columns[c];
					const f32 colWidth = col.width + ((c + 1 == m_columns.size()) ? lastColumnStretch : 0.0f);
					Rectangle cellBounds { cx, lineRect.y, colWidth, rowH };
					draw_cell_text(gfx, row.cellToString(c), cellBounds, col.align, textColor);
					if (isShowColumnSeparatorsEnabled() && c + 1 < m_columns.size())
						gfx.drawLine({ Vec2 { cx + colWidth, lineRect.y }, Vec2 { cx + colWidth, lineRect.y + rowH } }, getSeparatorLineColor(), 1);
					cx += colWidth;
				}

				if (isShowSeparatorLineEnabled())
					gfx.drawLine({ Vec2 { lineRect.x, lineRect.y + rowH }, Vec2 { lineRect.x + lineRect.w, lineRect.y + rowH } }, getSeparatorLineColor(), 1);

				y += rowH;
			}
		}

		void DetailList::draw_cell_text(ogfx::BasicRenderer2D& gfx, const String& text, const Rectangle& cellBounds, eAlign align, const Color& color)
		{
			const f32 padL = m_cellPadding.left();
			const f32 padR = m_cellPadding.right();
			Rectangle padded { cellBounds.x + padL, cellBounds.y, cellBounds.w - (padL + padR), cellBounds.h };
			String clipped = truncate_text(gfx, text, padded.w);
			switch (align)
			{
				case eAlign::Center: gfx.drawCenteredString(clipped, padded, color, getFontSize()); break;
				case eAlign::Right:
				{
					Vec2 dims = gfx.getStringDimensions(clipped, getFontSize());
					Vec2 pos { padded.x + padded.w - dims.x, padded.y + (padded.h - dims.y) * 0.5f };
					gfx.drawString(clipped, pos, color, getFontSize());
					break;
				}
				case eAlign::Left:
				case eAlign::Default:
				default:
					gfx.drawVCenteredString(clipped, padded, color, getFontSize());
					break;
			}
		}

		String DetailList::truncate_text(ogfx::BasicRenderer2D& gfx, const String& text, f32 maxWidth) const
		{
			static const String truncateIndicator = "... ";
			if (maxWidth <= 0.0f)
				return "";

			// getStringDimensionsPerCharacter is cached internally by the renderer, so walking it
			// is cheap and avoids the batch-flushing pushClippingRect/popClippingRect pair we used
			// to wrap each cell in.
			auto perChar = gfx.getStringDimensionsPerCharacter(text, getFontSize());
			if (perChar.empty())
				return text;

			f32 totalWidth = 0.0f;
			for (auto& d : perChar)
				totalWidth += d.x;
			if (totalWidth <= maxWidth)
				return text;

			const f32 indicatorWidth = gfx.getStringDimensions(truncateIndicator, getFontSize()).x;
			const f32 budget = maxWidth - indicatorWidth;
			if (budget <= 0.0f)
				return "";

			u32 charsThatFit = 0;
			f32 acc = 0.0f;
			for (auto& d : perChar)
			{
				acc += d.x;
				if (acc > budget)
					break;
				charsThatFit++;
			}
			if (charsThatFit == 0)
				return "";

			return text.new_fixedLength(charsThatFit + truncateIndicator.len(), ' ', truncateIndicator);
		}

		void DetailList::draw_header(ogfx::BasicRenderer2D& gfx)
		{
			const auto gpos = getGlobalPosition();
			const Rectangle headerBar { gpos, { getw(), m_headerHeight } };
			gfx.outlinedRect(headerBar, m_headerBgColor, m_headerBorderColor, m_headerBorderWidth, false, false, true, false);

			const f32 lastColumnStretch = get_last_column_stretch();
			m_columnHeaderBoundsList.clear();
			f32 x = gpos.x + getScrollOffset().x;
			for (u32 c = 0; c < m_columns.size(); c++)
			{
				auto& col = m_columns[c];
				const f32 colWidth = col.width + ((c + 1 == m_columns.size()) ? lastColumnStretch : 0.0f);
				Rectangle cellBounds { x, gpos.y, colWidth, m_headerHeight };
				m_columnHeaderBoundsList.push_back(cellBounds);

				const bool showArrow = col.sortOrder != eSortOrder::None;
				const f32 padL = m_cellPadding.left();
				const f32 arrowSpace = showArrow ? ((m_headerArrowSize * 2.0f) + m_headerArrowPadding) : 0.0f;
				const f32 reserved = padL + m_cellPadding.right() + arrowSpace;
				Rectangle textBounds { cellBounds.x + padL, cellBounds.y, cellBounds.w - reserved, cellBounds.h };
				String headerText = truncate_text(gfx, col.name, textBounds.w);
				gfx.drawVCenteredString(headerText, textBounds, m_headerTextColor, getFontSize());
				if (showArrow)
					draw_sort_arrow(gfx, cellBounds, col.sortOrder);

				if (isShowColumnSeparatorsEnabled())
					gfx.drawLine({ Vec2 { x + colWidth, gpos.y }, Vec2 { x + colWidth, gpos.y + m_headerHeight } }, m_headerBorderColor, m_headerBorderWidth);

				x += colWidth;
			}
		}

		void DetailList::draw_sort_arrow(ogfx::BasicRenderer2D& gfx, const Rectangle& cellBounds, eSortOrder order)
		{
			const f32 pad  = m_headerArrowPadding;
			const f32 size = m_headerArrowSize;
			const f32 cx = cellBounds.x + cellBounds.w - pad - size;
			const f32 cy = cellBounds.y + cellBounds.h * 0.5f;
			Vec2 p0, p1, p2;
			if (order == eSortOrder::Ascending)
			{
				// Up-pointing: the smallest values are "on top".
				p0 = { cx - size, cy + size * 0.5f };
				p1 = { cx + size, cy + size * 0.5f };
				p2 = { cx,        cy - size * 0.5f };
			}
			else
			{
				// Down-pointing.
				p0 = { cx - size, cy - size * 0.5f };
				p1 = { cx + size, cy - size * 0.5f };
				p2 = { cx,        cy + size * 0.5f };
			}
			gfx.fillTriangle(p0, p1, p2, m_headerArrowColor);
		}

		void DetailList::onMousePressed(const Event& event)
		{
			if (event.mouse->button != ogfx::MouseEventData::eButton::Left)
				return;
			if (isMouseInsideAnyScrollbar())
				return;
			const Rectangle headerBar { getGlobalPosition(), { getw(), m_headerHeight } };
			if (!headerBar.contains({ event.mouse->position_x, event.mouse->position_y }, true))
				return;
			for (u32 i = 0; i < m_columnHeaderBoundsList.size(); i++)
			{
				if (m_columnHeaderBoundsList[i].contains({ event.mouse->position_x, event.mouse->position_y }))
				{
					cycle_sort_column(i);
					event.handle();
					break;
				}
			}
		}

		void DetailList::onMouseReleased(const Event& event)
		{
			if (!isMouseInside())
				return;
			if (event.mouse->button != ogfx::MouseEventData::eButton::Left)
				return;
			if (isMouseInsideAnyScrollbar())
				return;

			ensure_sorted();
			const Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };
			const Vec2 origin = getGlobalBounds().getPosition();
			const Vec2 scroll = getScrollOffset();
			const f32 localY = mousePos.y - origin.y - m_headerHeight - scroll.y;
			if (localY < 0)
				return; // click landed in the header strip, not a row

			const f32 rowH = row_height();
			if (rowH <= 0)
				return;
			i32 idx = (i32)(localY / rowH);
			if (idx < 0 || idx >= (i32)m_displayOrder.size())
				return;

			Row& row = *m_displayOrder[idx];
			bool wasSelected = row.isSelected();
			row.set_selected(m_selectedList);
			if (!wasSelected && callback_onSelectionChanged)
				callback_onSelectionChanged(m_selectedList);
			event.handle();
		}

		void DetailList::cycle_sort_column(u32 columnIndex)
		{
			if (columnIndex >= m_columns.size() || !m_columns[columnIndex].sortable)
				return;

			eSortOrder next;
			switch (m_columns[columnIndex].sortOrder)
			{
				case eSortOrder::None:      next = eSortOrder::Ascending;  break;
				case eSortOrder::Ascending: next = eSortOrder::Descending; break;
				default:                    next = eSortOrder::None;       break;
			}
			// Single-column sort (Explorer semantics): every other column resets to None.
			for (auto& c : m_columns)
				c.sortOrder = eSortOrder::None;
			m_columns[columnIndex].sortOrder = next;
			m_sortColumnIndex = (next == eSortOrder::None) ? -1 : (i32)columnIndex;
			m_sortDirty = true;

			if (callback_onColumnSorted)
				callback_onColumnSorted(*this, columnIndex, next);
		}

		bool DetailList::cell_matches_type(const Cell& value, eColumnType type)
		{
			switch (type)
			{
				case eColumnType::String: return std::holds_alternative<String>(value);
				case eColumnType::I64:    return std::holds_alternative<i64>(value);
				case eColumnType::U64:    return std::holds_alternative<u64>(value);
				case eColumnType::F64:    return std::holds_alternative<f64>(value);
			}
			return false;
		}

		i32 DetailList::compare_cells(const Cell& a, const Cell& b, eColumnType type)
		{
			switch (type)
			{
				case eColumnType::String:
				{
					auto sa = std::get<String>(a).cpp_str();
					auto sb = std::get<String>(b).cpp_str();
					if (sa < sb) return -1;
					if (sb < sa) return 1;
					return 0;
				}
				case eColumnType::I64:
				{
					i64 va = std::get<i64>(a), vb = std::get<i64>(b);
					return (va < vb) ? -1 : (vb < va ? 1 : 0);
				}
				case eColumnType::U64:
				{
					u64 va = std::get<u64>(a), vb = std::get<u64>(b);
					return (va < vb) ? -1 : (vb < va ? 1 : 0);
				}
				case eColumnType::F64:
				{
					f64 va = std::get<f64>(a), vb = std::get<f64>(b);
					return (va < vb) ? -1 : (vb < va ? 1 : 0);
				}
			}
			return 0;
		}

		void DetailList::rebuild_natural_order(void) const
		{
			m_displayOrder.clear();
			m_displayOrder.reserve(m_rows.size());
			for (auto& r : m_rows)
				m_displayOrder.push_back(r.get());
		}

		void DetailList::ensure_sorted(void) const
		{
			if (!m_sortDirty)
				return;
			if (m_sortColumnIndex < 0)
			{
				rebuild_natural_order();
			}
			else
			{
				if (m_displayOrder.size() != m_rows.size())
					rebuild_natural_order();
				const u32 col = (u32)m_sortColumnIndex;
				const eColumnType type = m_columns[col].type;
				const eSortOrder order = m_columns[col].sortOrder;
				std::stable_sort(m_displayOrder.begin(), m_displayOrder.end(),
					[col, type, order](Row* a, Row* b) -> bool {
						i32 c = compare_cells(a->m_cells[col], b->m_cells[col], type);
						return (order == eSortOrder::Ascending) ? (c < 0) : (c > 0);
					});
			}
			m_sortDirty = false;
		}

		Rectangle DetailList::getContentExtents(void) const
		{
			if (!m_extentsDirty)
				return m_cachedExtents;
			f32 totalW = 0;
			for (auto& c : m_columns)
				totalW += c.width;
			f32 totalH = row_height() * (f32)m_rows.size();
			m_cachedExtents = { 0, 0, totalW, totalH };
			m_extentsDirty = false;
			return m_cachedExtents;
		}

		u32 DetailList::addColumn(const String& name, eColumnType type, f32 width, eAlign align)
		{
			if (!m_rows.empty())
			{
				OX_WARN("DetailList: addColumn() called after rows exist; the column schema is fixed once data has been added. Ignoring \"%s\".", name.c_str());
				return (u32)m_columns.size();
			}
			Column c;
			c.name = name;
			c.type = type;
			c.width = std::max(width, 24.0f);
			c.align = (align == eAlign::Default) ? ((type == eColumnType::String) ? eAlign::Left : eAlign::Right) : align;
			m_columns.push_back(c);
			m_extentsDirty = true;
			return (u32)m_columns.size() - 1;
		}

		u32 DetailList::addColumn(const String& name, eColumnType type, f32 width, u8 decimalPrecision, eAlign align)
		{
			u32 index = addColumn(name, type, width, align);
			if (index < m_columns.size())
				m_columns[index].precision = decimalPrecision;
			return index;
		}

		DetailList::Row& DetailList::addRow(const stdvec<Cell>& values, const Color& textColor)
		{
			if (values.size() != m_columns.size())
			{
				OX_WARN("DetailList: addRow() value count (%d) does not match column count (%d).", (i32)values.size(), (i32)m_columns.size());
				return m_invalidRow;
			}
			for (u32 i = 0; i < values.size(); i++)
			{
				if (!cell_matches_type(values[i], m_columns[i].type))
				{
					OX_WARN("DetailList: addRow() type mismatch on column %d (\"%s\").", (i32)i, m_columns[i].name.c_str());
					return m_invalidRow;
				}
			}
			auto row = std::make_unique<Row>(*this);
			row->m_cells = values;
			row->m_textColor = textColor;
			Row* ptr = row.get();
			m_rows.push_back(std::move(row));
			m_displayOrder.push_back(ptr);
			m_sortDirty = true;
			m_extentsDirty = true;
			return *ptr;
		}

		bool DetailList::removeRow(Row& row)
		{
			auto it = std::find_if(m_rows.begin(), m_rows.end(), [&](const std::unique_ptr<Row>& r) { return r.get() == &row; });
			if (it == m_rows.end())
				return false;
			STDVEC_REMOVE(m_selectedList, &row);
			STDVEC_REMOVE(m_displayOrder, &row);
			m_rows.erase(it);
			m_extentsDirty = true;
			return true;
		}

		bool DetailList::removeRow(u32 displayIndex)
		{
			ensure_sorted();
			if (displayIndex >= m_displayOrder.size())
				return false;
			return removeRow(*m_displayOrder[displayIndex]);
		}

		void DetailList::clearRows(void)
		{
			m_rows.clear();
			m_displayOrder.clear();
			m_selectedList.clear();
			m_extentsDirty = true;
			m_sortDirty = false;
		}

		DetailList::Row& DetailList::getRow(u32 displayIndex)
		{
			ensure_sorted();
			if (displayIndex < m_displayOrder.size())
				return *m_displayOrder[displayIndex];
			return m_invalidRow;
		}
		// ===================================================================
	}
}
