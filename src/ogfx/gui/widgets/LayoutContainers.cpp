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

#include "LayoutContainers.hpp"

#include <algorithm>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		// ============================================================================
		// Container
		// ============================================================================

		Container::Container(Window& window) : Widget({ 0, 0, 0, 0 }, window)
		{
			setTypeName("ogfx::gui::Container");
			setStylesheetCategoryName("container");
			setPadding({ 0, 0, 0, 0 });
			disableFocus();
			disableBackground();
			disableBorder();
			// Containers are transparent to the mouse: they never swallow events
			// meant for overlapping siblings.
			enableStopEvents(false);
			validate();
		}

		void Container::layoutChildren(void)
		{
			// A child changing its minimum size while we arrange would re-enter;
			// remember the request and run one more pass instead.
			if (m_arranging)
			{
				m_arrangeRequested = true;
				return;
			}
			m_arranging = true;
			i32 passes = 0;
			do
			{
				m_arrangeRequested = false;
				arrangeChildren(getContentBounds().getSize());
			} while (m_arrangeRequested && ++passes < 4);
			m_arranging = false;
		}

		Vec2 Container::getContentMinSize(void) const
		{
			return contentToWidgetSize(measureContent());
		}

		void Container::fitChildInRect(Widget& child, const Rectangle& rect) const
		{
			const Vec2 minSize = child.getCombinedMinSize();
			Rectangle r = rect;

			auto l_fit = [](eSizeFlags flags, f32 slotPos, f32 slotLen, f32 minLen, f32& outPos, f32& outLen) {
				if (hasSizeFlag(flags, eSizeFlags::Fill))
				{
					outPos = slotPos;
					outLen = slotLen;
					return;
				}
				outLen = minLen;
				if (hasSizeFlag(flags, eSizeFlags::ShrinkCenter))
					outPos = slotPos + std::floor((slotLen - minLen) * 0.5f);
				else if (hasSizeFlag(flags, eSizeFlags::ShrinkEnd))
					outPos = slotPos + (slotLen - minLen);
				else
					outPos = slotPos;
			};
			l_fit(child.getHSizeFlags(), rect.x, rect.w, minSize.x, r.x, r.w);
			l_fit(child.getVSizeFlags(), rect.y, rect.h, minSize.y, r.y, r.h);
			child.setLayoutRect(r);
		}

		stdvec<Widget*> Container::getLayoutChildren(void) const
		{
			stdvec<Widget*> list;
			auto& children = const_cast<Container*>(this)->getChildren();
			for (auto* c : children.getWidgets())
			{
				if (!c) continue;
				if (c->isInvalid()) continue;
				if (!c->isVisible()) continue;
				if (c->isIgnoreScrollEnabled()) continue;
				list.push_back(c);
			}
			return list;
		}

		void Container::containerPropertyChanged(void)
		{
			updateMinimumSize();
			layoutChildren();
		}


		// ============================================================================
		// BoxContainer
		// ============================================================================

		BoxContainer::BoxContainer(Window& window, eOrientation orientation) : Container(window), m_orientation(orientation)
		{
			setTypeName("ogfx::gui::BoxContainer");
		}

		void BoxContainer::setOrientation(eOrientation orientation)
		{
			if (m_orientation == orientation) return;
			m_orientation = orientation;
			containerPropertyChanged();
		}

		void BoxContainer::setSpacing(f32 spacing)
		{
			m_spacing = std::max(0.0f, spacing);
			containerPropertyChanged();
		}

		void BoxContainer::setAlignment(eBoxAlignment alignment)
		{
			m_alignment = alignment;
			layoutChildren();
		}

		void BoxContainer::arrangeChildren(const Vec2& area)
		{
			const auto children = getLayoutChildren();
			if (children.empty())
				return;

			const bool vert = isVertical();
			const f32 availMain = vert ? area.y : area.x;
			const f32 cross = vert ? area.x : area.y;

			struct Slot { Widget* w; f32 minMain; f32 ratio; bool expand; f32 size; };
			stdvec<Slot> slots;
			slots.reserve(children.size());

			// Space available to expanding children (non-expanding ones and the
			// spacing take what they need first).
			f32 stretchAvail = availMain - (m_spacing * cast<f32>(children.size() - 1));
			f32 ratioTotal = 0.0f;
			bool anyExpand = false;
			for (auto* c : children)
			{
				const Vec2 m = c->getCombinedMinSize();
				Slot s { c, vert ? m.y : m.x, c->getStretchRatio(), false, 0.0f };
				s.expand = hasSizeFlag(vert ? c->getVSizeFlags() : c->getHSizeFlags(), eSizeFlags::Expand);
				s.size = s.minMain;
				if (s.expand)
				{
					ratioTotal += s.ratio;
					anyExpand = true;
				}
				else
					stretchAvail -= s.minMain;
				slots.push_back(s);
			}

			// Distribute the leftover proportionally. An expanding child whose share
			// would be below its minimum is fixed at its minimum and the share is
			// recomputed for the others (same algorithm as Godot's BoxContainer).
			bool refit = true;
			while (refit)
			{
				refit = false;
				for (auto& s : slots)
				{
					if (!s.expand) continue;
					const f32 share = (ratioTotal > 0.0f) ? stretchAvail * (s.ratio / ratioTotal) : 0.0f;
					if (share < s.minMain)
					{
						s.expand = false;
						s.size = s.minMain;
						stretchAvail -= s.minMain;
						ratioTotal -= s.ratio;
						refit = true;
						break;
					}
					s.size = share;
				}
			}

			// Group alignment when nothing expands.
			f32 used = m_spacing * cast<f32>(slots.size() - 1);
			for (auto& s : slots)
				used += s.size;
			f32 cursor = 0.0f;
			if (!anyExpand && used < availMain)
			{
				if (m_alignment == eBoxAlignment::Center)
					cursor = std::floor((availMain - used) * 0.5f);
				else if (m_alignment == eBoxAlignment::End)
					cursor = availMain - used;
			}

			for (auto& s : slots)
			{
				// Round both edges so adjacent children stay pixel-aligned.
				const f32 from = std::round(cursor);
				const f32 to = std::round(cursor + s.size);
				if (vert)
					fitChildInRect(*s.w, { 0, from, cross, to - from });
				else
					fitChildInRect(*s.w, { from, 0, to - from, cross });
				cursor += s.size + m_spacing;
			}
		}

		Vec2 BoxContainer::measureContent(void) const
		{
			const auto children = getLayoutChildren();
			const bool vert = isVertical();
			f32 mainSum = 0.0f, crossMax = 0.0f;
			for (auto* c : children)
			{
				const Vec2 m = c->getCombinedMinSize();
				mainSum += vert ? m.y : m.x;
				crossMax = std::max(crossMax, vert ? m.x : m.y);
			}
			if (!children.empty())
				mainSum += m_spacing * cast<f32>(children.size() - 1);
			return vert ? Vec2 { crossMax, mainSum } : Vec2 { mainSum, crossMax };
		}


		// ============================================================================
		// GridContainer
		// ============================================================================

		GridContainer::GridContainer(Window& window, u32 columns) : Container(window), m_columns(std::max<u32>(1, columns))
		{
			setTypeName("ogfx::gui::GridContainer");
		}

		void GridContainer::setColumns(u32 columns)
		{
			m_columns = std::max<u32>(1, columns);
			containerPropertyChanged();
		}

		void GridContainer::setSpacing(f32 horizontal, f32 vertical)
		{
			m_hSpacing = std::max(0.0f, horizontal);
			m_vSpacing = std::max(0.0f, vertical);
			containerPropertyChanged();
		}

		void GridContainer::compute_mins(const stdvec<Widget*>& children, u32 cols, stdvec<f32>& colMin, stdvec<f32>& rowMin, stdvec<bool>& colExpand, stdvec<bool>& rowExpand) const
		{
			const u32 rows = (cast<u32>(children.size()) + cols - 1) / cols;
			colMin.assign(cols, 0.0f);
			rowMin.assign(rows, 0.0f);
			colExpand.assign(cols, false);
			rowExpand.assign(rows, false);
			for (u32 i = 0; i < children.size(); i++)
			{
				const u32 c = i % cols, r = i / cols;
				const Vec2 m = children[i]->getCombinedMinSize();
				colMin[c] = std::max(colMin[c], m.x);
				rowMin[r] = std::max(rowMin[r], m.y);
				if (hasSizeFlag(children[i]->getHSizeFlags(), eSizeFlags::Expand)) colExpand[c] = true;
				if (hasSizeFlag(children[i]->getVSizeFlags(), eSizeFlags::Expand)) rowExpand[r] = true;
			}
		}

		void GridContainer::arrangeChildren(const Vec2& area)
		{
			const auto children = getLayoutChildren();
			if (children.empty())
				return;
			const u32 cols = std::min<u32>(m_columns, cast<u32>(children.size()));

			stdvec<f32> colMin, rowMin;
			stdvec<bool> colExpand, rowExpand;
			compute_mins(children, cols, colMin, rowMin, colExpand, rowExpand);

			// Shares the leftover space equally among expanding tracks; a track
			// whose share would be below its minimum keeps its minimum instead.
			auto l_distribute = [](const stdvec<f32>& mins, stdvec<bool> expand, f32 avail) -> stdvec<f32> {
				stdvec<f32> sizes = mins;
				f32 stretchAvail = avail;
				i32 expandCount = 0;
				for (size_t i = 0; i < mins.size(); i++)
				{
					if (expand[i]) expandCount++;
					else stretchAvail -= mins[i];
				}
				bool refit = true;
				while (refit && expandCount > 0)
				{
					refit = false;
					const f32 share = stretchAvail / cast<f32>(expandCount);
					for (size_t i = 0; i < mins.size(); i++)
					{
						if (!expand[i]) continue;
						if (share < mins[i])
						{
							expand[i] = false;
							stretchAvail -= mins[i];
							expandCount--;
							refit = true;
							break;
						}
						sizes[i] = share;
					}
				}
				return sizes;
			};

			const stdvec<f32> colW = l_distribute(colMin, colExpand, area.x - m_hSpacing * cast<f32>(colMin.size() - 1));
			const stdvec<f32> rowH = l_distribute(rowMin, rowExpand, area.y - m_vSpacing * cast<f32>(rowMin.size() - 1));

			stdvec<f32> colX(colW.size(), 0.0f), rowY(rowH.size(), 0.0f);
			for (size_t i = 1; i < colW.size(); i++) colX[i] = colX[i - 1] + colW[i - 1] + m_hSpacing;
			for (size_t i = 1; i < rowH.size(); i++) rowY[i] = rowY[i - 1] + rowH[i - 1] + m_vSpacing;

			for (u32 i = 0; i < children.size(); i++)
			{
				const u32 c = i % cols, r = i / cols;
				const f32 x0 = std::round(colX[c]), x1 = std::round(colX[c] + colW[c]);
				const f32 y0 = std::round(rowY[r]), y1 = std::round(rowY[r] + rowH[r]);
				fitChildInRect(*children[i], { x0, y0, x1 - x0, y1 - y0 });
			}
		}

		Vec2 GridContainer::measureContent(void) const
		{
			const auto children = getLayoutChildren();
			if (children.empty())
				return { 0, 0 };
			const u32 cols = std::min<u32>(m_columns, cast<u32>(children.size()));
			stdvec<f32> colMin, rowMin;
			stdvec<bool> colExpand, rowExpand;
			compute_mins(children, cols, colMin, rowMin, colExpand, rowExpand);
			f32 w = m_hSpacing * cast<f32>(colMin.size() - 1);
			f32 h = m_vSpacing * cast<f32>(rowMin.size() - 1);
			for (f32 v : colMin) w += v;
			for (f32 v : rowMin) h += v;
			return { w, h };
		}


		// ============================================================================
		// MarginContainer
		// ============================================================================

		MarginContainer::MarginContainer(Window& window, const EdgeValues& margins) : Container(window), m_margins(margins)
		{
			setTypeName("ogfx::gui::MarginContainer");
		}

		void MarginContainer::setMargins(const EdgeValues& margins)
		{
			m_margins = margins;
			containerPropertyChanged();
		}

		void MarginContainer::arrangeChildren(const Vec2& area)
		{
			const Rectangle slot {
				m_margins.left,
				m_margins.top,
				std::max(0.0f, area.x - m_margins.left - m_margins.right),
				std::max(0.0f, area.y - m_margins.top - m_margins.bottom)
			};
			for (auto* c : getLayoutChildren())
				fitChildInRect(*c, slot);
		}

		Vec2 MarginContainer::measureContent(void) const
		{
			Vec2 result { 0, 0 };
			for (auto* c : getLayoutChildren())
			{
				const Vec2 m = c->getCombinedMinSize();
				result.x = std::max(result.x, m.x);
				result.y = std::max(result.y, m.y);
			}
			return result + Vec2 { m_margins.left + m_margins.right, m_margins.top + m_margins.bottom };
		}


		// ============================================================================
		// CenterContainer
		// ============================================================================

		CenterContainer::CenterContainer(Window& window) : Container(window)
		{
			setTypeName("ogfx::gui::CenterContainer");
		}

		void CenterContainer::arrangeChildren(const Vec2& area)
		{
			for (auto* c : getLayoutChildren())
			{
				const Vec2 m = c->getCombinedMinSize();
				const f32 x = std::floor((area.x - m.x) * 0.5f);
				const f32 y = std::floor((area.y - m.y) * 0.5f);
				c->setLayoutRect({ x, y, m.x, m.y });
			}
		}

		Vec2 CenterContainer::measureContent(void) const
		{
			Vec2 result { 0, 0 };
			for (auto* c : getLayoutChildren())
			{
				const Vec2 m = c->getCombinedMinSize();
				result.x = std::max(result.x, m.x);
				result.y = std::max(result.y, m.y);
			}
			return result;
		}
	}
}
