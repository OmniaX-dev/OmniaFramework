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

#pragma once

#include <ogfx/gui/LayoutHint.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/utils/Defines.hpp>

namespace ogfx
{
	namespace gui
	{
		class Widget;

		// ============================================================================
		// Layout — abstract base.
		//
		// A Layout is owned by a Widget (the "container") and is responsible for
		// positioning and sizing that widget's children inside the container's
		// content bounds (i.e. inside padding, after content offset).
		//
		// Two responsibilities:
		//   - arrange(parent): set position + size on every laid-out child.
		//   - measure(parent): report how much space this layout *wants* given
		//                      its current children. Used so a Layout can drive
		//                      a parent's contentExtents (and therefore scrollbars),
		//                      and so a parent Layout can ask a child container
		//                      "how big would you like to be?".
		// ============================================================================
		class Layout
		{
			public:
				virtual ~Layout(void) = default;

				// Place children. Called from Widget::relayout().
				virtual void arrange(Widget& parent) = 0;

				// Compute desired size of this layout's content. Should NOT mutate
				// children. Used by Widget::getContentExtents() override below and
				// by parent layouts asking for preferred size.
				virtual Vec2 measure(const Widget& parent) const = 0;

				// Spacing between adjacent items along the main axis (Box) or
				// between cells (Grid). FillLayout ignores it.
				inline f32 getSpacing(void) const { return m_spacing; }
				inline void setSpacing(f32 s) { m_spacing = s; }

			protected:
				// Helpers used by concrete layouts to honor a child's hint
				// without each subclass re-implementing the same min/max/preferred
				// resolution logic.
				static Vec2 resolvePreferred(const Widget& child);
				static f32  applyMinMax(f32 value, f32 minV, f32 maxV);
				static f32  alignOffset(eAlign a, f32 available, f32 size);

				f32 m_spacing { 0.0f };
		};


		// ============================================================================
		// FillLayout — every laid-out child is sized to fill the parent's content
		// area. Useful as a default (a "single slot" container) and as a building
		// block for things like SplitView panels.
		// ============================================================================
		class FillLayout : public Layout
		{
			public:
				void arrange(Widget& parent) override;
				Vec2 measure(const Widget& parent) const override;
		};


		// ============================================================================
		// BoxLayout — arrange children in a single row (Horizontal) or column
		// (Vertical) with stretch factors and spacing.
		//
		// Algorithm (per axis = "main"):
		//   1. Sum preferred sizes of non-stretching children + spacing.
		//   2. The remainder is distributed among stretching children proportional
		//      to their stretch factor.
		//   3. min/max are clamped per child; if a stretching child hits its max,
		//      the leftover is redistributed among the rest in a second pass.
		//
		// Cross axis: each child either fills (Stretch, default) or uses its
		// preferred size aligned per alignCross.
		// ============================================================================
		class BoxLayout : public Layout
		{
			public:
				enum class Orientation { Horizontal, Vertical };

				explicit BoxLayout(Orientation orientation) : m_orientation(orientation) {}

				void arrange(Widget& parent) override;
				Vec2 measure(const Widget& parent) const override;

				inline Orientation getOrientation(void) const { return m_orientation; }
				inline void setOrientation(Orientation o) { m_orientation = o; }

			private:
				Orientation m_orientation;
		};


		// ============================================================================
		// GridLayout — fixed rows × cols. Cell sizes are uniform (rows = h/N,
		// cols = w/M). Children are placed in row-major insertion order.
		// LayoutHint::colSpan / rowSpan let a child cover multiple cells.
		// ============================================================================
		class GridLayout : public Layout
		{
			public:
				GridLayout(u32 rows, u32 cols) : m_rows(rows), m_cols(cols) {}

				void arrange(Widget& parent) override;
				Vec2 measure(const Widget& parent) const override;

				inline u32 getRows(void) const { return m_rows; }
				inline u32 getCols(void) const { return m_cols; }
				inline void setDimensions(u32 rows, u32 cols) { m_rows = rows; m_cols = cols; }

			private:
				u32 m_rows;
				u32 m_cols;
		};
	}
}
