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
#include <ogfx/gui/widgets/Scrollbar.hpp>

namespace ogfx
{
	namespace gui
	{
		namespace widgets
		{
			class ListView : public ScrollableWidget
			{
				public: class Item : public ostd::__i_stringeable
				{
					public:
						inline Item(ListView& parent) { m_parent = &parent; }
						void setText(const String& text);
						void setFontSize(i32 fontSize);

						inline String toString(void) const override { return m_text; }
						inline bool isValid(void) const { return m_parent != nullptr && m_text.new_trim() != ""; }

						inline Vec2 getDimensions(void) const { return m_dimensions; }
						inline i32 getFontSize(void) const { return m_fontSize; }
						inline String getText(void) const { return m_text; }
						inline bool isSelected(void) const { return m_selected; }

						OSTD_PARAM_GETSET(Color, TextColor, m_textColor);
						OSTD_PARAM_GETSET(Rectangle, Padding, m_padding);
						OSTD_PARAM_GETSET(Color, SelectedTextColor, m_selectedTextColor);
						OSTD_PARAM_GETSET(Color, SelectedColor, m_selectedColor);

					private:
						void update_dimensions(void);
						void set_selected(stdvec<Item*>& selectionList);
						void add_selected(stdvec<Item*>& selectionList);
						void remove_selected(stdvec<Item*>& selectionList);

					private:
						Vec2 m_dimensions { 0, 0 };
						bool m_selected { false };
						ListView* m_parent { nullptr };
						String m_text { "" };
						i32 m_fontSize { 16 };
						Color m_textColor { 30, 30, 30 };
						Color m_selectedTextColor { Colors::White };
						Color m_selectedColor { Colors::Crimson };
						Rectangle m_padding { 5, 5, 20, 0 };

						friend class ListView;
				};
				public:
					inline ListView(WindowCore& window) : ScrollableWidget(window) { create(); }
					ListView& create(void);
					void applyTheme(const ostd::Stylesheet& theme) override;
					void onDraw(ogfx::BasicRenderer2D& gfx) override;
					void afterDraw(ogfx::BasicRenderer2D& gfx) override;
					void onMouseReleased(const Event& event) override;
					Rectangle getContentExtents(void) const override;
					Item& getItem(const String& text);
					Item& getItem(u32 index);
					Item& addLine(const String& text);
					inline void addLine(const Item& line) { m_list.push_back(line); if (m_list.size() == 1) m_list.back().set_selected(m_selectedList); }
					inline stdvec<Item*>& getSelection(void) { return m_selectedList; }
					inline void setSelectionChangedCallback(std::function<void(stdvec<Item*>& selection)> callback) { callback_onSelectionChanged = std::move(callback); }

					OSTD_PARAM_GETSET(Color, SeparatorLineColor, m_lineColor);
					OSTD_BOOL_PARAM_GETSET_E(ShowSeparatorLine, m_showLine);
					OSTD_PARAM_GETSET(i32, DefaultLineFontSize, m_defaultLinefontSize);
					OSTD_PARAM_GETSET(Color, DefaultLineTextColor, m_defaultLinetextColor);
					OSTD_PARAM_GETSET(Rectangle, DefaultLinePadding, m_defaultLinepadding);
					OSTD_PARAM_GETSET(Color, DefaultLineSelectionColor, m_defaultSelectionColor);
					OSTD_PARAM_GETSET(Color, DefaultLineSelectionTextColor, m_defaultSelectionTextColor);

				private:
					Item InvalidItem { *this };
					stdvec<Item> m_list;
					stdvec<Item*> m_selectedList;
					std::function<void(stdvec<Item*>& selection)> callback_onSelectionChanged { nullptr };
					Color m_lineColor { 40, 40, 40 };
					bool m_showLine { true };
					i32 m_defaultLinefontSize { 16 };
					Color m_defaultLinetextColor { 30, 30, 30 };
					Rectangle m_defaultLinepadding { 5, 5, 20, 0 };
					Color m_defaultSelectionColor { Colors::Crimson };
					Color m_defaultSelectionTextColor { Colors::White };
			};
		}
	}
}
