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

#include <ostd/math/Geometry.hpp>
#include <ostd/utils/Defines.hpp>

namespace ogfx
{
	namespace gui
	{
		// How a child wants to align itself within the slot a Layout gives it.
		enum class eAlign : u8
		{
			Start,    // top / left
			Center,   // center along the relevant axis
			End,      // bottom / right
			Stretch   // fill the available space along that axis
		};

		// Per-child metadata read by the parent's Layout when it arranges children.
		// All fields have sane defaults so unset hints behave like "use my current size,
		// no stretching, top-left, span 1".
		struct LayoutHint
		{
			// 0    = use the widget's current/preferred size on the main axis.
			// > 0  = take a share of leftover space proportional to this value.
			//        e.g. two children with stretch=1.0 split leftover space 50/50;
			//        stretch=2 vs stretch=1 splits it 2/3 vs 1/3.
			f32 stretch { 0.0f };

			// Alignment along the layout's main axis (only meaningful when the child
			// is smaller than the slot the layout assigned it — i.e. when stretch=0
			// and the slot ended up larger than preferred). Mostly relevant for Grid
			// and for FillLayout in single-child mode.
			eAlign alignMain  { eAlign::Start };

			// Alignment perpendicular to the main axis. Stretch (default) makes the
			// child fill the cross dimension of its slot — usually what you want.
			eAlign alignCross { eAlign::Stretch };

			// Hard limits the layout will respect. minSize is honored even if it
			// breaks stretch ratios; maxSize component < 0 means "unbounded".
			Vec2 minSize { 0, 0 };
			Vec2 maxSize { -1, -1 };

			// If a component is < 0, the layout uses the widget's current size
			// (getw()/geth()) for that axis. Otherwise this overrides it.
			Vec2 preferred { -1, -1 };

			// Grid-only. How many cells the child occupies. Ignored by Box/Fill.
			u8 colSpan { 1 };
			u8 rowSpan { 1 };

			// If true, the layout skips this child entirely. Useful for floating
			// overlays, drag-ghosts, or scrollbars (which manage their own bounds).
			// Note: ScrollableWidget already excludes its scrollbars via the
			// IgnoreScroll flag, but this gives finer control for other cases.
			bool ignored { false };
		};
	}
}
