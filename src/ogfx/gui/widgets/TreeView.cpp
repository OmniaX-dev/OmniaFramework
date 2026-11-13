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

#include "TreeView.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../Window.hpp"
#include "../../../ostd/io/Memory.hpp"
#include <algorithm>
#include <chrono>

namespace ogfx
{
	namespace gui
	{
		void TreeView::Item::setText(const String& text)
		{
			if (m_text == text)
				return;
			m_text = text;
			update_dimensions();
			if (m_treeView) m_treeView->m_extentsDirty = true;
		}

		void TreeView::Item::setIcon(const Icon& icon)
		{
			m_icon = icon;
			enableIcon();
		}

		void TreeView::Item::update_dimensions(void)
		{
			if (!isValid())
				return;
			auto pad = m_treeView->getLinePadding();
			m_dimensions = m_treeView->getWindow().getGFX().getStringDimensions(m_text, m_treeView->getFontSize());
			m_dimensions += Vec2 { pad.x + pad.w, pad.y + pad.h };
		}

		void TreeView::Item::set_selected(stdvec<Item*>& selectionList)
		{
			for (auto& sel : selectionList)
				sel->m_selected = false;
			selectionList.clear();
			selectionList.push_back(this);
			m_selected = true;
		}

		void TreeView::Item::add_selected(stdvec<Item*>& selectionList)
		{
			if (STDVEC_CONTAINS(selectionList, this))
				return;
			selectionList.push_back(this);
			m_selected = true;
		}

		void TreeView::Item::remove_selected(stdvec<Item*>& selectionList)
		{
			if (!STDVEC_CONTAINS(selectionList, this))
				return;
			STDVEC_REMOVE(selectionList, this);
			m_selected = false;
		}

		void TreeView::Item::setExpanded(bool expanded)
		{
			if (m_expanded == expanded)
				return;
			m_expanded = expanded;
			if (m_treeView) m_treeView->m_extentsDirty = true;
		}

		bool TreeView::Item::isVisible(void) const
		{
			// An item is visible if every ancestor is expanded.
			const Item* p = m_parentItem;
			while (p != nullptr)
			{
				if (!p->m_expanded) return false;
				p = p->m_parentItem;
			}
			return true;
		}

		stdvec<String> TreeView::Item::getFullPath(void) const
		{
			// Walk parent pointers up to the root, collecting text bottom-up,
			// then reverse so the result reads root -> ... -> this item.
			stdvec<String> path;
			path.push_back(m_text);
			const Item* p = m_parentItem;
			while (p != nullptr)
			{
				path.push_back(p->m_text);
				p = p->m_parentItem;
			}
			std::reverse(path.begin(), path.end());
			return path;
		}






		TreeView& TreeView::create(void)
		{
			setPadding({ 0, 0, 0, 0 });
			setTypeName("ogfx::gui::TreeView");
			enableStopEvents();
			enableBackground();
			enableBorder();
			setBackgroundColor({ 160, 160, 160 });
			setBorderColor({ 50, 50, 50 });
			setStylesheetCategoryName("tree");
			reloadTheme();
			validate();
			return *this;
		}

		void TreeView::applyTheme(const ostd::Stylesheet& theme)
		{
			enableAlternatingBackground(getThemeValue<bool>(theme, "showAlternatingBackground", isAlternatingBackgroundEnabled()));
			setBackgroundColor2(getThemeValue<Color>(theme, "backgroundColor2", getBackgroundColor2()));
			setSeparatorLineColor(getThemeValue<Color>(theme, "separatorLineColor", getSeparatorLineColor()));
			enableShowSeparatorLine(getThemeValue<bool>(theme, "showSeparatorLine", isShowSeparatorLineEnabled()));
			setLinePadding(getThemeValue<Rectangle>(theme, "linePadding", getLinePadding()));
			setLineSelectionColor(getThemeValue<Color>(theme, "selectionColor", getLineSelectionColor()));
			setLineSelectionTextColor(getThemeValue<Color>(theme, "selectionTextColor", getLineSelectionTextColor()));
			setIconSpacing(getThemeValue<f32>(theme, "iconSpacing", getIconSpacing()));
			setIconInset(getThemeValue<f32>(theme, "iconInset", getIconInset()));
			enableIcons(getThemeValue<bool>(theme, "showIcons", isIconsEnabled()));
			setIconTintColor(getThemeValue<Color>(theme, "iconTintColor", getIconTintColor()));
			setIndentWidth(getThemeValue<f32>(theme, "indentWidth", getIndentWidth()));
			setChevronColor(getThemeValue<Color>(theme, "chevronColor", getChevronColor()));
			setChevronInset(getThemeValue<f32>(theme, "chevronInset", getChevronInset()));
			setBranchLineColor(getThemeValue<Color>(theme, "branchLineColor", getBranchLineColor()));
			enableBranchLines(getThemeValue<bool>(theme, "showBranchLines", isBranchLinesEnabled()));
			setBranchLineThickness(getThemeValue<f32>(theme, "branchLineThickness", getBranchLineThickness()));
			setHighlightColor(getThemeValue<Color>(theme, "highlightColor", getHighlightColor()));
		}

		void TreeView::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			// Rows never draw under the scrollbars.
			gfx.pushClippingRect(getGlobalPureContentBounds(), true);
			draw_rows(gfx);
			gfx.popClippingRect();
		}

		void TreeView::draw_rows(ogfx::BasicRenderer2D& gfx)
		{
			const auto& bounds = getGlobalBounds();
			const auto& content = getContentExtents();
			const f32 scrollY = -getScrollOffset().y;
			const f32 visibleH = getContentBounds().h;
			const f32 visibleStart = scrollY;
			const f32 visibleEnd  = scrollY + visibleH;
			Color textColor;

			// Walk forward through visible items only, advancing y for each one we draw.
			f32 y = 0;
			i32 startIdx = 0;
			u32 visibleRow = 0; // counts only items whose row is actually drawn
			for (auto& itemPtr : m_list)
			{
				auto& item = *itemPtr;
				if (!item.isValid() || !item.isVisible()) { startIdx++; continue; }
				f32 itemH = item.getDimensions().y;
				if (y + itemH > visibleStart) break;
				y += itemH;
				visibleRow++;
				startIdx++;
			}

			for (i32 i = startIdx; i < (i32)m_list.size(); i++)
			{
				auto& item = *m_list[i];
				if (!item.isValid()) continue;
				if (!item.isVisible()) continue;
				f32 itemH = item.getDimensions().y;
				if (y > visibleEnd) break;
				f32 lineW = content.w;
				if (content.w < bounds.w)
					lineW = bounds.w;
				auto pad = getLinePadding();
				Rectangle lineRect = { Vec2 { bounds.x, bounds.y + y + pad.right() } + getScrollOffset(), { lineW, itemH } };
				if (item.isSelected())
				{
					textColor = getLineSelectionTextColor();
					gfx.fillRect(lineRect, getLineSelectionColor());
				}
				else
				{
					textColor = (item.hasTextColor() ? item.getTextColor() : getTextColor());
					if (isAlternatingBackgroundEnabled())
					{
						bool lineColorToggle = visibleRow % 2 == 0;
						gfx.fillRect(lineRect, (lineColorToggle ? getBackgroundColor() : getBackgroundColor2()));
					}
				}

				// Branch lines (drawn under everything else for this row).
				if (isBranchLinesEnabled())
					render_branch_lines(gfx, item, lineRect);

				// The chevron occupies one slot at column == depth.
				// Text/icon start one slot further right (column == depth + 1).
				const f32 indent = getIndentWidth();
				const f32 chevronX = bounds.x + item.getDepth() * indent + getScrollOffset().x;
				const f32 contentLeftX = chevronX + indent; // where icon (if any) or text starts

				if (item.hasChildren())
				{
					Rectangle chevronBounds {
						chevronX,
						bounds.y + y + pad.h + getScrollOffset().y,
						indent,
						itemH
					};
					if (m_chevronDrawCallback)
					{
						gfx.pushClippingRect(chevronBounds, true);
						m_chevronDrawCallback(*this, item, chevronBounds, gfx);
						gfx.popClippingRect();
					}
					else
					{
						render_chevron(gfx, item, chevronBounds);
					}
				}

				f32 textX = contentLeftX - bounds.x - getScrollOffset().x;
				if (item.isIconEnabled() && isIconsEnabled())
				{
					gfx.drawAnimation(item.getIcon(),
						Vec2 { contentLeftX + getIconInset(), bounds.y + getIconInset() + y + pad.h + getScrollOffset().y },
						{ itemH - (getIconInset() * 2), itemH - (getIconInset() * 2) },
						getIconTintColor());
					textX += itemH + getIconSpacing();
				}
				if (isShowSeparatorLineEnabled())
					gfx.drawLine({ Vec2 { bounds.x, bounds.y + y + itemH + pad.h } + getScrollOffset(), Vec2 { bounds.x + lineW, bounds.y + y + itemH + pad.h } + getScrollOffset() }, getSeparatorLineColor(), 1);
				gfx.drawVCenteredString(item, lineRect + Rectangle { textX, 0, -(textX * 2), 0 }, textColor, getFontSize());
				if (&item == m_highlightedItem)
					gfx.drawRect({ lineRect.x + 1, lineRect.y + 1, std::min(lineRect.w, getContentBounds().w) - 2, lineRect.h - 2 }, m_highlightColor, 2);
				y += itemH;
				visibleRow++;
			}
		}

		void TreeView::onUpdate(void)
		{
			ScrollableWidget::onUpdate();
			const f32 scrollY = -getScrollOffset().y;
			const f32 visibleH = getContentBounds().h;
			const f32 visibleStart = scrollY;
			const f32 visibleEnd  = scrollY + visibleH;
			f32 y = 0;
			i32 startIdx = 0;
			for (auto& itemPtr : m_list)
			{
				auto& item = *itemPtr;
				if (!item.isValid() || !item.isVisible()) { startIdx++; continue; }
				f32 itemH = item.getDimensions().y;
				if (y + itemH > visibleStart) break;
				y += itemH;
				startIdx++;
			}

			for (i32 i = startIdx; i < (i32)m_list.size(); i++)
			{
				auto& item = *m_list[i];
				if (!item.isValid()) continue;
				if (!item.isVisible()) continue;
				f32 itemH = item.getDimensions().y;
				if (y > visibleEnd) break;
				if (item.isIconEnabled())
					item.getIcon().update();
				y += itemH;
			}
		}

		void TreeView::afterDraw(ogfx::BasicRenderer2D& gfx)
		{
			drawScrollbars(gfx);
		}

		void TreeView::onMouseReleased(const Event& event)
		{
			if (!isMouseInside())
				return;
			const bool left = (event.mouse->button == ogfx::MouseEventData::eButton::Left);
			const bool right = (event.mouse->button == ogfx::MouseEventData::eButton::Right);
			if (!left && !right)
				return;
			if (isMouseInsideAnyScrollbar())
				return;

			const Vec2 mousePos { event.mouse->position_x, event.mouse->position_y };
			Item* hit = getItemAt(mousePos);
			if (hit == nullptr)
			{
				if (right && m_onItemContext)
				{
					m_onItemContext(nullptr, mousePos);
					event.handle();
				}
				return;
			}
			Item& item = *hit;
			// Chevron hit-test takes priority over selection.
			if (left && item.hasChildren())
			{
				const f32 localX = mousePos.x - getGlobalBounds().x - getScrollOffset().x;
				const f32 chevronXLocal = item.getDepth() * getIndentWidth();
				if (localX >= chevronXLocal && localX < chevronXLocal + getIndentWidth())
				{
					item.toggleExpanded();
					event.handle();
					return;
				}
			}
			select_and_notify(item);
			event.handle();
			if (right)
			{
				if (m_onItemContext)
					m_onItemContext(&item, mousePos);
				return;
			}
			const u64 now = cast<u64>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
			const bool doubleClick = (m_lastClickedItem == &item && now - m_lastClickMs <= DoubleClickMs);
			m_lastClickedItem = (doubleClick ? nullptr : &item);
			m_lastClickMs = now;
			if (doubleClick)
			{
				if (m_onItemActivated)
					m_onItemActivated(item);
				else if (item.hasChildren())
					item.toggleExpanded();
			}
		}

		void TreeView::onKeyPressed(const Event& event)
		{
			if (!isFocused() || event.keyboard == nullptr)
				return;
			const i32 key = event.keyboard->keyCode;
			const stdvec<Item*> items = visible_items();
			if (items.empty())
				return;
			Item* current = (m_selectedList.empty() ? nullptr : m_selectedList.back());
			i32 index = -1;
			for (i32 i = 0; i < cast<i32>(items.size()); i++)
			{
				if (items[i] == current)
					index = i;
			}
			Item* target = nullptr;
			if (key == KeyCode::Up)
				target = items[std::max(0, index - 1)];
			else if (key == KeyCode::Down)
				target = items[std::min(cast<i32>(items.size()) - 1, index + 1)];
			else if (key == KeyCode::Home)
				target = items.front();
			else if (key == KeyCode::End)
				target = items.back();
			else if (key == KeyCode::Left && current != nullptr)
			{
				if (current->hasChildren() && current->isExpanded())
					current->setExpanded(false);
				else if (current->getParentItem() != nullptr)
					target = current->getParentItem();
			}
			else if (key == KeyCode::Right && current != nullptr && current->hasChildren())
			{
				if (!current->isExpanded())
					current->setExpanded(true);
				else if (index + 1 < cast<i32>(items.size()))
					target = items[index + 1];
			}
			else if ((key == KeyCode::Return || key == KeyCode::KpEnter) && current != nullptr)
			{
				if (m_onItemActivated)
					m_onItemActivated(*current);
				else if (current->hasChildren())
					current->toggleExpanded();
			}
			else
				return;
			if (target != nullptr && target != current)
			{
				select_and_notify(*target);
				ensureItemVisible(*target);
			}
			event.handle();
		}

		stdvec<TreeView::Item*> TreeView::visible_items(void) const
		{
			stdvec<Item*> items;
			for (const auto& itemPtr : m_list)
			{
				if (itemPtr->isValid() && itemPtr->isVisible())
					items.push_back(itemPtr.get());
			}
			return items;
		}

		void TreeView::select_and_notify(Item& item)
		{
			const bool wasSelected = item.isSelected() && m_selectedList.size() == 1;
			item.set_selected(m_selectedList);
			if (!wasSelected && callback_onSelectionChanged)
				callback_onSelectionChanged(m_selectedList);
		}

		TreeView::Item* TreeView::getItemAt(const Vec2& position)
		{
			const Rectangle bounds = getGlobalBounds();
			if (!getGlobalPureContentBounds().contains(position, true))
				return nullptr;
			const f32 localY = position.y - bounds.y - getScrollOffset().y;
			f32 y = 0;
			for (auto& itemPtr : m_list)
			{
				auto& item = *itemPtr;
				if (!item.isValid() || !item.isVisible())
					continue;
				const f32 itemH = item.getDimensions().y;
				if (localY >= y && localY < y + itemH)
					return &item;
				y += itemH;
			}
			return nullptr;
		}

		void TreeView::selectItem(Item& item, bool notify)
		{
			for (Item* p = item.getParentItem(); p != nullptr; p = p->getParentItem())
				p->setExpanded(true);
			if (notify)
				select_and_notify(item);
			else
				item.set_selected(m_selectedList);
		}

		void TreeView::ensureItemVisible(const Item& item)
		{
			f32 y = 0;
			for (const auto& itemPtr : m_list)
			{
				if (!itemPtr->isValid() || !itemPtr->isVisible())
					continue;
				const f32 itemH = itemPtr->getDimensions().y;
				if (itemPtr.get() == &item)
				{
					const f32 top = -getScrollOffset().y;
					const f32 visibleH = getContentBounds().h;
					if (y < top)
						setScrollOffset({ getScrollOffset().x, -y });
					else if (y + itemH > top + visibleH)
						setScrollOffset({ getScrollOffset().x, -(y + itemH - visibleH) });
					return;
				}
				y += itemH;
			}
		}

		void TreeView::clear(void)
		{
			m_selectedList.clear();
			m_list.clear();
			m_lastRoot = nullptr;
			m_highlightedItem = nullptr;
			m_lastClickedItem = nullptr;
			m_extentsDirty = true;
		}

		Rectangle TreeView::getContentExtents(void) const
		{
			if (!m_extentsDirty)
				return m_cachedExtents;
			f32 maxX = 0, maxY = 0;
			const f32 indent = m_indentWidth;
			for (auto& itemPtr : m_list)
			{
				auto& item = *itemPtr;
				if (!item.isValid()) continue;
				if (!item.isVisible()) continue;
				Vec2 size = item.getDimensions();
				// Width of a row is its text/icon width plus the indent + chevron-slot space to its left.
				f32 rowW = size.x + (item.getDepth() + 1) * indent;
				maxX = std::max(maxX, rowW);
				maxY += size.y;
			}
			m_cachedExtents = { 0, 0, maxX, maxY };
			m_extentsDirty = false;
			return m_cachedExtents;
		}

		TreeView::Item& TreeView::getLine(const String& text)
		{
			for (auto& itemPtr : m_list)
				if (itemPtr->getText() == text)
					return *itemPtr;
			return InvalidItem;
		}

		TreeView::Item& TreeView::getLine(u32 index)
		{
			if (index < m_list.size())
				return *m_list[index];
			return InvalidItem;
		}

		TreeView::Item& TreeView::addRoot(const String& text)
		{
			// Flip the previously-last root's flag, then become the new last root.
			if (m_lastRoot) m_lastRoot->m_isLastChild = false;

			auto item = std::make_unique<Item>(*this);
			item->setText(text);
			item->m_parentItem = nullptr;
			item->m_depth = 0;
			item->m_isLastChild = true;
			Item* itemPtr = item.get();
			m_list.push_back(std::move(item));
			m_lastRoot = itemPtr;

			if (m_selectedList.empty())
				itemPtr->set_selected(m_selectedList);
			m_extentsDirty = true;
			return *itemPtr;
		}

		TreeView::Item& TreeView::addRoot(const String& text, const Icon& icon)
		{
			auto& item = addRoot(text);
			item.setIcon(icon);
			return item;
		}

		TreeView::Item& TreeView::addChild(Item& parentItem, const String& text)
		{
			// Locate the parent's slot in m_list (needed for the insertion offset).
			i32 parentIdx = -1;
			for (i32 i = 0; i < (i32)m_list.size(); i++)
			{
				if (m_list[i].get() == &parentItem) { parentIdx = i; break; }
			}
			if (parentIdx < 0)
				return InvalidItem;

			// Insert right after the parent's last descendant so display order == deque order.
			u32 insertIdx = get_last_descendant_index((u32)parentIdx) + 1;

			// The previously-last direct child (if any) loses its last-child flag.
			if (parentItem.m_lastChild)
				parentItem.m_lastChild->m_isLastChild = false;

			auto newItem = std::make_unique<Item>(*this);
			newItem->setText(text);
			newItem->m_parentItem = &parentItem;
			newItem->m_depth      = parentItem.m_depth + 1;
			newItem->m_isLastChild = true;
			Item* newPtr = newItem.get();

			// std::deque::insert moves the pointer slots around, but the Item objects
			// themselves are heap-allocated and stay put. Pointers stored on other Items
			// (m_parentItem, m_lastChild) and in m_selectedList remain valid.
			m_list.insert(m_list.begin() + insertIdx, std::move(newItem));

			parentItem.m_hasChildren = true;
			parentItem.m_lastChild   = newPtr;
			m_extentsDirty = true;
			return *newPtr;
		}

		TreeView::Item& TreeView::addChild(Item& parentItem, const String& text, const Icon& icon)
		{
			auto& item = addChild(parentItem, text);
			item.setIcon(icon);
			return item;
		}

		u32 TreeView::get_last_descendant_index(u32 itemIndex) const
		{
			// Walk forward while items are deeper than the anchor; the last such index is the answer.
			if (itemIndex >= m_list.size()) return itemIndex;
			const u32 anchorDepth = m_list[itemIndex]->m_depth;
			u32 last = itemIndex;
			for (u32 i = itemIndex + 1; i < m_list.size(); i++)
			{
				if (m_list[i]->m_depth > anchorDepth)
					last = i;
				else
					break;
			}
			return last;
		}

		void TreeView::recompute_has_children_and_last_sibling(Item* parentItem)
		{
			// Recomputes m_hasChildren on parentItem (or m_lastRoot if null) and
			// m_isLastChild flags + cached last-child pointer for the children of that parent.
			Item* lastSeen = nullptr;
			bool any = false;
			for (auto& itPtr : m_list)
			{
				auto& it = *itPtr;
				if (!it.isValid()) continue;
				if (it.m_parentItem == parentItem)
				{
					it.m_isLastChild = false;
					lastSeen = &it;
					any = true;
				}
			}
			if (lastSeen) lastSeen->m_isLastChild = true;
			if (parentItem)
			{
				parentItem->m_hasChildren = any;
				parentItem->m_lastChild = lastSeen;
			}
			else
			{
				m_lastRoot = lastSeen;
			}
		}

		bool TreeView::removeLine(const String& text)
		{
			i32 idx = -1;
			for (i32 i = 0; i < (i32)m_list.size(); i++)
			{
				if (m_list[i]->getText() == text) { idx = i; break; }
			}
			if (idx < 0) return false;
			return removeLine((u32)idx);
		}

		bool TreeView::removeLine(u32 index)
		{
			if (!hasLine(index))
				return false;

			Item* removedParent = m_list[index]->m_parentItem;
			// Remove the item plus its entire descendant range.
			u32 lastDesc = get_last_descendant_index(index);
			// Drop any selection pointers that fall inside the removed range.
			for (u32 i = index; i <= lastDesc; i++)
			{
				Item* p = m_list[i].get();
				STDVEC_REMOVE(m_selectedList, p);
				if (p == m_highlightedItem)
					m_highlightedItem = nullptr;
				if (p == m_lastClickedItem)
					m_lastClickedItem = nullptr;
			}
			m_list.erase(m_list.begin() + index, m_list.begin() + lastDesc + 1);

			// Fix up parent flags / sibling last-child markers.
			recompute_has_children_and_last_sibling(removedParent);
			m_extentsDirty = true;
			return true;
		}

		bool TreeView::hasLine(const String& text)
		{
			for (auto& itemPtr : m_list)
				if (itemPtr->getText() == text)
					return true;
			return false;
		}

		bool TreeView::hasLine(u32 index)
		{
			return m_list.size() > index;
		}

		void TreeView::expandAll(void)
		{
			for (auto& itemPtr : m_list)
			{
				if (itemPtr->m_hasChildren && !itemPtr->m_expanded)
					itemPtr->m_expanded = true;
			}
			m_extentsDirty = true;
		}

		void TreeView::collapseAll(void)
		{
			for (auto& itemPtr : m_list)
			{
				if (itemPtr->m_hasChildren && itemPtr->m_expanded)
					itemPtr->m_expanded = false;
			}
			m_extentsDirty = true;
		}

		void TreeView::render_chevron(ogfx::BasicRenderer2D& gfx, const Item& item, const Rectangle& bounds)
		{
			// Default chevron: a small triangle. Right-pointing when collapsed, down-pointing when expanded.
			const f32 inset = getChevronInset();
			const f32 cx = bounds.x + bounds.w * 0.5f;
			const f32 cy = bounds.y + bounds.h * 0.5f;
			// Half-extents inside the slot.
			const f32 hx = std::max(2.0f, bounds.w * 0.5f - inset);
			const f32 hy = std::max(2.0f, bounds.h * 0.25f);
			Vec2 p0, p1, p2;
			if (item.isExpanded())
			{
				// Down-pointing: ▼
				p0 = { cx - hx, cy - hy };
				p1 = { cx + hx, cy - hy };
				p2 = { cx,      cy + hy };
			}
			else
			{
				// Right-pointing: ▶
				p0 = { cx - hy, cy - hx };
				p1 = { cx - hy, cy + hx };
				p2 = { cx + hy, cy      };
			}
			gfx.fillTriangle(p0, p1, p2, getChevronColor());
		}

		void TreeView::render_branch_lines(ogfx::BasicRenderer2D& gfx, const Item& item, const Rectangle& rowRect)
		{
			const f32 indent = getIndentWidth();
			const f32 thick  = getBranchLineThickness();
			const Color col  = getBranchLineColor();
			const u32 d      = item.getDepth();

			if (d == 0) return; // roots have no parent line / no elbow

			// Collect ancestor "isLastChild" flags. ancestorIsLast[0] = immediate parent (depth d-1),
			// ancestorIsLast[k] = ancestor at depth (d-1-k).
			std::vector<bool> ancestorIsLast;
			ancestorIsLast.reserve(d);
			const Item* p = item.getParentItem();
			while (p != nullptr)
			{
				ancestorIsLast.push_back(p->isLastChild());
				p = p->getParentItem();
			}

			// For each "passing-through" column a in [0, d-1): draw a vertical line spanning the
			// full row IF the ancestor at depth (a+1) is NOT the last child of its parent.
			// (Column 'a' carries the sibling line for depth a+1; if the depth-a+1 ancestor has
			// no later sibling, the line ended above us and shouldn't be drawn here.)
			// The column for our own row (a == d-1) is handled by the elbow logic below.
			for (u32 a = 0; a + 1 < d; a++)
			{
				const u32 ancestorDepth  = a + 1;
				const u32 ancestorIdx    = d - 1 - ancestorDepth; // index into ancestorIsLast
				if (ancestorIsLast[ancestorIdx]) continue; // no continuing line in this column
				const f32 colX = rowRect.x + a * indent + indent * 0.5f;
				gfx.drawLine({ Vec2 { colX, rowRect.y }, Vec2 { colX, rowRect.y + rowRect.h } }, col, thick);
			}

			// Elbow in column d-1 connecting our row to the chevron slot at column d.
			const f32 colX   = rowRect.x + (d - 1) * indent + indent * 0.5f;
			const f32 midY   = rowRect.y + rowRect.h * 0.5f;
			// If this item has no chevron drawn, extend the horizontal arm all the way to where
			// the icon/text starts (the right edge of the chevron slot, i.e. column d+1's left).
			// Otherwise stop at the left edge of the chevron slot so the chevron sits clean.
			const f32 armEndX = item.hasChildren()
				? rowRect.x + d * indent
				: rowRect.x + (d + 1) * indent;

			// Vertical: top -> middle of the row (always)
			gfx.drawLine({ Vec2 { colX, rowRect.y }, Vec2 { colX, midY } }, col, thick);
			// Vertical: middle -> bottom (only if there is a sibling below us at this depth)
			if (!item.isLastChild())
				gfx.drawLine({ Vec2 { colX, midY }, Vec2 { colX, rowRect.y + rowRect.h } }, col, thick);
			// Horizontal arm
			gfx.drawLine({ Vec2 { colX, midY }, Vec2 { armEndX, midY } }, col, thick);
		}
	}
}
