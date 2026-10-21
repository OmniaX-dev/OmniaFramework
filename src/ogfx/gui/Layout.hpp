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

#include <ostd/math/Geometry.hpp>
#include <ostd/utils/Defines.hpp>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// Anchor-based layout (modelled after Godot's Control layout).
		//
		// Every widget stores four ANCHORS and four OFFSETS, one per edge:
		//
		//   anchors: fractions (0.0 .. 1.0) of the parent's content area.
		//            0.0 = the parent's left/top edge, 1.0 = its right/bottom edge,
		//            0.5 = its center. Think of them as percentages (0.25 = 25%).
		//   offsets: pixel distances added to the anchor positions.
		//
		// The widget's rectangle (local to the parent's content frame) is:
		//
		//   left   = parentW * anchors.left   + offsets.left
		//   top    = parentH * anchors.top    + offsets.top
		//   right  = parentW * anchors.right  + offsets.right
		//   bottom = parentH * anchors.bottom + offsets.bottom
		//
		// Whenever the parent is resized, the rectangle is recomputed from these
		// values. Some examples:
		//
		//   - anchors { 0, 0, 0, 0 }:  fixed position and size (the default; a widget
		//                              behaves exactly like with plain setPosition/setSize).
		//   - anchors { 0, 0, 1, 1 }, offsets { 10, 10, -10, -10 }:
		//                              fills the parent with a 10px margin.
		//   - anchors { 1, 1, 1, 1 }, offsets { -110, -40, -10, -10 }:
		//                              100x30 box glued to the bottom-right corner.
		//   - anchors { 0.25, 0, 0.75, 1 }:
		//                              always covers the middle 50% of the parent's width.
		//
		// setPosition()/setSize() keep working: they update the offsets so that the
		// widget ends up at the requested rectangle with the current anchors.
		//
		// When the computed size is smaller than the widget's minimum size, the
		// widget grows according to its grow direction (End = towards right/bottom,
		// Begin = towards left/top, Both = around its center).
		//
		// Widgets whose parent is a Container (BoxContainer, GridContainer, ...)
		// ignore anchors entirely: the container decides their rectangle, using
		// the child's minimum size, size flags and stretch ratio.
		// ============================================================================

		// One value per edge. Used for both anchors (fractions) and offsets (pixels).
		struct EdgeValues
		{
			f32 left   { 0.0f };
			f32 top    { 0.0f };
			f32 right  { 0.0f };
			f32 bottom { 0.0f };

			inline EdgeValues(void) = default;
			inline EdgeValues(f32 l, f32 t, f32 r, f32 b) : left(l), top(t), right(r), bottom(b) {  }
			inline bool operator==(const EdgeValues& o) const { return left == o.left && top == o.top && right == o.right && bottom == o.bottom; }
			inline bool operator!=(const EdgeValues& o) const { return !(*this == o); }
			inline bool isZero(void) const { return left == 0.0f && top == 0.0f && right == 0.0f && bottom == 0.0f; }
		};
		using Anchors = EdgeValues;
		using Offsets = EdgeValues;

		enum class eSide : u8
		{
			Left = 0,
			Top,
			Right,
			Bottom
		};

		// Common anchor configurations. "Wide" presets stretch along one axis.
		enum class eAnchorPreset : u8
		{
			TopLeft = 0,
			TopRight,
			BottomLeft,
			BottomRight,
			CenterLeft,
			CenterTop,
			CenterRight,
			CenterBottom,
			Center,
			LeftWide,      // left edge, full height
			TopWide,       // top edge, full width
			RightWide,     // right edge, full height
			BottomWide,    // bottom edge, full width
			VCenterWide,   // vertical band through the center, full height
			HCenterWide,   // horizontal band through the center, full width
			FullRect       // fill the parent
		};

		// Which size to use when a preset computes offsets for a non-stretching axis.
		enum class ePresetResizeMode : u8
		{
			KeepSize = 0,  // keep the widget's current width and height
			MinSize,       // shrink to the widget's minimum size
			KeepWidth,     // keep width, use minimum height
			KeepHeight     // keep height, use minimum width
		};

		// Direction a widget grows in when its anchored rectangle is smaller than
		// its minimum size (or when an auto-sizing widget changes its own size).
		enum class eGrowDirection : u8
		{
			Begin = 0,  // towards left / top
			End,        // towards right / bottom
			Both        // symmetrically around the center
		};

		// How a child of a Container uses the space the container gives it.
		// Flags can be combined with '|' (e.g. Fill | Expand).
		enum class eSizeFlags : u8
		{
			ShrinkBegin  = 0,       // use minimum size, aligned to the start of the slot
			Fill         = 1 << 0,  // fill the whole slot
			Expand       = 1 << 1,  // ask for a share of the container's leftover space
			ShrinkCenter = 1 << 2,  // use minimum size, centered in the slot
			ShrinkEnd    = 1 << 3,  // use minimum size, aligned to the end of the slot

			ExpandFill   = Fill | Expand
		};
		inline constexpr eSizeFlags operator|(eSizeFlags a, eSizeFlags b) { return cast<eSizeFlags>(cast<u8>(a) | cast<u8>(b)); }
		inline constexpr eSizeFlags operator&(eSizeFlags a, eSizeFlags b) { return cast<eSizeFlags>(cast<u8>(a) & cast<u8>(b)); }
		inline constexpr bool hasSizeFlag(eSizeFlags flags, eSizeFlags flag) { return (cast<u8>(flags) & cast<u8>(flag)) != 0; }

		// Alignment of the whole group of children inside a BoxContainer when no
		// child expands.
		enum class eBoxAlignment : u8
		{
			Begin = 0,
			Center,
			End
		};

		enum class eOrientation : u8
		{
			Horizontal = 0,
			Vertical
		};

		// All per-widget layout state. Owned by Widget, see Widget's LAYOUT section.
		struct LayoutData
		{
			Anchors anchors { 0, 0, 0, 0 };
			Offsets offsets { 0, 0, 0, 0 };

			// User-defined minimum size; the effective minimum is the max of this and
			// the widget's own content minimum (see Widget::getCombinedMinSize()).
			Vec2 minSize { 0, 0 };

			eGrowDirection growH { eGrowDirection::End };
			eGrowDirection growV { eGrowDirection::End };

			// Only used when the parent is a Container.
			eSizeFlags sizeFlagsH { eSizeFlags::Fill };
			eSizeFlags sizeFlagsV { eSizeFlags::Fill };
			f32 stretchRatio { 1.0f };
		};
	}
}
