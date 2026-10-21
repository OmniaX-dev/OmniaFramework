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

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// Container — base class for widgets that position their children
		// automatically (Godot-style). Children of a Container ignore their own
		// anchors; instead the container gives each child a slot and the child's
		// size flags decide how it uses it:
		//
		//   Fill          -> take the whole slot (default)
		//   ShrinkBegin / ShrinkCenter / ShrinkEnd
		//                 -> use the child's minimum size, aligned inside the slot
		//   Expand        -> (Box/Grid) ask for a share of the leftover space,
		//                    proportional to the child's stretch ratio
		//
		// A container's own minimum size is computed from its children, so nested
		// containers (and anchored containers inside a Panel) size themselves
		// correctly. The container itself is placed like any other widget: with
		// anchors, by its parent container, or manually.
		//
		// Invisible children and children with IgnoreScroll enabled are skipped.
		// ============================================================================
		class Container : public Widget
		{
			public:
				Container(Window& window);

				inline bool managesChildLayout(void) const override { return true; }
				void layoutChildren(void) override;
				Vec2 getContentMinSize(void) const override;

			protected:
				// area: size of the content area; slots are local to it (0, 0 = top-left).
				virtual void arrangeChildren(const Vec2& area) = 0;
				// Minimum size of the content area needed by the children.
				virtual Vec2 measureContent(void) const = 0;

				// Places child inside rect, honoring its size flags.
				void fitChildInRect(Widget& child, const Rectangle& rect) const;
				stdvec<Widget*> getLayoutChildren(void) const;
				// Re-arranges and propagates the (possibly) new minimum size.
				void containerPropertyChanged(void);

			private:
				bool m_arranging { false };
				bool m_arrangeRequested { false };
		};


		// ============================================================================
		// BoxContainer — stacks children in a row (Horizontal) or column (Vertical).
		//
		// Every child gets at least its minimum size along the main axis. Leftover
		// space goes to the children with the Expand flag, proportionally to their
		// stretch ratio. If no child expands, the whole group is aligned according
		// to the alignment (Begin/Center/End). On the cross axis each child gets
		// the full thickness of the box (and then its size flags apply).
		// ============================================================================
		class BoxContainer : public Container
		{
			public:
				BoxContainer(Window& window, eOrientation orientation = eOrientation::Horizontal);

				void setOrientation(eOrientation orientation);
				inline eOrientation getOrientation(void) const { return m_orientation; }
				inline bool isVertical(void) const { return m_orientation == eOrientation::Vertical; }

				void setSpacing(f32 spacing);
				inline f32 getSpacing(void) const { return m_spacing; }

				void setAlignment(eBoxAlignment alignment);
				inline eBoxAlignment getAlignment(void) const { return m_alignment; }

			protected:
				void arrangeChildren(const Vec2& area) override;
				Vec2 measureContent(void) const override;

			private:
				eOrientation m_orientation;
				f32 m_spacing { 4.0f };
				eBoxAlignment m_alignment { eBoxAlignment::Begin };
		};

		class HBoxContainer : public BoxContainer
		{
			public:
				inline HBoxContainer(Window& window) : BoxContainer(window, eOrientation::Horizontal) { setTypeName("ogfx::gui::HBoxContainer"); }
		};

		class VBoxContainer : public BoxContainer
		{
			public:
				inline VBoxContainer(Window& window) : BoxContainer(window, eOrientation::Vertical) { setTypeName("ogfx::gui::VBoxContainer"); }
		};


		// ============================================================================
		// GridContainer — arranges children in a grid with a fixed number of
		// columns, filling rows left to right in insertion order.
		//
		// Each column is as wide as its widest child (minimum size), each row as
		// tall as its tallest child. A column expands if any of its children has
		// the horizontal Expand flag (rows likewise with the vertical flag); the
		// leftover space is shared equally among expanding columns/rows.
		// ============================================================================
		class GridContainer : public Container
		{
			public:
				GridContainer(Window& window, u32 columns = 2);

				void setColumns(u32 columns);
				inline u32 getColumns(void) const { return m_columns; }

				void setSpacing(f32 horizontal, f32 vertical);
				inline void setSpacing(f32 spacing) { setSpacing(spacing, spacing); }
				inline f32 getHSpacing(void) const { return m_hSpacing; }
				inline f32 getVSpacing(void) const { return m_vSpacing; }

			protected:
				void arrangeChildren(const Vec2& area) override;
				Vec2 measureContent(void) const override;

			private:
				void compute_mins(const stdvec<Widget*>& children, u32 cols, stdvec<f32>& colMin, stdvec<f32>& rowMin, stdvec<bool>& colExpand, stdvec<bool>& rowExpand) const;

			private:
				u32 m_columns;
				f32 m_hSpacing { 4.0f };
				f32 m_vSpacing { 4.0f };
		};


		// ============================================================================
		// MarginContainer — every child fills the container minus the margins.
		// Children overlap; usually it holds a single child (often another
		// container) to give it some breathing room.
		// ============================================================================
		class MarginContainer : public Container
		{
			public:
				MarginContainer(Window& window, const EdgeValues& margins = { 0, 0, 0, 0 });

				void setMargins(const EdgeValues& margins);
				inline void setMargins(f32 l, f32 t, f32 r, f32 b) { setMargins(EdgeValues { l, t, r, b }); }
				inline void setMargins(f32 all) { setMargins(EdgeValues { all, all, all, all }); }
				inline const EdgeValues& getMargins(void) const { return m_margins; }

			protected:
				void arrangeChildren(const Vec2& area) override;
				Vec2 measureContent(void) const override;

			private:
				EdgeValues m_margins;
		};


		// ============================================================================
		// CenterContainer — keeps every child at its minimum size, centered.
		// ============================================================================
		class CenterContainer : public Container
		{
			public:
				CenterContainer(Window& window);

			protected:
				void arrangeChildren(const Vec2& area) override;
				Vec2 measureContent(void) const override;
		};
	}
}
