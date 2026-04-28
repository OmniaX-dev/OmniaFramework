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

#include "Layout.hpp"
#include "widgets/Widget.hpp"

#include <algorithm>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// Layout — shared helpers
		// ============================================================================

		Vec2 Layout::resolvePreferred(const Widget& child)
		{
			const auto& h = child.layoutHint();
			Vec2 pref;
			pref.x = (h.preferred.x < 0) ? child.getw() : h.preferred.x;
			pref.y = (h.preferred.y < 0) ? child.geth() : h.preferred.y;
			return pref;
		}

		f32 Layout::applyMinMax(f32 value, f32 minV, f32 maxV)
		{
			if (value < minV) value = minV;
			if (maxV >= 0 && value > maxV) value = maxV;
			return value;
		}

		f32 Layout::alignOffset(eAlign a, f32 available, f32 size)
		{
			switch (a)
			{
				case eAlign::Center:  return (available - size) * 0.5f;
				case eAlign::End:     return  available - size;
				case eAlign::Start:
				case eAlign::Stretch: // when stretched the caller already used `available`
				default:              return 0.0f;
			}
		}

		// Iterate the children we actually care about: visible, valid, not ignored,
		// not "ignore-scroll" (scrollbars and friends), and not manually drawn.
		// This is a small inline helper to avoid repeating the predicate across
		// every layout subclass below.
		namespace
		{
			template<typename F>
			void for_each_laid_out(const Widget& parent, F&& fn)
			{
				auto& mgr = const_cast<Widget&>(parent).getChildren();
				for (auto* c : mgr.getWidgets())
				{
					if (!c) continue;
					if (c->isInvalid()) continue;
					if (!c->isVisible()) continue;
					if (c->isIgnoreScrollEnabled()) continue;  // scrollbars etc.
					if (c->layoutHint().ignored) continue;
					fn(*c);
				}
			}
		}


		// ============================================================================
		// FillLayout
		// ============================================================================

		void FillLayout::arrange(Widget& parent)
		{
			// IMPORTANT: child positions are LOCAL to the parent's content frame.
			// getGlobalPosition() automatically adds the parent's padding and
			// contentOffset, so we must NOT include them here or they'd be
			// applied twice. We only use the SIZE from getContentBounds().
			const Vec2 size = parent.getContentBounds().getSize();

			for_each_laid_out(parent, [&](Widget& c) {
				c.setPosition({ 0, 0 });
				c.setSize(size);
				c.onBoundsChanged(size);
			});
		}

		Vec2 FillLayout::measure(const Widget& parent) const
		{
			// Fill wants whatever the parent gives it; report the largest child
			// preferred size as a hint for nested measure passes.
			Vec2 result { 0, 0 };
			for_each_laid_out(parent, [&](Widget& c) {
				Vec2 pref = resolvePreferred(c);
				result.x = std::max(result.x, pref.x);
				result.y = std::max(result.y, pref.y);
			});
			return result;
		}


		// ============================================================================
		// BoxLayout
		// ============================================================================

		void BoxLayout::arrange(Widget& parent)
		{
			const bool horiz   = (m_orientation == Orientation::Horizontal);
			// IMPORTANT: child positions are LOCAL to the parent's content frame.
			// getGlobalPosition() auto-adds parent's padding and contentOffset,
			// so we use only the SIZE from getContentBounds() and start child
			// positions at (0, 0). Adding area.x/area.y here would double-count.
			const Vec2 areaSize = parent.getContentBounds().getSize();
			const f32  totalMain  = horiz ? areaSize.x : areaSize.y;
			const f32  totalCross = horiz ? areaSize.y : areaSize.x;

			// --- Pass 1: gather children, classify as "fixed" vs "stretching",
			//     and accumulate fixed-size + total stretch weight.
			struct Slot { Widget* w; f32 main; f32 cross; bool stretches; f32 stretch;
						  f32 mainMin, mainMax; f32 crossMin, crossMax; };
			stdvec<Slot> slots;
			slots.reserve(8);

			f32 fixedMain    = 0.0f;
			f32 stretchTotal = 0.0f;

			for_each_laid_out(parent, [&](Widget& c) {
				const auto& h = c.layoutHint();
				const Vec2 pref = resolvePreferred(c);

				Slot s;
				s.w         = &c;
				s.main      = horiz ? pref.x : pref.y;
				s.cross     = horiz ? pref.y : pref.x;
				s.stretches = (h.stretch > 0.0f);
				s.stretch   = h.stretch;
				s.mainMin   = horiz ? h.minSize.x : h.minSize.y;
				s.mainMax   = horiz ? h.maxSize.x : h.maxSize.y;
				s.crossMin  = horiz ? h.minSize.y : h.minSize.x;
				s.crossMax  = horiz ? h.maxSize.y : h.maxSize.x;

				if (!s.stretches)
				{
					s.main = applyMinMax(s.main, s.mainMin, s.mainMax);
					fixedMain += s.main;
				}
				else
				{
					stretchTotal += s.stretch;
				}
				slots.push_back(s);
			});

			if (slots.empty())
				return;

			const f32 spacingTotal = m_spacing * std::max<f32>(0, (f32)slots.size() - 1);
			f32 leftover = std::max(0.0f, totalMain - fixedMain - spacingTotal);

			// --- Pass 2: distribute leftover among stretching children.
			// We may need multiple rounds: if a stretcher hits its maxSize cap,
			// we lock it and redistribute the residual among the rest.
			stdvec<bool> locked(slots.size(), false);
			for (size_t i = 0; i < slots.size(); ++i)
				if (!slots[i].stretches) locked[i] = true;

			while (stretchTotal > 0.0f && leftover > 0.0f)
			{
				bool changed = false;
				f32 newLeftover = leftover;
				f32 newStretchTotal = stretchTotal;

				for (size_t i = 0; i < slots.size(); ++i)
				{
					if (locked[i]) continue;
					f32 share = leftover * (slots[i].stretch / stretchTotal);
					f32 capped = applyMinMax(share, slots[i].mainMin, slots[i].mainMax);
					if (capped < share)  // hit the cap
					{
						slots[i].main = capped;
						newLeftover     -= capped;
						newStretchTotal -= slots[i].stretch;
						locked[i] = true;
						changed = true;
					}
					else
					{
						slots[i].main = share;  // tentative; may be re-revised next round
					}
				}

				if (!changed) break;             // no caps hit -> stable
				leftover     = newLeftover;
				stretchTotal = newStretchTotal;
			}

			// --- Pass 3: place children.
			f32 cursor = 0.0f;  // local to parent's content frame
			for (auto& s : slots)
			{
				const auto& h = s.w->layoutHint();

				// Cross size: stretch fills, otherwise use child preferred (clamped).
				f32 crossSize;
				if (h.alignCross == eAlign::Stretch)
					crossSize = totalCross;
				else
					crossSize = applyMinMax(s.cross, s.crossMin, s.crossMax);
				crossSize = std::min(crossSize, totalCross);

				const f32 crossOff = alignOffset(h.alignCross, totalCross, crossSize);

				if (horiz)
				{
					s.w->setPosition(cursor, crossOff);
					s.w->setSize(s.main, crossSize);
				}
				else
				{
					s.w->setPosition(crossOff, cursor);
					s.w->setSize(crossSize, s.main);
				}
				s.w->onBoundsChanged(s.w->getSize());

				cursor += s.main + m_spacing;
			}
		}

		Vec2 BoxLayout::measure(const Widget& parent) const
		{
			const bool horiz = (m_orientation == Orientation::Horizontal);
			f32 mainSum = 0, crossMax = 0;
			i32 count = 0;

			for_each_laid_out(parent, [&](Widget& c) {
				Vec2 pref = resolvePreferred(c);
				f32 m  = horiz ? pref.x : pref.y;
				f32 cr = horiz ? pref.y : pref.x;
				mainSum += m;
				crossMax = std::max(crossMax, cr);
				++count;
			});
			mainSum += m_spacing * std::max(0, count - 1);
			return horiz ? Vec2 { mainSum, crossMax } : Vec2 { crossMax, mainSum };
		}


		// ============================================================================
		// GridLayout
		// ============================================================================

		void GridLayout::arrange(Widget& parent)
		{
			if (m_rows == 0 || m_cols == 0) return;

			// IMPORTANT: child positions are LOCAL to the parent's content frame.
			// getGlobalPosition() auto-adds parent's padding and contentOffset,
			// so we use only the SIZE from getContentBounds() here.
			const Vec2 areaSize = parent.getContentBounds().getSize();
			const f32 cellW = (areaSize.x - m_spacing * (m_cols - 1)) / (f32)m_cols;
			const f32 cellH = (areaSize.y - m_spacing * (m_rows - 1)) / (f32)m_rows;

			u32 cellIndex = 0;
			const u32 maxCells = m_rows * m_cols;

			for_each_laid_out(parent, [&](Widget& c) {
				if (cellIndex >= maxCells) return;  // overflow — drop
				const auto& h = c.layoutHint();

				const u32 col = cellIndex % m_cols;
				const u32 row = cellIndex / m_cols;

				const u32 colSpan = std::max<u32>(1, h.colSpan);
				const u32 rowSpan = std::max<u32>(1, h.rowSpan);

				const f32 slotW = cellW * colSpan + m_spacing * (colSpan - 1);
				const f32 slotH = cellH * rowSpan + m_spacing * (rowSpan - 1);

				const f32 slotX = col * (cellW + m_spacing);  // local to content frame
				const f32 slotY = row * (cellH + m_spacing);

				// Apply alignment within the cell slot if the child isn't stretching.
				Vec2 size { slotW, slotH };
				if (h.alignMain != eAlign::Stretch || h.alignCross != eAlign::Stretch)
				{
					Vec2 pref = resolvePreferred(c);
					if (h.alignMain  != eAlign::Stretch) size.x = std::min(pref.x, slotW);
					if (h.alignCross != eAlign::Stretch) size.y = std::min(pref.y, slotH);
				}

				const f32 offX = (h.alignMain  == eAlign::Stretch) ? 0 : alignOffset(h.alignMain,  slotW, size.x);
				const f32 offY = (h.alignCross == eAlign::Stretch) ? 0 : alignOffset(h.alignCross, slotH, size.y);

				c.setPosition(slotX + offX, slotY + offY);
				c.setSize(size);
				c.onBoundsChanged(size);

				cellIndex += colSpan;
			});
		}

		Vec2 GridLayout::measure(const Widget& parent) const
		{
			// Find the largest preferred size among children; report grid total.
			f32 maxW = 0, maxH = 0;
			for_each_laid_out(parent, [&](Widget& c) {
				Vec2 pref = resolvePreferred(c);
				maxW = std::max(maxW, pref.x);
				maxH = std::max(maxH, pref.y);
			});
			const f32 totalW = maxW * m_cols + m_spacing * (m_cols - 1);
			const f32 totalH = maxH * m_rows + m_spacing * (m_rows - 1);
			return { totalW, totalH };
		}
	}
}
