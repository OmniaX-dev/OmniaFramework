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
#include <ogfx/utils/Animation.hpp>
#include <deque>
#include <memory>

namespace ogfx
{
	namespace gui
	{
		class TreeView : public ScrollableWidget
		{
			public: class Item : public ostd::__i_stringeable
			{
				public:
					inline Item(TreeView& parent) { m_treeView = &parent; }
					void setText(const String& text);
					void setIcon(const Icon& icon);
					inline Animation& getIcon(void) { return m_icon; }

					inline String toString(void) const override { return m_text; }
					inline bool isValid(void) const { return m_treeView != nullptr && m_text.new_trim() != ""; }
					inline Vec2 getDimensions(void) const { return m_dimensions; }
					inline String getText(void) const { return m_text; }
					inline bool isSelected(void) const { return m_selected; }

					inline bool isExpanded(void) const { return m_expanded; }
					inline bool hasChildren(void) const { return m_hasChildren; }
					inline u32 getDepth(void) const { return m_depth; }
					inline Item* getParentItem(void) { return m_parentItem; }
					inline const Item* getParentItem(void) const { return m_parentItem; }
					inline bool isLastChild(void) const { return m_isLastChild; }
					void setExpanded(bool expanded);
					inline void toggleExpanded(void) { setExpanded(!m_expanded); }
					bool isVisible(void) const;
					stdvec<String> getFullPath(void) const;

					OSTD_BOOL_PARAM_GETSET_E(Icon, m_showIcon);

				private:
					void update_dimensions(void);
					void set_selected(stdvec<Item*>& selectionList);
					void add_selected(stdvec<Item*>& selectionList);
					void remove_selected(stdvec<Item*>& selectionList);

				private:
					Vec2 m_dimensions { 0, 0 };
					bool m_selected { false };
					TreeView* m_treeView { nullptr };
					String m_text { "" };
					Animation m_icon;
					bool m_showIcon { false };

					// Tree state
					Item* m_parentItem { nullptr };
					Item* m_lastChild { nullptr };
					u32 m_depth { 0 };
					bool m_expanded { true };
					bool m_hasChildren { false };
					bool m_isLastChild { true };

					friend class TreeView;
			};
			public:
				using ChevronDrawCallback = std::function<void(TreeView& sender, const Item& item, const Rectangle& bounds, BasicRenderer2D& gfx)>;

				inline TreeView(Window& window) : ScrollableWidget(window) { create(); }
				TreeView& create(void);
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;
				void afterDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMouseReleased(const Event& event) override;
				Rectangle getContentExtents(void) const override;
				Item& getLine(const String& text);
				Item& getLine(u32 index);
				Item& addRoot(const String& text);
				Item& addRoot(const String& text, const Icon& icon);
				Item& addChild(Item& parentItem, const String& text);
				Item& addChild(Item& parentItem, const String& text, const Icon& icon);
				bool removeLine(const String& text);
				bool removeLine(u32 index);
				bool hasLine(const String& text);
				bool hasLine(u32 index);
				void expandAll(void);
				void collapseAll(void);
				inline stdvec<Item*>& getSelection(void) { return m_selectedList; }
				inline void setSelectionChangedCallback(std::function<void(stdvec<Item*>& selection)> callback) { callback_onSelectionChanged = std::move(callback); }
				inline void setChevronDrawCallback(ChevronDrawCallback cb) { m_chevronDrawCallback = std::move(cb); }


				OSTD_PARAM_GETSET(Color, BackgroundColor2, m_backgroundColor2);
				OSTD_BOOL_PARAM_GETSET_E(AlternatingBackground, m_showAlternatingBackground);
				OSTD_PARAM_GETSET(Color, SeparatorLineColor, m_lineColor);
				OSTD_BOOL_PARAM_GETSET_E(ShowSeparatorLine, m_showLine);
				OSTD_BOOL_PARAM_GETSET_E(Icons, m_showIcons);
				OSTD_PARAM_GETSET(Rectangle, LinePadding, m_linePadding);
				OSTD_PARAM_GETSET(Color, LineSelectionColor, m_selectionColor);
				OSTD_PARAM_GETSET(Color, LineSelectionTextColor, m_selectionTextColor);
				OSTD_PARAM_GETSET(Color, IconTintColor, m_iconTintColor);
				OSTD_PARAM_GETSET(f32, IconSpacing, m_iconSpacing);
				OSTD_PARAM_GETSET(f32, IconInset, m_iconInset);
				OSTD_PARAM_GETSET(f32, IndentWidth, m_indentWidth);
				OSTD_PARAM_GETSET(Color, ChevronColor, m_chevronColor);
				OSTD_PARAM_GETSET(f32, ChevronInset, m_chevronInset);
				OSTD_PARAM_GETSET(Color, BranchLineColor, m_branchLineColor);
				OSTD_BOOL_PARAM_GETSET_E(BranchLines, m_showBranchLines);
				OSTD_PARAM_GETSET(f32, BranchLineThickness, m_branchLineThickness);

			private:
				void render_chevron(ogfx::BasicRenderer2D& gfx, const Item& item, const Rectangle& bounds);
				void render_branch_lines(ogfx::BasicRenderer2D& gfx, const Item& item, const Rectangle& rowRect);
				u32 get_last_descendant_index(u32 itemIndex) const;
				void recompute_has_children_and_last_sibling(Item* parentItem);

			private:
				Item InvalidItem { *this };
				std::deque<std::unique_ptr<Item>> m_list;
				stdvec<Item*> m_selectedList;
				std::function<void(stdvec<Item*>& selection)> callback_onSelectionChanged { nullptr };
				ChevronDrawCallback m_chevronDrawCallback { nullptr };
				mutable Rectangle m_cachedExtents { 0, 0, 0, 0 };
				mutable bool m_extentsDirty { true };
				Item* m_lastRoot { nullptr };

				bool m_showIcons { false };
				f32 m_iconSpacing { 0 };
				f32 m_iconInset { 4 };
				Color m_lineColor { 40, 40, 40 };
				bool m_showLine { true };
				Rectangle m_linePadding { 5, 5, 20, 0 };
				Color m_selectionColor { Colors::Crimson };
				Color m_selectionTextColor{ Colors::White };
				Color m_iconTintColor { Colors::White };
				Color m_backgroundColor2 { Colors::Black };
				bool m_showAlternatingBackground { true };

				f32 m_indentWidth { 16 };
				Color m_chevronColor { Colors::White };
				f32 m_chevronInset { 4 };
				Color m_branchLineColor { 80, 80, 80 };
				bool m_showBranchLines { true };
				f32 m_branchLineThickness { 1 };
		};
	}
}
