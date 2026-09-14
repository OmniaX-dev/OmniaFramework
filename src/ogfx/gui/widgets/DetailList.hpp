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

#pragma once

#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/widgets/Scrollbar.hpp>
#include <deque>
#include <memory>
#include <variant>
#include <functional>

namespace ogfx
{
	namespace gui
	{
		// A Windows-Explorer-"Details view"-style list: a scrollable row area with a fixed,
		// non-scrolling header row whose columns can be clicked to cycle sort direction.
		class DetailList : public ScrollableWidget
		{
			public: enum class eColumnType : u8 { String = 0, I64, U64, F64 };
			public: enum class eSortOrder  : u8 { None = 0, Ascending, Descending };
			// Default resolves to Left for String columns, Right for numeric ones (addColumn()).
			public: enum class eAlign      : u8 { Left = 0, Center, Right, Default };

			// A cell can only ever hold the type declared for its column. Note: since this is a
			// std::variant of multiple numeric alternatives, a bare numeric literal (e.g. 30) is
			// ambiguous - pass exactly-typed values, e.g. (i64)30, (u64)5, 3.14 (matches f64).
			public: using Cell = std::variant<String, i64, u64, f64>;

			public: struct Column
			{
				String      name      { "" };
				eColumnType type      { eColumnType::String };
				f32         width     { 120 };
				eAlign      align     { eAlign::Left };
				u8          precision { 2 };     // only used when type == F64
				eSortOrder  sortOrder { eSortOrder::None };
				bool        sortable  { true };
			};

			public: class Row
			{
				public:
					inline Row(DetailList& owner) : m_owner(&owner) {  }
					inline bool isValid(void) const { return m_owner != nullptr && !m_cells.empty(); }
					inline bool isSelected(void) const { return m_selected; }
					// Colors::Transparent (the default) means "use the theme's textColor for this row".
					inline Color getTextColor(void) const { return m_textColor; }
					inline void setTextColor(const Color& color) { m_textColor = color; }
					Cell getCell(u32 col) const;
					String getString(u32 col) const;
					i64 getI64(u32 col) const;
					u64 getU64(u32 col) const;
					f64 getF64(u32 col) const;
					bool setCell(u32 col, const Cell& value);
					String cellToString(u32 col) const;

				private:
					void set_selected(stdvec<Row*>& selectionList);

				private:
					DetailList* m_owner { nullptr };
					stdvec<Cell> m_cells;
					bool m_selected { false };
					Color m_textColor { Colors::Transparent };

					friend class DetailList;
			};

			public:
				using SelectionChangedCallback = std::function<void(stdvec<Row*>& selection)>;
				using ColumnSortedCallback     = std::function<void(DetailList& sender, u32 columnIndex, eSortOrder order)>;

				inline DetailList(Window& window) : ScrollableWidget(window) { create(); }
				DetailList& create(void);
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void afterDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMousePressed(const Event& event) override;
				void onMouseReleased(const Event& event) override;
				Rectangle getContentExtents(void) const override;

				u32 addColumn(const String& name, eColumnType type, f32 width = 120, eAlign align = eAlign::Default);
				u32 addColumn(const String& name, eColumnType type, f32 width, u8 decimalPrecision, eAlign align = eAlign::Default);
				inline u32 getColumnCount(void) const { return (u32)m_columns.size(); }
				inline Column& getColumn(u32 index) { return m_columns[index]; }
				inline const Column& getColumn(u32 index) const { return m_columns[index]; }

				Row& addRow(const stdvec<Cell>& values, const Color& textColor = Colors::Transparent);
				bool removeRow(Row& row);
				bool removeRow(u32 displayIndex);
				void clearRows(void);
				inline u32 getRowCount(void) const { return (u32)m_rows.size(); }
				Row& getRow(u32 displayIndex);

				inline stdvec<Row*>& getSelection(void) { return m_selectedList; }
				inline i32 getSortColumnIndex(void) const { return m_sortColumnIndex; }
				inline void setSelectionChangedCallback(SelectionChangedCallback cb) { callback_onSelectionChanged = std::move(cb); }
				inline void setColumnSortedCallback(ColumnSortedCallback cb) { callback_onColumnSorted = std::move(cb); }

				void setHeaderHeight(f32 height);
				inline f32 getHeaderHeight(void) const { return m_headerHeight; }

				OSTD_PARAM_GETSET(Color, BackgroundColor2, m_backgroundColor2);
				OSTD_BOOL_PARAM_GETSET_E(AlternatingBackground, m_showAlternatingBackground);
				OSTD_PARAM_GETSET(Color, SeparatorLineColor, m_lineColor);
				OSTD_BOOL_PARAM_GETSET_E(ShowSeparatorLine, m_showLine);
				OSTD_BOOL_PARAM_GETSET_E(ShowColumnSeparators, m_showColumnSeparators);
				OSTD_PARAM_GETSET(Rectangle, CellPadding, m_cellPadding);
				OSTD_PARAM_GETSET(Color, SelectionColor, m_selectionColor);
				OSTD_PARAM_GETSET(Color, SelectionTextColor, m_selectionTextColor);

				OSTD_PARAM_GETSET(Color, HeaderBackgroundColor, m_headerBgColor);
				OSTD_PARAM_GETSET(Color, HeaderTextColor, m_headerTextColor);
				OSTD_PARAM_GETSET(Color, HeaderBorderColor, m_headerBorderColor);
				OSTD_PARAM_GETSET(i32, HeaderBorderWidth, m_headerBorderWidth);
				OSTD_PARAM_GETSET(Color, HeaderSortArrowColor, m_headerArrowColor);
				OSTD_PARAM_GETSET(f32, HeaderSortArrowSize, m_headerArrowSize);
				OSTD_PARAM_GETSET(f32, HeaderSortArrowPadding, m_headerArrowPadding);

			private:
				static bool cell_matches_type(const Cell& value, eColumnType type);
				static i32 compare_cells(const Cell& a, const Cell& b, eColumnType type);
				void ensure_sorted(void) const;
				void rebuild_natural_order(void) const;
				void cycle_sort_column(u32 columnIndex);
				void draw_header(ogfx::BasicRenderer2D& gfx);
				void draw_rows(ogfx::BasicRenderer2D& gfx);
				void draw_sort_arrow(ogfx::BasicRenderer2D& gfx, const Rectangle& cellBounds, eSortOrder order);
				void draw_cell_text(ogfx::BasicRenderer2D& gfx, const String& text, const Rectangle& cellBounds, eAlign align, const Color& color);
				String truncate_text(ogfx::BasicRenderer2D& gfx, const String& text, f32 maxWidth) const;
				f32 row_height(void) const;
				f32 get_last_column_stretch(void) const;

			private:
				Row m_invalidRow { *this };
				stdvec<Column> m_columns;
				std::deque<std::unique_ptr<Row>> m_rows;         // insertion order, stable Row identity
				mutable stdvec<Row*> m_displayOrder;              // current (possibly sorted) view
				mutable bool m_sortDirty { true };
				i32 m_sortColumnIndex { -1 };

				stdvec<Row*> m_selectedList;
				SelectionChangedCallback callback_onSelectionChanged { nullptr };
				ColumnSortedCallback     callback_onColumnSorted { nullptr };

				mutable stdvec<Rectangle> m_columnHeaderBoundsList;
				mutable Rectangle m_cachedExtents { 0, 0, 0, 0 };
				mutable bool m_extentsDirty { true };

				f32 m_headerHeight { 28 };
				Color m_headerBgColor { 60, 60, 60 };
				Color m_headerTextColor { Colors::White };
				Color m_headerBorderColor { 30, 30, 30 };
				i32 m_headerBorderWidth { 1 };
				Color m_headerArrowColor { Colors::White };
				f32 m_headerArrowSize { 5 };
				f32 m_headerArrowPadding { 8 };

				Color m_backgroundColor2 { Colors::Black };
				bool m_showAlternatingBackground { true };
				Color m_lineColor { 40, 40, 40 };
				bool m_showLine { true };
				bool m_showColumnSeparators { true };
				Rectangle m_cellPadding { 6, 4, 6, 4 };
				Color m_selectionColor { Colors::Crimson };
				Color m_selectionTextColor { Colors::White };
		};
	}
}
