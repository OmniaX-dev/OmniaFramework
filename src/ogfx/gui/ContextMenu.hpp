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

#include <ogfx/gui/widgets/Widget.hpp>

namespace ogfx
{
	namespace gui
	{
		class ContextMenu : public Rectangle
		{
			public: struct Entry
			{
				inline Entry(const String& t, i32 id = -1, const stdvec<Entry>& sub = {}) { text = t; this->id = id; submenus = sub; }
				inline Entry(const String& t, const stdvec<Entry>& sub) { text = t; id = -1; submenus = sub; }
				String text { "" };
				i32 id { -1 };
				stdvec<Entry> submenus;

				private:
					Vec2 size;
					void update_size(BasicRenderer2D& gfx, i32 fontSize);
					friend class ContextMenu;
			};
			public: struct Instance
			{
				using Callback = std::function<void(const Entry&)>;
				stdvec<Entry> entries;
				Callback onActivate { nullptr };
				f32 minWidth { 0 };

				// Remove first entry whose text matches. Returns true if something was removed.
				inline bool removeByText(const String& text, bool recursive = true)
				{
					return remove_if(entries, [&](const Entry& e) { return e.text == text; }, recursive);
				}
				// Remove first entry whose id matches. ID -1 is treated as "no id" and is skipped.
				inline bool removeById(i32 id, bool recursive = true)
				{
					if (id < 0) return false;
					return remove_if(entries, [&](const Entry& e) { return e.id == id; }, recursive);
				}
				// Remove ALL matches (handy if you reuse text/ids across submenus).
				inline size_t removeAllByText(const String& text, bool recursive = true)
				{
					return remove_all_if(entries, [&](const Entry& e) { return e.text == text; }, recursive);
				}
				inline size_t removeAllById(i32 id, bool recursive = true)
				{
					if (id < 0) return 0;
					return remove_all_if(entries, [&](const Entry& e) { return e.id == id; }, recursive);
				}

			private:
				template <typename Pred>
				static bool remove_if(stdvec<Entry>& list, Pred pred, bool recursive)
				{
					for (auto it = list.begin(); it != list.end(); ++it)
					{
						if (pred(*it)) { list.erase(it); return true; }
					}
					if (!recursive) return false;
					for (auto& e : list)
					{
						if (remove_if(e.submenus, pred, true)) return true;
					}
					return false;
				}
				template <typename Pred>
				static size_t remove_all_if(stdvec<Entry>& list, Pred pred, bool recursive)
				{
					size_t count = 0;
					for (auto it = list.begin(); it != list.end(); )
					{
						if (pred(*it)) { it = list.erase(it); ++count; }
						else ++it;
					}
					if (recursive)
					{
						for (auto& e : list)
							count += remove_all_if(e.submenus, pred, true);
					}
					return count;
				}
			};
			private: struct Panel
			{
				stdvec<Entry>* entries { nullptr };
				Vec2 position { 0, 0 };
				Vec2 size { 0, 0 };
				f32 entryHeight { 0 };
				i32 hoveredIndex { -1 };
				i32 openedSubmenuIndex { -1 };
			};
			public:
				inline ContextMenu(WindowCore& window) : m_window(window) { create(); }
				ContextMenu& create(void);
				void applyTheme(const ostd::Stylesheet& theme);
				void draw(BasicRenderer2D& gfx);
				void update(void);
				void onMouseReleased(const Event& event);
				void onMouseMoved(const Event& event);
				void onMousePressed(const Event& event);
				void onMouseScrolled(const Event& event);
				void show(const Vec2& pos);
				void hide(void);
				void setInstance(const Instance& instance);
				void setFontSize(i32 size);

				inline bool isVisible(void) const { return m_visible; }
				inline i32 getFontSize(void) const { return m_fontSize; }

				OSTD_PARAM_GETSET(Rectangle, Padding, m_padding);
				OSTD_PARAM_GETSET(f32, ItemSpacing, m_spacing);
				OSTD_PARAM_GETSET(Color, BackgroundColor, m_backgroundColor);
				OSTD_PARAM_GETSET(Color, SelectionColor, m_selectionColor);
				OSTD_PARAM_GETSET(Color, SelectionTextColor, m_selectionTextColor);
				OSTD_PARAM_GETSET(Color, SeparatorLineColor, m_separatorLineColor);
				OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
				OSTD_PARAM_GETSET(Color, SubmenuIndicatorColor, m_submenuIndicatorColor);
				OSTD_PARAM_GETSET(Color, BorderColor, m_borderColor);
				OSTD_PARAM_GETSET(ColorGradient, SelectionGradient, m_selectionGradient);
				OSTD_BOOL_PARAM_GETSET_E(SelectionGradient, m_useSelectionGradient);

			private:
				void push_panel(stdvec<Entry>& entries, const Vec2& anchorTopLeft, bool flipLeft = false);
				void pop_to_depth(size_t depth);
				void compute_panel_size(Panel& panel);
				i32  entry_index_at(const Panel& panel, const Vec2& mousePos) const;
				Rectangle entry_rect(const Panel& panel, i32 index) const;
				void draw_panel(BasicRenderer2D& gfx, const Panel& panel);
				void relayout_panels(void);

			private:
				WindowCore& m_window;
				bool m_visible { false };
				Instance m_data;
				f32 m_entryHeight { 0 };
				Vec2 m_mousePos { 0, 0 };

				stdvec<Panel> m_panels;

				i32 m_pendingOpenPanelDepth { -1 };
				i32 m_pendingOpenEntryIndex { -1 };
				ostd::Timer m_hoverOpenTimer;
				ostd::Timer m_hoverCloseTimer;
				bool m_hoverOpenTimerActive { false };
				bool m_hoverCloseTimerActive { false };

				Rectangle m_padding { 16, 0, 16, 0 };
				f32 m_spacing { 8 };
				i32 m_fontSize { 18 };
				Color m_backgroundColor { "#6B0A1DFF" };
				Color m_selectionColor { "#DC143CFF" };
				Color m_selectionTextColor { "#F16A85FF" };
				Color m_separatorLineColor { "#700000FF" };
				Color m_textColor { "#F16A85FF" };
				Color m_submenuIndicatorColor { "#111111FF" };
				Color m_borderColor { "#400000FF" };
				bool m_useSelectionGradient { true };
				ColorGradient m_selectionGradient { { "#C21135FF", "#820B23FF" }, { 1.0f } };

				static constexpr u64 SubmenuOpenDelayMs  { 300 };
				static constexpr u64 SubmenuCloseDelayMs { 50 };
				friend class Instance;
		};
	}
}
