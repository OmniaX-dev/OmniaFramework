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

#include "HexEditor.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../../utils/Keycodes.hpp"
#include "../Window.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ostd/math/Random.hpp>

namespace ogfx
{
	namespace gui
	{
		static u64 now_ms(void)
		{
			using namespace std::chrono;
			return cast<u64>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
		}

		static constexpr u64 DoubleClickMs { 400 };
		static constexpr u8 UnitSizes[4] { 1, 2, 4, 8 };

		static i64 sign_extend(u64 value, u8 size)
		{
			if (size >= 8)
				return cast<i64>(value);
			const u32 bits = size * 8;
			const u64 sign = 1ull << (bits - 1);
			return cast<i64>((value ^ sign) - sign);
		}

		static u64 size_mask(u8 size)
		{
			return size >= 8 ? ~0ull : ((1ull << (size * 8)) - 1);
		}

		static String hex_string(u64 value, i32 digits)
		{
			char buf[32];
			std::snprintf(buf, sizeof(buf), "%0*llX", digits, cast<unsigned long long>(value));
			return String(buf);
		}

		static String binary_string(u64 value, u8 size, bool grouped)
		{
			String out;
			for (i32 bit = size * 8 - 1; bit >= 0; bit--)
			{
				out.addChar(((value >> bit) & 1) ? '1' : '0');
				if (grouped && bit > 0 && bit % 8 == 0)
					out.addChar(' ');
			}
			return out;
		}

		static String float_string(f64 value, i32 precision)
		{
			char buf[64];
			std::snprintf(buf, sizeof(buf), "%.*g", precision, value);
			return String(buf);
		}

		// Only '0' and '1', up to `maxDigits`.
		class BinaryFilter : public TextEdit::CharacterFilter
		{
			public:
				inline BinaryFilter(i32 maxDigits) : m_maxDigits(maxDigits) {  }
				inline bool isValidChar(const String& utf8, const ostd::TextBuffer& buffer) override
				{
					const String result = predictResult(utf8, buffer);
					if (m_maxDigits >= 0 && cast<i32>(result.len()) > m_maxDigits)
						return false;
					for (char c : result.cpp_str())
					{
						if (c != '0' && c != '1')
							return false;
					}
					return true;
				}
				inline void setMaxDigits(i32 maxDigits) { m_maxDigits = maxDigits; }
			private:
				i32 m_maxDigits;
		};

		// Space releases trigger ActionPerformed on the focused widget too; text fields
		// only want Return.
		static bool is_space_action(const Event& event)
		{
			return event.keyboard != nullptr && event.keyboard->keyCode == KeyCode::Space;
		}



		// ===================================================================
		// HexEditor
		// ===================================================================
		HexEditor::HexEditor(Window& window) : Widget({ 0, 0, 600, 400 }, window),
			m_topBar(window), m_navRow(window), m_optRow(window),
			m_addrLabel(window, "Address"), m_addrInput(window), m_goButton(window, "Go"), m_backButton(window, "Back"),
			m_up10Button(window, "-10"), m_upButton(window, "-1"), m_downButton(window, "+1"), m_down10Button(window, "+10"),
			m_sizeLabel(window, "Size"),
			m_sizeButtons { { window, "1" }, { window, "2" }, { window, "4" }, { window, "8" } },
			m_endianButton(window, "LE"), m_alignCheck(window), m_followCheck(window), m_flashCheck(window),
			m_watchButton(window, "+ Watch"), m_watchListButton(window, "Watchlist (0)"), m_regionsButton(window, "Regions"), m_aliasesButton(window, "Aliases"), m_searchButton(window, "Search"),
			m_editWindow(window, "Edit value"), m_editBox(window), m_editInfo(window), m_editCurrent(window), m_editFormatRow(window),
			m_editFormatButtons { { window, "Dec" }, { window, "Hex" }, { window, "Bin" } },
			m_editInput(window), m_editStatus(window), m_editButtonRow(window), m_editApply(window, "Apply"), m_editCancel(window, "Cancel"),
			m_watchWindow(window, "Watchlist"), m_watchBox(window), m_watchList(window, *this),
			m_watchEditRow(window), m_watchNameLabel(window, "Name"), m_watchNameInput(window), m_watchRename(window, "Rename"),
			m_watchColorLabel(window, "Color"), m_watchColorButton(window), m_watchButtonRow(window),
			m_watchJump(window, "Jump to"), m_watchRemove(window, "Remove"), m_watchClear(window, "Clear all"),
			m_regionWindow(window, "Regions"), m_regionBox(window), m_regionList(window, *this), m_regionButtonRow(window), m_regionJump(window, "Jump to"),
			m_aliasWindow(window, "Aliases"), m_aliasBox(window), m_aliasList(window, *this), m_aliasEditRow(window),
			m_aliasPatternLabel(window, "Pattern"), m_aliasPatternInput(window), m_aliasNameLabel(window, "Alias"), m_aliasNameInput(window),
			m_aliasColorButton(window), m_aliasButtonRow(window), m_aliasFromSelection(window, "From selection"), m_aliasAdd(window, "Add"),
			m_aliasUpdate(window, "Update"), m_aliasRemove(window, "Remove"), m_aliasClear(window, "Clear all"), m_aliasFind(window, "Find next"),
			m_aliasStatus(window),
			m_searchWindow(window, "Search"), m_searchBox(window), m_searchModeRow(window),
			m_searchModeButtons { { window, "Hex" }, { window, "ASCII" } }, m_searchCaseCheck(window), m_searchInput(window),
			m_searchButtonRow(window), m_searchPrev(window, "Find previous"), m_searchNext(window, "Find next"),
			m_searchAll(window, "Find all"), m_searchClear(window, "Clear"), m_searchStatus(window), m_searchList(window, *this)
		{
			setTypeName("ogfx::gui::HexEditor");
			setStylesheetCategoryName("hexEditor");
			enableFocus();
			enableBackground();
			setBackgroundColor({ 25, 25, 28 });
			setPadding({ 4, 4, 4, 4 });

			m_hexStrings.reserve(256);
			m_asciiStrings.reserve(256);
			for (i32 i = 0; i < 256; i++)
			{
				m_hexStrings.push_back(hex_string(cast<u64>(i), 2));
				String a;
				a.addChar((i >= 32 && i < 127) ? cast<char>(i) : '.');
				m_asciiStrings.push_back(a);
			}

			build_top_bar();
			build_edit_window();
			build_watch_window();
			build_region_window();
			build_search_window();
			m_nextWatchHue = ostd::Random::getf32(0.0f, 1.0f);
			build_alias_window();
			update_toggles();
			validate();
		}

		HexEditor::~HexEditor(void)
		{
		}

		void HexEditor::build_top_bar(void)
		{
			addWidget(m_topBar);
			m_topBar.setAnchorPreset(eAnchorPreset::TopWide, ePresetResizeMode::KeepWidth);
			m_topBar.setSpacing(3);
			m_topBar.addWidget(m_navRow);
			m_topBar.addWidget(m_optRow);
			m_navRow.setSpacing(4);
			m_optRow.setSpacing(6);

			m_navRow.addWidget(m_addrLabel);
			m_addrInput.useHexadecimalFilter(16);
			m_addrInput.setMinSize(170, 0);
			auto l_go = [this](void) {
				u64 address = 0;
				if (!parseValue(String("0x") + m_addrInput.getText().new_trim(), eFormat::Hex, 8, address) || !jumpTo(address))
					show_message("Invalid address: " + m_addrInput.getText());
			};
			m_addrInput.setCallback(Widget::eCallback::ActionPerformed, [l_go](const Event& event) { if (!is_space_action(event)) l_go(); });
			m_navRow.addWidget(m_addrInput);
			m_goButton.setCallback(Widget::eCallback::ActionPerformed, [l_go](const Event&) { l_go(); });
			m_navRow.addWidget(m_goButton);
			m_backButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { goBack(); });
			m_navRow.addWidget(m_backButton);
			m_up10Button.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { scrollRows(-10); });
			m_upButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { scrollRows(-1); });
			m_downButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { scrollRows(1); });
			m_down10Button.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { scrollRows(10); });
			for (Button* b : { &m_up10Button, &m_upButton, &m_downButton, &m_down10Button })
				m_navRow.addWidget(*b);
			m_searchButton.setTooltipText("Search (Ctrl+F, F3 / Shift+F3)");
			m_searchButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { openSearchWindow(); });
			m_navRow.addWidget(m_searchButton);
			m_regionsButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { openRegionList(); });
			m_navRow.addWidget(m_regionsButton);
			m_aliasesButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { openAliasList(); });
			m_navRow.addWidget(m_aliasesButton);

			m_optRow.addWidget(m_sizeLabel);
			for (i32 i = 0; i < 4; i++)
			{
				m_sizeButtons[i].addThemeID("hex_toggle");
				m_sizeButtons[i].setMinSize(28, 0);
				m_sizeButtons[i].setCallback(Widget::eCallback::ActionPerformed, [this, i](const Event&) { setUnitSize(UnitSizes[i]); });
				m_optRow.addWidget(m_sizeButtons[i]);
			}
			m_endianButton.addThemeID("hex_toggle");
			m_endianButton.setMinSize(36, 0);
			m_endianButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { setBigEndian(!m_bigEndian); });
			m_optRow.addWidget(m_endianButton);
			m_alignCheck.setText("Align");
			m_alignCheck.setStateChangedCallback([this](CheckBox&, bool checked) { enableAlign(checked); });
			m_optRow.addWidget(m_alignCheck);
			m_followCheck.setText("Follow");
			m_followCheck.setStateChangedCallback([this](CheckBox&, bool checked) { enableFollowHighlight(checked); });
			m_optRow.addWidget(m_followCheck);
			m_flashCheck.setText("Flash");
			m_flashCheck.setChecked(m_changeFlash);
			m_flashCheck.setStateChangedCallback([this](CheckBox&, bool checked) { enableChangeFlash(checked); });
			m_optRow.addWidget(m_flashCheck);
			m_watchButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				if (hasSelection())
					addWatch(getSelection(), m_unitSize);
				else
					show_message("Select a value to watch first");
			});
			m_optRow.addWidget(m_watchButton);
			m_watchListButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { openWatchList(); });
			m_optRow.addWidget(m_watchListButton);

			// Compact styling for everything in the bar: (@hex_bar label), (@hex_bar button), ...
			for (Widget* w : { (Widget*)&m_addrLabel, (Widget*)&m_addrInput, (Widget*)&m_goButton, (Widget*)&m_backButton,
							   (Widget*)&m_up10Button, (Widget*)&m_upButton, (Widget*)&m_downButton, (Widget*)&m_down10Button,
							   (Widget*)&m_sizeLabel, (Widget*)&m_endianButton, (Widget*)&m_alignCheck, (Widget*)&m_followCheck,
							   (Widget*)&m_flashCheck, (Widget*)&m_watchButton, (Widget*)&m_watchListButton,
							   (Widget*)&m_regionsButton, (Widget*)&m_aliasesButton, (Widget*)&m_searchButton })
				w->addThemeID("hex_bar");
			for (auto& b : m_sizeButtons)
				b.addThemeID("hex_bar");
		}

		void HexEditor::build_edit_window(void)
		{
			m_editWindow.setModal();
			m_editWindow.setButtons(SubWindow::Buttons::Close);
			m_editWindow.disableResizable();
			m_editWindow.setBounds({ 0, 0, 560, 290 });
			m_editWindow.addWidget(m_editBox);
			m_editBox.setAnchorPreset(eAnchorPreset::FullRect);
			m_editBox.setSpacing(6);
			m_editBox.addWidget(m_editInfo);
			m_editBox.addWidget(m_editCurrent);
			m_editBox.addWidget(m_editFormatRow);
			m_editFormatRow.setSpacing(4);
			for (i32 i = 0; i < 3; i++)
			{
				m_editFormatButtons[i].addThemeID("hex_toggle");
				m_editFormatButtons[i].setMinSize(60, 0);
				m_editFormatButtons[i].setCallback(Widget::eCallback::ActionPerformed, [this, i](const Event&) { set_edit_format(cast<eFormat>(i)); });
				m_editFormatRow.addWidget(m_editFormatButtons[i]);
			}
			m_editInput.setCallback(Widget::eCallback::ActionPerformed, [this](const Event& event) { if (!is_space_action(event)) apply_edit(); });
			m_editBox.addWidget(m_editInput);
			m_editBox.addWidget(m_editStatus);
			m_editButtonRow.setSpacing(6);
			m_editButtonRow.setAlignment(eBoxAlignment::End);
			m_editApply.setMinSize(90, 0);
			m_editCancel.setMinSize(90, 0);
			m_editApply.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { apply_edit(); });
			m_editCancel.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { m_editWindow.close(); });
			m_editButtonRow.addWidget(m_editApply);
			m_editButtonRow.addWidget(m_editCancel);
			m_editBox.addWidget(m_editButtonRow);
			m_binaryFilter = std::make_unique<BinaryFilter>(8);
		}

		void HexEditor::build_watch_window(void)
		{
			m_watchWindow.setBounds({ 0, 0, 760, 380 });
			m_watchWindow.addWidget(m_watchBox);
			m_watchBox.setAnchorPreset(eAnchorPreset::FullRect);
			m_watchBox.setSpacing(6);

			m_watchList.setColumns({ "Label", "Address", "Size", "Hex", "Dec" });
			m_watchList.setEmptyText("No watches: select a value and press \"+ Watch\"");
			m_watchList.setRowCountFunction([this](void) { return cast<i32>(m_watches.size()); });
			m_watchList.setCellFunction([this](i32 row, i32 column) -> String {
				const Watch& w = m_watches[row];
				switch (column)
				{
					case 0: return w.label;
					case 1: return formatAddress(m_baseAddress + w.index);
					case 2: return String("").add(cast<i32>(w.size)).add(" B");
					case 3: return String("0x") + hex_string(w.value, w.size * 2);
					default: return String(std::to_string(w.value));
				}
			});
			m_watchList.setCellColorFunction([this](i32 row, i32 column) {
				return column == 0 ? m_watches[row].color : (column == 1 ? m_addressColor : m_statusTextColor);
			});
			m_watchList.setSwatchFunction([this](i32 row) { return m_watches[row].color; });
			m_watchList.setSelectCallback([this](i32 row) { select_watch_row(row); });
			m_watchList.setActivateCallback([this](i32 row) { jumpTo(m_baseAddress + m_watches[row].index); });
			m_watchList.setRemoveCallback([this](i32 row) { removeWatch(row); });
			m_watchList.setSwatchCallback([this](i32 row) {
				m_watchList.setSelectedRow(row, true);
				m_watchColorButton.openPopup();
			});
			m_watchList.setVSizeFlags(eSizeFlags::ExpandFill);
			m_watchBox.addWidget(m_watchList);

			// Rename / recolor the selected watch.
			m_watchEditRow.setSpacing(6);
			auto l_rename = [this](void) {
				const i32 row = m_watchList.getSelectedRow();
				if (!setWatchLabel(row, m_watchNameInput.getText().new_trim()))
					show_message("Select a watch to rename first");
			};
			m_watchNameInput.setHSizeFlags(eSizeFlags::ExpandFill);
			m_watchNameInput.setMinSize(160, 0);
			m_watchNameInput.setCallback(Widget::eCallback::ActionPerformed, [l_rename](const Event& event) { if (!is_space_action(event)) l_rename(); });
			m_watchRename.setCallback(Widget::eCallback::ActionPerformed, [l_rename](const Event&) { l_rename(); });
			m_watchColorButton.setMinSize(48, 0);
			m_watchColorButton.setColorChangedCallback([this](const Color& color) { setWatchColor(m_watchList.getSelectedRow(), color); });
			for (Widget* w : { (Widget*)&m_watchNameLabel, (Widget*)&m_watchNameInput, (Widget*)&m_watchRename, (Widget*)&m_watchColorLabel, (Widget*)&m_watchColorButton })
			{
				w->addThemeID("hex_bar");
				m_watchEditRow.addWidget(*w);
			}
			m_watchBox.addWidget(m_watchEditRow);

			m_watchButtonRow.setSpacing(6);
			m_watchButtonRow.setAlignment(eBoxAlignment::End);
			m_watchJump.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				const i32 row = m_watchList.getSelectedRow();
				if (row >= 0 && row < cast<i32>(m_watches.size()))
					jumpTo(m_baseAddress + m_watches[row].index);
			});
			m_watchRemove.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { removeWatch(m_watchList.getSelectedRow()); });
			m_watchClear.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { clearWatches(); });
			m_watchButtonRow.addWidget(m_watchJump);
			m_watchButtonRow.addWidget(m_watchRemove);
			m_watchButtonRow.addWidget(m_watchClear);
			m_watchBox.addWidget(m_watchButtonRow);
		}

		void HexEditor::build_region_window(void)
		{
			m_regionWindow.setBounds({ 0, 0, 640, 360 });
			m_regionWindow.addWidget(m_regionBox);
			m_regionBox.setAnchorPreset(eAnchorPreset::FullRect);
			m_regionBox.setSpacing(6);
			m_regionList.setColumns({ "Name", "Start", "End", "Size" });
			m_regionList.setEmptyText("No regions (see HexEditor::addHighlight())");
			m_regionList.setRowCountFunction([this](void) { return cast<i32>(m_highlights.size()); });
			m_regionList.setCellFunction([this](i32 row, i32 column) -> String {
				const Highlight& h = m_highlights[row];
				switch (column)
				{
					case 0: return h.label == "" ? String("(unnamed)") : h.label;
					case 1: return formatAddress(m_baseAddress + h.index);
					case 2: return formatAddress(m_baseAddress + h.index + h.size - 1);
					default: return String(std::to_string(h.size)).add(" B");
				}
			});
			m_regionList.setCellColorFunction([this](i32 row, i32 column) { return column == 1 || column == 2 ? m_addressColor : m_statusTextColor; });
			m_regionList.setSwatchFunction([this](i32 row) { return m_highlights[row].color; });
			m_regionList.setActivateCallback([this](i32 row) { jumpTo(m_baseAddress + m_highlights[row].index); });
			m_regionList.setVSizeFlags(eSizeFlags::ExpandFill);
			m_regionBox.addWidget(m_regionList);
			m_regionButtonRow.setAlignment(eBoxAlignment::End);
			m_regionJump.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				const i32 row = m_regionList.getSelectedRow();
				if (row >= 0 && row < cast<i32>(m_highlights.size()))
					jumpTo(m_baseAddress + m_highlights[row].index);
			});
			m_regionButtonRow.addWidget(m_regionJump);
			m_regionBox.addWidget(m_regionButtonRow);
		}

		void HexEditor::build_search_window(void)
		{
			m_searchWindow.setBounds({ 0, 0, 660, 460 });
			m_searchWindow.addWidget(m_searchBox);
			m_searchBox.setAnchorPreset(eAnchorPreset::FullRect);
			m_searchBox.setSpacing(6);

			m_searchModeRow.setSpacing(4);
			for (i32 i = 0; i < 2; i++)
			{
				m_searchModeButtons[i].addThemeID("hex_toggle");
				m_searchModeButtons[i].setMinSize(64, 0);
				m_searchModeButtons[i].setCallback(Widget::eCallback::ActionPerformed, [this, i](const Event&) { set_search_mode(cast<eSearchMode>(i)); });
				m_searchModeRow.addWidget(m_searchModeButtons[i]);
			}
			m_searchCaseCheck.setText("Match case");
			m_searchCaseCheck.setChecked(true);
			m_searchModeRow.addWidget(m_searchCaseCheck);
			m_searchBox.addWidget(m_searchModeRow);

			m_searchInput.setCallback(Widget::eCallback::ActionPerformed, [this](const Event& event) {
				if (!is_space_action(event))
					search_window_find(event.keyboard != nullptr && event.keyboard->modifiers.shiftOnly());
			});
			m_searchBox.addWidget(m_searchInput);

			m_searchButtonRow.setSpacing(6);
			m_searchPrev.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { search_window_find(true); });
			m_searchNext.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { search_window_find(false); });
			m_searchAll.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				if (!apply_search_input())
					return;
				findAll();
				m_searchList.resetColumnWidths();
				m_searchList.setSelectedRow(-1);
				update_search_status();
			});
			m_searchClear.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				clearSearch();
				m_searchInput.setText("");
				update_search_status();
			});
			for (Button* b : { &m_searchPrev, &m_searchNext, &m_searchAll, &m_searchClear })
				m_searchButtonRow.addWidget(*b);
			m_searchBox.addWidget(m_searchButtonRow);
			m_searchBox.addWidget(m_searchStatus);

			m_searchList.setColumns({ "#", "Address", "Bytes" });
			m_searchList.setEmptyText("\"Find all\" lists every match here");
			m_searchList.setRowCountFunction([this](void) { return cast<i32>(m_searchResults.size()); });
			m_searchList.setCellFunction([this](i32 row, i32 column) -> String {
				const StreamIndex index = m_searchResults[row];
				if (column == 0)
					return String(std::to_string(row + 1));
				if (column == 1)
					return formatAddress(m_baseAddress + index);
				String bytes;
				const u64 end = std::min<u64>(cast<u64>(index) + std::min<u32>(getSearchPatternLength(), 16), m_data.size());
				for (u64 i = index; i < end; i++)
					bytes += m_hexStrings[cast<u8>(m_data[i])] + " ";
				return bytes;
			});
			m_searchList.setCellColorFunction([this](i32 row, i32 column) {
				if (column == 1)
					return m_addressColor;
				return match_at(m_searchResults[row]) ? m_statusTextColor : m_zeroColor; // stale matches are dimmed
			});
			m_searchList.setSelectCallback([this](i32 row) {
				const StreamIndex index = m_searchResults[row];
				m_searchCurrent = index;
				select(index);
				ensureVisible(index, std::max<u32>(1, getSearchPatternLength()));
				update_search_status();
			});
			m_searchList.setActivateCallback([this](i32 row) { jumpTo(m_baseAddress + m_searchResults[row]); m_searchCurrent = m_searchResults[row]; });
			m_searchList.setVSizeFlags(eSizeFlags::ExpandFill);
			m_searchBox.addWidget(m_searchList);
			set_search_mode(eSearchMode::Hex);
		}

		void HexEditor::build_alias_window(void)
		{
			m_aliasWindow.setBounds({ 0, 0, 780, 420 });
			m_aliasWindow.addWidget(m_aliasBox);
			m_aliasBox.setAnchorPreset(eAnchorPreset::FullRect);
			m_aliasBox.setSpacing(6);

			m_aliasList.setColumns({ "Pattern", "Alias", "Bytes" });
			m_aliasList.setEmptyText("No aliases: type a pattern (e.g. \"B1\" or \"F1 ??\") and a name, then Add");
			m_aliasList.setRowCountFunction([this](void) { return cast<i32>(m_aliases.size()); });
			m_aliasList.setCellFunction([this](i32 row, i32 column) -> String {
				const Alias& a = m_aliases[row];
				if (column == 0)
					return a.pattern;
				if (column == 1)
					return a.name;
				return String("").add(cast<i32>(a.bytes.size()));
			});
			m_aliasList.setCellColorFunction([this](i32 row, i32 column) { return column == 1 ? m_aliases[row].color : m_statusTextColor; });
			m_aliasList.setSwatchFunction([this](i32 row) { return m_aliases[row].color; });
			m_aliasList.setSelectCallback([this](i32 row) { select_alias_row(row); });
			m_aliasList.setSwatchCallback([this](i32 row) {
				m_aliasList.setSelectedRow(row, true);
				m_aliasColorButton.openPopup();
			});
			// Double-click: next occurrence of the pattern.
			m_aliasList.setActivateCallback([this](i32 row) {
				setSearchPattern(m_aliases[row].pattern, eSearchMode::Hex);
				findNext();
			});
			m_aliasList.setRemoveCallback([this](i32 row) { removeAlias(m_aliases[row].id); });
			m_aliasList.setVSizeFlags(eSizeFlags::ExpandFill);
			m_aliasBox.addWidget(m_aliasList);

			m_aliasEditRow.setSpacing(6);
			m_aliasPatternInput.setMinSize(150, 0);
			m_aliasNameInput.setMinSize(150, 0);
			m_aliasNameInput.setHSizeFlags(eSizeFlags::ExpandFill);
			m_aliasColorButton.setMinSize(48, 0);
			m_aliasColorButton.setColor(next_watch_color());
			// Live recolor of the selected alias (Add uses the button's color too).
			m_aliasColorButton.setColorChangedCallback([this](const Color& color) {
				const i32 row = m_aliasList.getSelectedRow();
				if (row >= 0 && row < cast<i32>(m_aliases.size()))
					m_aliases[row].color = color;
			});
			for (Widget* w : { (Widget*)&m_aliasPatternLabel, (Widget*)&m_aliasPatternInput, (Widget*)&m_aliasNameLabel, (Widget*)&m_aliasNameInput, (Widget*)&m_aliasColorButton })
			{
				w->addThemeID("hex_bar");
				m_aliasEditRow.addWidget(*w);
			}
			m_aliasBox.addWidget(m_aliasEditRow);

			auto l_add = [this](void) {
				const i32 id = addAlias(m_aliasPatternInput.getText(), m_aliasNameInput.getText().new_trim(), m_aliasColorButton.getColor());
				if (id < 0)
				{
					set_alias_message(m_aliasNameInput.getText().new_trim() == "" ? "Give the alias a name" : "Invalid pattern (hex bytes, ?? = any byte)", true);
					return;
				}
				m_aliasList.setSelectedRow(-1);
				m_aliasColorButton.setColor(next_watch_color());	// fresh color for the next one
				m_aliasPatternInput.setText("");
				m_aliasNameInput.setText("");
				set_alias_message(String("Added \"").add(m_aliases.back().name).add("\""), false);
			};
			m_aliasNameInput.setCallback(Widget::eCallback::ActionPerformed, [l_add](const Event& event) { if (!is_space_action(event)) l_add(); });
			m_aliasAdd.setCallback(Widget::eCallback::ActionPerformed, [l_add](const Event&) { l_add(); });
			m_aliasFromSelection.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				if (!hasSelection())
				{
					set_alias_message("Select some bytes in the editor first", true);
					return;
				}
				// The selected bytes in memory order (not the value's endianness).
				String pattern;
				const u64 end = std::min<u64>(getSelection() + m_unitSize, m_data.size());
				for (u64 i = getSelection(); i < end; i++)
					pattern += (pattern == "" ? String("") : String(" ")) + m_hexStrings[cast<u8>(m_data[i])];
				m_aliasPatternInput.setText(pattern);
				getWindow().getFocusManager().requestFocus(m_aliasNameInput);
			});
			m_aliasUpdate.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				const i32 row = m_aliasList.getSelectedRow();
				if (row < 0 || row >= cast<i32>(m_aliases.size()))
				{
					set_alias_message("Select an alias to update", true);
					return;
				}
				if (!updateAlias(m_aliases[row].id, m_aliasPatternInput.getText(), m_aliasNameInput.getText().new_trim(), m_aliasColorButton.getColor()))
					set_alias_message("Invalid pattern or empty name", true);
				else
					set_alias_message("Updated", false);
			});
			m_aliasRemove.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				const i32 row = m_aliasList.getSelectedRow();
				if (row >= 0 && row < cast<i32>(m_aliases.size()))
					removeAlias(m_aliases[row].id);
			});
			m_aliasClear.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { clearAliases(); });
			m_aliasFind.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) {
				const i32 row = m_aliasList.getSelectedRow();
				if (row < 0 || row >= cast<i32>(m_aliases.size()))
					return;
				setSearchPattern(m_aliases[row].pattern, eSearchMode::Hex);
				if (findNext() < 0)
					set_alias_message("No occurrence in the data", true);
			});
			m_aliasButtonRow.setSpacing(6);
			for (Button* b : { &m_aliasFromSelection, &m_aliasAdd, &m_aliasUpdate, &m_aliasRemove, &m_aliasClear, &m_aliasFind })
				m_aliasButtonRow.addWidget(*b);
			m_aliasBox.addWidget(m_aliasButtonRow);
			m_aliasBox.addWidget(m_aliasStatus);
			set_alias_message("Longest match wins: with \"B1\" and \"B1 50\", the bytes B1 50 are one \"B1 50\" token", false);
		}

		void HexEditor::openAliasList(void)
		{
			const bool wasOpen = m_aliasWindow.isVisible() && m_aliasWindow.getParent() != nullptr;
			m_aliasList.resetColumnWidths();
			m_aliasWindow.open();
			if (!wasOpen)
				m_aliasWindow.centerInParent();
		}

		void HexEditor::select_alias_row(i32 row)
		{
			if (row < 0 || row >= cast<i32>(m_aliases.size()))
				return;
			m_aliasPatternInput.setText(m_aliases[row].pattern);
			m_aliasNameInput.setText(m_aliases[row].name);
			m_aliasColorButton.setColor(m_aliases[row].color);
		}

		void HexEditor::set_alias_message(const String& message, bool error)
		{
			m_aliasStatus.setText(message);
			m_aliasStatus.setTextColor(error ? m_highlightColor : m_statusLabelColor);
		}

		i32 HexEditor::addAlias(const String& pattern, const String& name, const Color& color)
		{
			Alias a;
			if (name == "" || !parse_hex_pattern(pattern, a.bytes))
				return -1;
			a.id = m_nextAliasId++;
			a.pattern = pattern.new_trim();
			a.name = name;
			a.color = (color.a == 0 ? next_watch_color() : color);
			for (i16 b : a.bytes)
				a.wildcards += (b < 0 ? 1 : 0);
			if (a.wildcards == a.bytes.size())
				return -1; // all wildcards would match everything
			m_aliases.push_back(a);
			rebuild_alias_index();
			m_aliasList.resetColumnWidths();
			return a.id;
		}

		bool HexEditor::updateAlias(i32 id, const String& pattern, const String& name, const Color& color)
		{
			for (auto& a : m_aliases)
			{
				if (a.id != id)
					continue;
				stdvec<i16> bytes;
				if (name == "" || !parse_hex_pattern(pattern, bytes))
					return false;
				u32 wildcards = 0;
				for (i16 b : bytes)
					wildcards += (b < 0 ? 1 : 0);
				if (wildcards == bytes.size())
					return false;
				a.bytes = bytes;
				a.wildcards = wildcards;
				a.pattern = pattern.new_trim();
				a.name = name;
				a.color = color;
				rebuild_alias_index();
				m_aliasList.resetColumnWidths();
				return true;
			}
			return false;
		}

		bool HexEditor::removeAlias(i32 id)
		{
			const bool removed = std::erase_if(m_aliases, [id](const Alias& a) { return a.id == id; }) > 0;
			if (removed)
			{
				rebuild_alias_index();
				if (m_aliasList.getSelectedRow() >= cast<i32>(m_aliases.size()))
					m_aliasList.setSelectedRow(cast<i32>(m_aliases.size()) - 1, true);
			}
			return removed;
		}

		void HexEditor::clearAliases(void)
		{
			m_aliases.clear();
			m_aliasList.setSelectedRow(-1);
			rebuild_alias_index();
		}

		void HexEditor::rebuild_alias_index(void)
		{
			for (auto& bucket : m_aliasByFirstByte)
				bucket.clear();
			m_aliasWildFirst.clear();
			m_maxAliasLength = 0;
			// Best first: longest, then fewest wildcards, then oldest.
			stdvec<i32> order(m_aliases.size());
			for (i32 i = 0; i < cast<i32>(order.size()); i++)
				order[i] = i;
			std::stable_sort(order.begin(), order.end(), [this](i32 l, i32 r) {
				const Alias& a = m_aliases[l];
				const Alias& b = m_aliases[r];
				if (a.bytes.size() != b.bytes.size())
					return a.bytes.size() > b.bytes.size();
				return a.wildcards < b.wildcards;
			});
			for (i32 i : order)
			{
				const Alias& a = m_aliases[i];
				m_maxAliasLength = std::max<u32>(m_maxAliasLength, cast<u32>(a.bytes.size()));
				if (a.bytes[0] < 0)
					m_aliasWildFirst.push_back(i);
				else
					m_aliasByFirstByte[a.bytes[0]].push_back(i);
			}
		}

		const HexEditor::Alias* HexEditor::alias_match_at(u64 index) const
		{
			if (index >= m_data.size() || m_aliases.empty())
				return nullptr;
			// Both lists are sorted best first: the first match in each is its best.
			const Alias* best = nullptr;
			for (i32 i : m_aliasByFirstByte[cast<u8>(m_data[index])])
			{
				if (match_pattern(index, m_aliases[i].bytes, false))
				{
					best = &m_aliases[i];
					break;
				}
			}
			for (i32 i : m_aliasWildFirst)
			{
				const Alias& a = m_aliases[i];
				if (best && (a.bytes.size() < best->bytes.size() || (a.bytes.size() == best->bytes.size() && a.wildcards >= best->wildcards)))
					break; // can't beat it
				if (match_pattern(index, a.bytes, false))
				{
					best = &a;
					break;
				}
			}
			return best;
		}

		void HexEditor::resolve_aliases(u64 from, u64 to, const std::function<void(u64, u32, const Alias&)>& fn) const
		{
			if (m_aliases.empty() || from >= to)
				return;
			// Tokenize from a bit before `from`, so a token that starts above the
			// visible area (and the boundaries that follow it) come out right. The
			// start is rounded down to a block boundary, so it doesn't move (and the
			// tokens don't change) while scrolling, and nearby queries agree.
			const u64 lookBehind = 256 + m_maxAliasLength;
			const u64 block = 1024;
			u64 i = from > lookBehind ? from - lookBehind : 0;
			i -= i % block;
			const u64 end = std::min<u64>(to, m_data.size());
			while (i < end)
			{
				const Alias* a = alias_match_at(i);
				if (a == nullptr)
				{
					i++;
					continue;
				}
				const u32 len = cast<u32>(a->bytes.size());
				if (i + len > from)
					fn(i, len, *a);
				i += len;
			}
		}

		const HexEditor::Alias* HexEditor::getAliasAt(StreamIndex index, StreamIndex* outStart, u32* outLength) const
		{
			const Alias* found = nullptr;
			resolve_aliases(index, cast<u64>(index) + 1, [&](u64 start, u32 length, const Alias& a) {
				if (index >= start && index < start + length)
				{
					found = &a;
					if (outStart) *outStart = cast<StreamIndex>(start);
					if (outLength) *outLength = length;
				}
			});
			return found;
		}

		// ============================ DATA ============================
		void HexEditor::setData(std::span<const i8> data, StreamIndex start, u8 lineLen, u16 nRows, i32 addrHighlight, u32 highlightRange, const String& title)
		{
			set_data(data, nullptr);
			m_lineLen = lineLen;
			m_rowCount = nRows;
			m_title = title;
			setStart(start);
			setHighlight(addrHighlight, highlightRange);
		}

		void HexEditor::setData(std::span<i8> data, StreamIndex start, u8 lineLen, u16 nRows, i32 addrHighlight, u32 highlightRange, const String& title)
		{
			setData(std::span<const i8>(data.data(), data.size()), start, lineLen, nRows, addrHighlight, highlightRange, title);
			m_mutableData = data.data();
		}

		void HexEditor::updateData(std::span<const i8> data)
		{
			set_data(data, nullptr);
		}

		void HexEditor::updateData(std::span<i8> data)
		{
			set_data(std::span<const i8>(data.data(), data.size()), data.data());
		}

		void HexEditor::set_data(std::span<const i8> data, i8* mutableData)
		{
			m_data = data;
			m_mutableData = mutableData;
			if (m_start >= m_data.size())
				m_start = cast<StreamIndex>(max_start());
		}

		void HexEditor::setHighlight(i32 addrHighlight, u32 highlightRange)
		{
			m_fixedHighlight = addrHighlight;
			m_fixedHighlightRange = std::max<u32>(1, highlightRange);
			if (m_followHighlight && m_fixedHighlight >= 0)
				ensureVisible(cast<StreamIndex>(m_fixedHighlight), m_fixedHighlightRange);
		}

		// ============================ VIEW ============================
		void HexEditor::setStart(StreamIndex start)
		{
			m_start = cast<StreamIndex>(std::min<u64>(start, max_start()));
		}

		void HexEditor::scrollRows(i64 rows)
		{
			const i64 bpr = getBytesPerRow();
			const i64 next = std::clamp<i64>(cast<i64>(m_start) + rows * bpr, 0, cast<i64>(max_start()));
			m_start = cast<StreamIndex>(next);
		}

		bool HexEditor::jumpTo(u64 address, bool select, bool addToHistory)
		{
			if (address < m_baseAddress || address - m_baseAddress >= m_data.size())
				return false;
			const StreamIndex index = cast<StreamIndex>(address - m_baseAddress);
			if (addToHistory)
				push_history(m_start);
			const u32 bpr = getBytesPerRow();
			setStart(index - (index % bpr));
			if (select)
				this->select(index);
			return true;
		}

		bool HexEditor::goBack(void)
		{
			if (m_history.empty())
				return false;
			setStart(m_history.back());
			m_history.pop_back();
			update_toggles();
			return true;
		}

		void HexEditor::ensureVisible(StreamIndex index, u32 size)
		{
			const i64 bpr = getBytesPerRow();
			const i64 rows = std::max(1, getVisibleRows());
			const i64 first = index;
			const i64 last = cast<i64>(index) + std::max<u32>(1, size) - 1;
			const i64 viewEnd = cast<i64>(m_start) + rows * bpr;
			if (first >= m_start && last < viewEnd)
				return;
			// Keep the phase of the current rows, and some context above the target.
			const i64 phase = cast<i64>(m_start) % bpr;
			i64 rowStart = first - ((first - phase) % bpr + bpr) % bpr;
			rowStart -= (rows / 3) * bpr;
			m_start = cast<StreamIndex>(std::clamp<i64>(rowStart, 0, cast<i64>(max_start())));
		}

		Rectangle HexEditor::getCellBounds(StreamIndex index, bool ascii) const
		{
			const Layout& l = m_layout;
			if (index < m_start || index >= m_data.size() || l.bytesPerRow == 0)
				return { 0, 0, 0, 0 };
			const u64 rel = index - m_start;
			const u64 row = rel / l.bytesPerRow;
			const u32 column = cast<u32>(rel % l.bytesPerRow);
			if (row >= cast<u64>(l.rows))
				return { 0, 0, 0, 0 };
			const f32 y = l.grid.y + l.headerH + row * l.rowH;
			if (ascii)
				return { l.asciiX + column * m_metrics.asciiCellW, y, m_metrics.asciiCellW, l.rowH };
			return { hex_x(column), y, m_metrics.hexCellW, l.rowH };
		}

		u32 HexEditor::getBytesPerRow(void) const
		{
			return std::max<u32>(1, m_layout.bytesPerRow);
		}

		i32 HexEditor::getVisibleRows(void) const
		{
			return m_layout.rows;
		}

		u64 HexEditor::max_start(void) const
		{
			if (m_data.empty())
				return 0;
			const i64 bpr = std::max<u32>(1, m_layout.bytesPerRow);
			const i64 rows = std::max(1, m_layout.rows);
			const i64 phase = cast<i64>(m_start) % bpr;
			const i64 last = cast<i64>(m_data.size()) - 1;
			i64 lastRow = last - ((last - phase) % bpr + bpr) % bpr;
			return cast<u64>(std::max<i64>(0, lastRow - (rows - 1) * bpr));
		}

		// ============================ SELECTION ============================
		void HexEditor::select(StreamIndex index)
		{
			if (index >= m_data.size())
				return;
			m_selection = index;
			ensureVisible(index, m_unitSize);
			notify_selection();
		}

		void HexEditor::setUnitSize(u8 size)
		{
			if (size != 1 && size != 2 && size != 4 && size != 8)
				return;
			m_unitSize = size;
			if (m_align && hasSelection())
				m_selection = unit_start(getSelection());
			update_toggles();
			notify_selection();
		}

		void HexEditor::setBigEndian(bool bigEndian)
		{
			m_bigEndian = bigEndian;
			update_toggles();
		}

		void HexEditor::enableAlign(bool align)
		{
			if (m_align == align)
				return;
			m_align = align;
			if (m_align && hasSelection())
				m_selection = unit_start(getSelection());
			update_toggles();
		}

		void HexEditor::enableFollowHighlight(bool follow)
		{
			if (m_followHighlight == follow)
				return;
			m_followHighlight = follow;
			if (m_followHighlight && m_fixedHighlight >= 0)
				ensureVisible(cast<StreamIndex>(m_fixedHighlight), m_fixedHighlightRange);
			update_toggles();
		}

		void HexEditor::enableChangeFlash(bool flash)
		{
			if (m_changeFlash == flash)
				return;
			m_changeFlash = flash;
			std::fill(m_trackHeat.begin(), m_trackHeat.end(), 0.0f);
			update_toggles();
		}

		void HexEditor::notify_selection(void)
		{
			if (m_selectionCallback && hasSelection())
				m_selectionCallback(*this, getSelection(), m_unitSize);
		}

		// ============================ VALUES ============================
		u64 HexEditor::readValue(StreamIndex index, u8 size) const
		{
			if (index >= m_data.size())
				return 0;
			const u32 n = cast<u32>(std::min<u64>(size, m_data.size() - index));
			u64 value = 0;
			for (u32 i = 0; i < n; i++)
			{
				const u64 b = cast<u8>(m_data[index + i]);
				if (m_bigEndian)
					value = (value << 8) | b;
				else
					value |= b << (8 * i);
			}
			return value;
		}

		bool HexEditor::writeValue(StreamIndex index, u64 value, u8 size)
		{
			if (index >= m_data.size())
				return false;
			const u32 n = cast<u32>(std::min<u64>(size, m_data.size() - index));
			bool written = false;
			if (m_mutableData != nullptr)
			{
				for (u32 i = 0; i < n; i++)
				{
					const u32 shift = m_bigEndian ? 8 * (n - 1 - i) : 8 * i;
					m_mutableData[index + i] = cast<i8>((value >> shift) & 0xFF);
				}
				written = true;
			}
			if (m_writeCallback)
				written = m_writeCallback(index, value & size_mask(cast<u8>(n)), cast<u8>(n)) || written;
			return written;
		}

		// ============================ HIGHLIGHTS ============================
		i32 HexEditor::addHighlight(StreamIndex index, u32 size, const Color& color, const String& label)
		{
			Highlight h;
			h.id = m_nextHighlightId++;
			h.index = index;
			h.size = std::max<u32>(1, size);
			h.color = color;
			h.label = label;
			m_highlights.push_back(h);
			return h.id;
		}

		bool HexEditor::updateHighlight(i32 id, StreamIndex index, u32 size)
		{
			for (auto& h : m_highlights)
			{
				if (h.id != id)
					continue;
				h.index = index;
				h.size = std::max<u32>(1, size);
				return true;
			}
			return false;
		}

		bool HexEditor::removeHighlight(i32 id)
		{
			return std::erase_if(m_highlights, [id](const Highlight& h) { return h.id == id; }) > 0;
		}

		// ============================ WATCHES ============================
		i32 HexEditor::addWatch(StreamIndex index, u8 size, const String& label, const Color& color)
		{
			if (index >= m_data.size())
				return -1;
			Watch w;
			w.index = index;
			w.size = size;
			w.value = readValue(index, size);
			w.label = (label == "" ? String("W").add(cast<i32>(m_watches.size()) + 1) : label);
			w.color = (color.a == 0 ? next_watch_color() : color);
			m_watches.push_back(w);
			update_toggles();
			show_message(String("Watching ").add(cast<i32>(size)).add(" byte(s) at ").add(formatAddress(m_baseAddress + index)));
			return cast<i32>(m_watches.size()) - 1;
		}

		bool HexEditor::setWatchLabel(i32 watchIndex, const String& label)
		{
			if (watchIndex < 0 || watchIndex >= cast<i32>(m_watches.size()) || label == "")
				return false;
			m_watches[watchIndex].label = label;
			m_watchList.resetColumnWidths();
			return true;
		}

		bool HexEditor::setWatchColor(i32 watchIndex, const Color& color)
		{
			if (watchIndex < 0 || watchIndex >= cast<i32>(m_watches.size()))
				return false;
			m_watches[watchIndex].color = color;
			// (setColor() runs the button's callback, which calls this again: only on a change.)
			const Color shown = m_watchColorButton.getColor();
			if (watchIndex == m_watchList.getSelectedRow() && (shown.r != color.r || shown.g != color.g || shown.b != color.b || shown.a != color.a))
				m_watchColorButton.setColor(color);
			return true;
		}

		Color HexEditor::next_watch_color(void)
		{
			// Random start, then golden-ratio hue steps: random-looking but never two
			// similar colors in a row.
			const f32 hue = m_nextWatchHue;
			m_nextWatchHue = std::fmod(m_nextWatchHue + 0.618034f, 1.0f);
			f32 r = 0, g = 0, b = 0;
			Color::HSVtoRGB(hue, 0.65f, 0.95f, r, g, b);
			return Color(cast<u8>(r * 255.0f), cast<u8>(g * 255.0f), cast<u8>(b * 255.0f));
		}

		void HexEditor::select_watch_row(i32 row)
		{
			if (row < 0 || row >= cast<i32>(m_watches.size()))
				return;
			m_watchNameInput.setText(m_watches[row].label);
			m_watchColorButton.setColor(m_watches[row].color);
		}

		bool HexEditor::removeWatch(i32 watchIndex)
		{
			if (watchIndex < 0 || watchIndex >= cast<i32>(m_watches.size()))
				return false;
			m_watches.erase(m_watches.begin() + watchIndex);
			if (m_watchList.getSelectedRow() >= cast<i32>(m_watches.size()))
				m_watchList.setSelectedRow(cast<i32>(m_watches.size()) - 1, true);
			update_toggles();
			return true;
		}

		void HexEditor::clearWatches(void)
		{
			m_watches.clear();
			m_watchList.setSelectedRow(-1);
			update_toggles();
		}

		void HexEditor::checkWatches(void)
		{
			for (i32 i = 0; i < cast<i32>(m_watches.size()); i++)
			{
				Watch& w = m_watches[i];
				if (w.index >= m_data.size())
					continue;
				const u64 value = readValue(w.index, w.size);
				if (value == w.value)
					continue;
				WatchEvent evt;
				evt.editor = this;
				evt.watchIndex = i;
				evt.index = w.index;
				evt.address = m_baseAddress + w.index;
				evt.size = w.size;
				evt.oldValue = w.value;
				evt.newValue = value;
				evt.bigEndian = m_bigEndian;
				evt.label = w.label;
				w.value = value;
				// The handlers may add/remove watches: nothing below uses `w` any more.
				ostd::SignalHandler::emitSignal(m_watchSignalID, ostd::Signal::Priority::RealTime, &evt);
				if (m_watchCallback)
					m_watchCallback(evt);
			}
		}

		// ============================ WINDOWS ============================
		void HexEditor::openEditWindow(void)
		{
			if (!hasSelection())
			{
				show_message("Select a value to edit first");
				return;
			}
			m_editIndex = getSelection();
			m_editSize = cast<u8>(std::min<u64>(m_unitSize, m_data.size() - m_editIndex));
			m_editWindow.setTitle(String("Edit ").add(formatAddress(m_baseAddress + m_editIndex)));
			m_editWindow.open();
			set_edit_format(m_editFormat);
			m_editInput.setText(formatValue(readValue(m_editIndex, m_editSize), m_editFormat, m_editSize));
			m_editInput.selectAll();
			refresh_edit_window();
			// Fit the window to its content (it depends on the theme's fonts and paddings).
			const Vec2 needed = m_editWindow.contentToWidgetSize(m_editBox.getCombinedMinSize());
			m_editWindow.setSize(std::max(560.0f, std::ceil(needed.x)), std::ceil(needed.y) + 4.0f);
			m_editWindow.centerInParent();
			getWindow().getFocusManager().requestFocus(m_editInput);
		}

		void HexEditor::openWatchList(void)
		{
			const bool wasOpen = m_watchWindow.isVisible() && m_watchWindow.getParent() != nullptr;
			m_watchWindow.open();
			if (!wasOpen)
				m_watchWindow.centerInParent();
		}

		void HexEditor::openRegionList(void)
		{
			const bool wasOpen = m_regionWindow.isVisible() && m_regionWindow.getParent() != nullptr;
			m_regionList.resetColumnWidths();
			m_regionWindow.open();
			if (!wasOpen)
				m_regionWindow.centerInParent();
		}

		void HexEditor::openSearchWindow(void)
		{
			const bool wasOpen = m_searchWindow.isVisible() && m_searchWindow.getParent() != nullptr;
			m_searchWindow.open();
			if (!wasOpen)
				m_searchWindow.centerInParent();
			update_search_status();
			m_searchInput.selectAll();
			getWindow().getFocusManager().requestFocus(m_searchInput);
		}

		// ============================ SEARCH ============================
		bool HexEditor::parse_hex_pattern(const String& text, stdvec<i16>& out)
		{
			// Pairs of digits or "??"; separators and "0x" prefixes are skipped.
			out.clear();
			const ostd::cpp_string& src = text.cpp_str();
			ostd::cpp_string digits;
			for (size_t i = 0; i < src.size(); i++)
			{
				const char c = src[i];
				if (c == ' ' || c == ',' || c == '\t')
				{
					if (digits.size() % 2 != 0)
						return false; // "A BC": a lone digit
					continue;
				}
				if (c == '0' && i + 1 < src.size() && (src[i + 1] == 'x' || src[i + 1] == 'X') && digits.size() % 2 == 0)
				{
					i++;
					continue;
				}
				if (!std::isxdigit(cast<u8>(c)) && c != '?')
					return false;
				digits.push_back(c);
			}
			if (digits.empty() || digits.size() % 2 != 0)
				return false;
			for (size_t i = 0; i < digits.size(); i += 2)
			{
				const char a = digits[i], b = digits[i + 1];
				if (a == '?' || b == '?')
				{
					if (a != b)
					{
						out.clear();
						return false; // "A?" isn't supported, only whole wildcard bytes
					}
					out.push_back(-1);
					continue;
				}
				out.push_back(cast<i16>(std::stoi(ostd::cpp_string { a, b }, nullptr, 16)));
			}
			return true;
		}

		bool HexEditor::setSearchPattern(const String& text, eSearchMode mode, bool matchCase)
		{
			clearSearch();
			m_searchMode = mode;
			m_searchIgnoreCase = (mode == eSearchMode::Ascii && !matchCase);
			bool valid = false;
			if (mode == eSearchMode::Ascii)
			{
				for (char c : text.cpp_str())
					m_searchPattern.push_back(cast<i16>(cast<u8>(c)));
				valid = !m_searchPattern.empty();
			}
			else
				valid = parse_hex_pattern(text, m_searchPattern);
			if (!valid)
				m_searchPattern.clear();

			// The search window shows whatever is being searched for, also when set from code.
			m_searchWindowMode = mode;
			for (i32 i = 0; i < 2; i++)
				m_searchModeButtons[i].setThemeQualifier("active", i == cast<i32>(mode));
			m_searchCaseCheck.enable(mode == eSearchMode::Ascii);
			if (mode == eSearchMode::Ascii && m_searchCaseCheck.isChecked() != matchCase)
				m_searchCaseCheck.setChecked(matchCase);
			if (m_searchInput.getText() != text)
				m_searchInput.setText(text);
			m_lastSearchText = text;
			update_search_status();
			return valid;
		}

		bool HexEditor::match_pattern(u64 index, const stdvec<i16>& pattern, bool ignoreCase) const
		{
			const u64 len = pattern.size();
			if (len == 0 || index + len > m_data.size())
				return false;
			for (u64 k = 0; k < len; k++)
			{
				const i16 p = pattern[k];
				if (p < 0)
					continue;
				const u8 b = cast<u8>(m_data[index + k]);
				if (ignoreCase ? std::tolower(b) != std::tolower(p) : b != p)
					return false;
			}
			return true;
		}

		bool HexEditor::match_at(StreamIndex index) const
		{
			return match_pattern(index, m_searchPattern, m_searchIgnoreCase);
		}

		i64 HexEditor::find(i64 from, bool backwards, bool wrap) const
		{
			const i64 len = cast<i64>(m_searchPattern.size());
			const i64 last = cast<i64>(m_data.size()) - len;
			if (len == 0 || last < 0)
				return -1;
			// Fast path: jump between occurrences of a fixed, case-exact first byte.
			const i16 first = m_searchPattern[0];
			const bool useMemchr = first >= 0 && !(m_searchIgnoreCase && std::isalpha(first));
			const u8* base = reinterpret_cast<const u8*>(m_data.data());
			auto l_forward = [&](i64 begin, i64 end) -> i64 {
				for (i64 i = begin; i <= end; i++)
				{
					if (useMemchr)
					{
						const void* hit = std::memchr(base + i, first, cast<size_t>(end - i + 1));
						if (hit == nullptr)
							return -1;
						i = static_cast<const u8*>(hit) - base;
					}
					if (match_at(cast<StreamIndex>(i)))
						return i;
				}
				return -1;
			};
			auto l_backward = [&](i64 begin, i64 end) -> i64 {
				for (i64 i = begin; i >= end; i--)
				{
					if (match_at(cast<StreamIndex>(i)))
						return i;
				}
				return -1;
			};
			if (!backwards)
			{
				const i64 start = std::clamp<i64>(from, 0, last + 1);
				i64 found = l_forward(start, last);
				if (found < 0 && wrap && start > 0)
					found = l_forward(0, std::min(start - 1, last));
				return found;
			}
			const i64 start = std::clamp<i64>(from, -1, last);
			i64 found = l_backward(start, 0);
			if (found < 0 && wrap && start < last)
				found = l_backward(last, start + 1);
			return found;
		}

		i64 HexEditor::findNext(bool backwards)
		{
			if (m_searchPattern.empty())
				return -1;
			// From the selection: past it when it's the current match, else including it.
			i64 cursor = hasSelection() ? m_selection : cast<i64>(m_start);
			const bool onMatch = (m_searchCurrent >= 0 && cursor == m_searchCurrent);
			const i64 from = backwards ? cursor - 1 : (onMatch ? cursor + 1 : cursor);
			const i64 found = find(from, backwards, true);
			if (found < 0)
			{
				show_message("Not found");
				return -1;
			}
			if ((!backwards && found < from) || (backwards && found > from))
				show_message(backwards ? "Search wrapped to the end" : "Search wrapped to the start");
			m_searchCurrent = found;
			select(cast<StreamIndex>(found));
			ensureVisible(cast<StreamIndex>(found), getSearchPatternLength());
			return found;
		}

		size_t HexEditor::findAll(size_t maxResults)
		{
			m_searchResults.clear();
			m_searchTruncated = false;
			for (i64 i = find(0, false, false); i >= 0; i = find(i + 1, false, false))
			{
				if (m_searchResults.size() >= maxResults)
				{
					m_searchTruncated = true;
					break;
				}
				m_searchResults.push_back(cast<StreamIndex>(i));
			}
			return m_searchResults.size();
		}

		void HexEditor::clearSearch(void)
		{
			m_searchPattern.clear();
			m_searchResults.clear();
			m_searchTruncated = false;
			m_searchCurrent = -1;
		}

		void HexEditor::set_search_mode(eSearchMode mode)
		{
			// The pattern means something else in the other mode.
			if (mode != m_searchWindowMode)
				clearSearch();
			m_searchWindowMode = mode;
			for (i32 i = 0; i < 2; i++)
				m_searchModeButtons[i].setThemeQualifier("active", i == cast<i32>(mode));
			m_searchCaseCheck.enable(mode == eSearchMode::Ascii);
			update_search_status();
		}

		bool HexEditor::apply_search_input(void)
		{
			// Only re-parse (which clears the results) when something changed.
			const String text = m_searchInput.getText();
			const eSearchMode mode = m_searchWindowMode;
			const bool ignoreCase = (mode == eSearchMode::Ascii && !m_searchCaseCheck.isChecked());
			if (!m_searchPattern.empty() && text == m_lastSearchText && mode == m_searchMode && ignoreCase == m_searchIgnoreCase)
				return true;
			m_lastSearchText = text;
			if (!setSearchPattern(text, mode, !ignoreCase))
			{
				m_searchStatus.setText(mode == eSearchMode::Hex ? "Invalid hex pattern (e.g. \"DE AD ?? EF\")" : "Type some text to search for");
				return false;
			}
			m_searchList.resetColumnWidths();
			return true;
		}

		void HexEditor::search_window_find(bool backwards)
		{
			if (!apply_search_input())
				return;
			findNext(backwards);
			update_search_status();
		}

		void HexEditor::update_search_status(void)
		{
			if (m_searchPattern.empty())
			{
				m_searchStatus.setText(m_searchWindowMode == eSearchMode::Hex ? "Hex bytes, e.g. \"DE AD ?? EF\" (?? = any byte)" : "Text to search for");
				return;
			}
			String status = String("").add(cast<i32>(m_searchPattern.size())).add(" byte pattern");
			if (m_searchCurrent >= 0)
				status += String("  |  at ") + formatAddress(m_baseAddress + cast<u64>(m_searchCurrent));
			if (!m_searchResults.empty() || m_searchTruncated)
			{
				status += String("  |  ").add(std::to_string(m_searchResults.size())).add(m_searchTruncated ? "+ matches (stopped)" : " matches");
				auto it = std::lower_bound(m_searchResults.begin(), m_searchResults.end(), cast<StreamIndex>(std::max<i64>(0, m_searchCurrent)));
				if (m_searchCurrent >= 0 && it != m_searchResults.end() && cast<i64>(*it) == m_searchCurrent)
					status += String(" (").add(std::to_string(it - m_searchResults.begin() + 1)).add(")");
			}
			m_searchStatus.setText(status);
		}

		void HexEditor::set_edit_format(eFormat format)
		{
			// Converts what's typed so far to the new format.
			u64 value = 0;
			const bool valid = parseValue(m_editInput.getText(), m_editFormat, m_editSize, value);
			m_editFormat = format;
			switch (format)
			{
				case eFormat::Dec: m_editInput.useIntegerFilter(true, 20); break;
				case eFormat::Hex: m_editInput.useHexadecimalFilter(m_editSize * 2); break;
				case eFormat::Bin:
					static_cast<BinaryFilter*>(m_binaryFilter.get())->setMaxDigits(m_editSize * 8);
					m_editInput.useCustomFilter(m_binaryFilter.get());
					break;
			}
			if (valid)
				m_editInput.setText(formatValue(value, format, m_editSize));
			for (i32 i = 0; i < 3; i++)
				m_editFormatButtons[i].setThemeQualifier("active", i == cast<i32>(format));
			refresh_edit_window();
		}

		void HexEditor::refresh_edit_window(void)
		{
			const u64 current = readValue(m_editIndex, m_editSize);
			m_editInfo.setText(String("").add(cast<i32>(m_editSize)).add(" byte(s) at ").add(formatAddress(m_baseAddress + m_editIndex))
				.add(m_bigEndian ? " (big endian)" : " (little endian)").add(isEditable() || m_writeCallback ? "" : " - read only"));
			m_editCurrent.setText(String("Current: 0x").add(hex_string(current, m_editSize * 2)).add("  =  ")
				.add(std::to_string(current)).add("  /  ").add(std::to_string(sign_extend(current, m_editSize))));
			m_lastEditText = m_editInput.getText();
			u64 value = 0;
			if (m_lastEditText.new_trim() == "")
				m_editStatus.setText("Type a value (or 0x... / 0b...)");
			else if (!parseValue(m_lastEditText, m_editFormat, m_editSize, value))
				m_editStatus.setText(String("Invalid, or doesn't fit in ").add(cast<i32>(m_editSize)).add(" byte(s)"));
			else
				m_editStatus.setText(String("New: 0x").add(hex_string(value, m_editSize * 2)).add("  =  ").add(std::to_string(value))
					.add(m_editSize <= 2 ? String("  =  0b") + binary_string(value, m_editSize, false) : String("")));
		}

		void HexEditor::apply_edit(void)
		{
			u64 value = 0;
			if (!parseValue(m_editInput.getText(), m_editFormat, m_editSize, value))
			{
				refresh_edit_window();
				return;
			}
			if (!writeValue(m_editIndex, value, m_editSize))
			{
				m_editStatus.setText("The data is read only (no write callback)");
				return;
			}
			m_editWindow.close();
			getWindow().getFocusManager().requestFocus(*this);
		}

		// ============================ HELPERS ============================
		String HexEditor::formatAddress(u64 address) const
		{
			const u64 last = m_baseAddress + std::max<u64>(1, m_data.size()) - 1;
			i32 digits = 4;
			while (digits < 16 && (last >> (digits * 4)) != 0)
				digits += 2;
			return String("0x") + hex_string(address, digits);
		}

		bool HexEditor::parseValue(const String& text, eFormat format, u8 size, u64& outValue)
		{
			ostd::cpp_string s = text.new_trim().cpp_str();
			std::erase(s, '_');
			std::erase(s, ' ');
			if (s.empty())
				return false;
			bool negative = false;
			if (s[0] == '-')
			{
				negative = true;
				s.erase(0, 1);
				format = eFormat::Dec;
			}
			if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
			{
				format = eFormat::Hex;
				s.erase(0, 2);
			}
			else if (s.size() > 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B') && format != eFormat::Hex)
			{
				format = eFormat::Bin;
				s.erase(0, 2);
			}
			if (s.empty() || (negative && format != eFormat::Dec))
				return false;

			const u64 base = (format == eFormat::Hex ? 16 : (format == eFormat::Bin ? 2 : 10));
			u64 value = 0;
			for (char c : s)
			{
				u64 digit = 0;
				if (c >= '0' && c <= '9') digit = cast<u64>(c - '0');
				else if (c >= 'a' && c <= 'f') digit = cast<u64>(c - 'a' + 10);
				else if (c >= 'A' && c <= 'F') digit = cast<u64>(c - 'A' + 10);
				else return false;
				if (digit >= base)
					return false;
				if (value > (~0ull - digit) / base)
					return false; // overflows 64 bits
				value = value * base + digit;
			}
			if (negative)
			{
				// Two's complement; the magnitude must fit the signed range.
				const u64 limit = (size >= 8 ? (1ull << 63) : (1ull << (size * 8 - 1)));
				if (value > limit)
					return false;
				value = (~value + 1) & size_mask(size);
			}
			else if ((value & ~size_mask(size)) != 0)
				return false;
			outValue = value;
			return true;
		}

		String HexEditor::formatValue(u64 value, eFormat format, u8 size)
		{
			value &= size_mask(size);
			switch (format)
			{
				case eFormat::Dec: return String(std::to_string(value));
				case eFormat::Hex: return hex_string(value, size * 2);
				case eFormat::Bin: return binary_string(value, size, false);
			}
			return "";
		}

		void HexEditor::push_history(StreamIndex index)
		{
			if (!m_history.empty() && m_history.back() == index)
				return;
			m_history.push_back(index);
			if (cast<i32>(m_history.size()) > MaxHistory)
				m_history.erase(m_history.begin());
			update_toggles();
		}

		void HexEditor::show_message(const String& message)
		{
			m_message = message;
			m_messageUntilMs = now_ms() + 3000;
		}

		void HexEditor::update_toggles(void)
		{
			for (i32 i = 0; i < 4; i++)
				m_sizeButtons[i].setThemeQualifier("active", UnitSizes[i] == m_unitSize);
			m_endianButton.setText(m_bigEndian ? "BE" : "LE");
			m_endianButton.setTooltipText(m_bigEndian ? "Big endian" : "Little endian");
			if (m_alignCheck.isChecked() != m_align) m_alignCheck.setChecked(m_align);
			if (m_followCheck.isChecked() != m_followHighlight) m_followCheck.setChecked(m_followHighlight);
			if (m_flashCheck.isChecked() != m_changeFlash) m_flashCheck.setChecked(m_changeFlash);
			m_backButton.enable(!m_history.empty());
			m_watchListButton.setText(String("Watchlist (").add(cast<i32>(m_watches.size())).add(")"));
		}

		bool HexEditor::in_range(StreamIndex index, i64 start, u32 size) const
		{
			return start >= 0 && cast<i64>(index) >= start && cast<i64>(index) < start + cast<i64>(size);
		}

		const HexEditor::Highlight* HexEditor::highlight_at(StreamIndex index) const
		{
			// Later highlights are drawn on top, so they win.
			for (auto it = m_highlights.rbegin(); it != m_highlights.rend(); ++it)
			{
				if (in_range(index, it->index, it->size))
					return &*it;
			}
			return nullptr;
		}

		const HexEditor::Watch* HexEditor::watch_at(StreamIndex index) const
		{
			for (const auto& w : m_watches)
			{
				if (in_range(index, w.index, w.size))
					return &w;
			}
			return nullptr;
		}

		StreamIndex HexEditor::unit_start(StreamIndex index) const
		{
			if (!m_align)
				return index;
			// Aligned on the address, not the offset in the data.
			const u64 misalign = (m_baseAddress + index) % m_unitSize;
			return cast<StreamIndex>(index >= misalign ? index - misalign : 0);
		}

		// ============================ THEME ============================
		void HexEditor::applyTheme(const ostd::Stylesheet& theme)
		{
			m_gridFontSize = getThemeValue<i32>(theme, "gridFontSize", m_gridFontSize);
			m_rowSpacing = getThemeValue<f32>(theme, "rowSpacing", m_rowSpacing);
			m_groupSpacing = getThemeValue<f32>(theme, "groupSpacing", m_groupSpacing);
			m_groupSize = cast<u32>(std::max(1, getThemeValue<i32>(theme, "groupSize", cast<i32>(m_groupSize))));
			m_scrollbarWidth = getThemeValue<f32>(theme, "scrollbarWidth", m_scrollbarWidth);
			m_changeFadeTime = getThemeValue<f32>(theme, "changeFadeTime", m_changeFadeTime);
			m_addressColor = getThemeValue<Color>(theme, "addressColor", m_addressColor);
			m_headerColor = getThemeValue<Color>(theme, "headerColor", m_headerColor);
			m_hexColor = getThemeValue<Color>(theme, "hexColor", m_hexColor);
			m_zeroColor = getThemeValue<Color>(theme, "zeroColor", m_zeroColor);
			m_asciiColor = getThemeValue<Color>(theme, "asciiColor", m_asciiColor);
			m_asciiDotColor = getThemeValue<Color>(theme, "asciiDotColor", m_asciiDotColor);
			m_titleColor = getThemeValue<Color>(theme, "titleColor", m_titleColor);
			m_selectionColor = getThemeValue<Color>(theme, "selectionColor", m_selectionColor);
			m_selectionTextColor = getThemeValue<Color>(theme, "selectionTextColor", m_selectionTextColor);
			m_hoverColor = getThemeValue<Color>(theme, "hoverColor", m_hoverColor);
			m_hoverBorderColor = getThemeValue<Color>(theme, "hoverBorderColor", m_hoverBorderColor);
			m_highlightColor = getThemeValue<Color>(theme, "highlightColor", m_highlightColor);
			m_changedColor = getThemeValue<Color>(theme, "changedColor", m_changedColor);
			m_watchColor = getThemeValue<Color>(theme, "watchColor", m_watchColor);
			m_crosshairColor = getThemeValue<Color>(theme, "crosshairColor", m_crosshairColor);
			m_rowStripeColor = getThemeValue<Color>(theme, "rowStripeColor", m_rowStripeColor);
			m_separatorColor = getThemeValue<Color>(theme, "separatorColor", m_separatorColor);
			m_gridBackgroundColor = getThemeValue<Color>(theme, "gridBackgroundColor", m_gridBackgroundColor);
			m_topBarColor = getThemeValue<Color>(theme, "topBarColor", m_topBarColor);
			m_statusBackgroundColor = getThemeValue<Color>(theme, "statusBar.backgroundColor", m_statusBackgroundColor);
			m_statusTextColor = getThemeValue<Color>(theme, "statusBar.textColor", m_statusTextColor);
			m_statusLabelColor = getThemeValue<Color>(theme, "statusBar.labelColor", m_statusLabelColor);
			m_scrollTrackColor = getThemeValue<Color>(theme, "scrollbar.trackColor", m_scrollTrackColor);
			m_scrollThumbColor = getThemeValue<Color>(theme, "scrollbar.thumbColor", m_scrollThumbColor);
			m_scrollMarkerColor = getThemeValue<Color>(theme, "scrollbar.markerColor", m_scrollMarkerColor);
			m_searchMatchColor = getThemeValue<Color>(theme, "search.matchColor", m_searchMatchColor);
			m_searchCurrentColor = getThemeValue<Color>(theme, "search.currentColor", m_searchCurrentColor);
			m_searchBorderColor = getThemeValue<Color>(theme, "search.borderColor", m_searchBorderColor);
			m_metrics.fontSize = 0; // re-measure
		}

		Vec2 HexEditor::getContentMinSize(void) const
		{
			const Vec2 bar = m_topBar.getCombinedMinSize();
			const Rectangle pad = getPadding();
			return { std::max(bar.x, 420.0f) + pad.left() + pad.right(), bar.y + 220.0f + pad.top() + pad.bottom() };
		}

		// ============================ LAYOUT ============================
		void HexEditor::update_metrics(ogfx::BasicRenderer2D& gfx)
		{
			if (m_metrics.fontSize == m_gridFontSize && !m_hexWidths.empty())
				return;
			m_metrics.fontSize = m_gridFontSize;
			m_hexWidths.assign(256, 0.0f);
			m_asciiWidths.assign(256, 0.0f);
			f32 maxHex = 0, maxAscii = 0, maxDigit = 0;
			for (i32 i = 0; i < 256; i++)
			{
				m_hexWidths[i] = gfx.getStringDimensions(m_hexStrings[i], m_gridFontSize).x;
				m_asciiWidths[i] = gfx.getStringDimensions(m_asciiStrings[i], m_gridFontSize).x;
				maxHex = std::max(maxHex, m_hexWidths[i]);
				maxAscii = std::max(maxAscii, m_asciiWidths[i]);
			}
			for (const char* d : { "0", "8", "A", "B", "D", "F" })
				maxDigit = std::max(maxDigit, gfx.getStringDimensions(d, m_gridFontSize).x);
			m_metrics.hexCellW = std::ceil(maxHex + m_gridFontSize * 0.5f);
			m_metrics.asciiCellW = std::ceil(maxAscii + 1.0f);
			m_metrics.digitW = maxDigit;
			m_metrics.charH = std::ceil(gfx.getStringDimensions("0Ag", m_gridFontSize).y);
		}

		void HexEditor::update_layout(void)
		{
			const Rectangle content = getGlobalContentBounds();
			const f32 topH = m_topBar.isVisible() ? m_topBar.geth() + 6.0f : 0.0f;
			const f32 lineH = m_metrics.charH + 2.0f;
			const f32 statusH = lineH * 3.0f + 10.0f;
			Layout& l = m_layout;
			l.rowH = m_metrics.charH + m_rowSpacing;
			l.headerH = l.rowH + 4.0f;
			l.grid = { content.x, content.y + topH, std::max(0.0f, content.w - m_scrollbarWidth), std::max(0.0f, content.h - topH - statusH - 4.0f) };
			l.status = { content.x, content.y + content.h - statusH, content.w, statusH };
			l.scrollbar = { l.grid.x + l.grid.w, l.grid.y + l.headerH, m_scrollbarWidth, std::max(0.0f, l.grid.h - l.headerH) };

			const String sample = formatAddress(m_baseAddress);
			l.addrW = (cast<f32>(sample.len()) - 2.0f) * m_metrics.digitW + 20.0f;
			l.hexX = l.grid.x + l.addrW;
			if (m_lineLen > 0)
				l.bytesPerRow = m_lineLen;
			else
			{
				// The largest power of two that fits (8 to 64), so rows start at round addresses.
				const f32 perByte = m_metrics.hexCellW + m_metrics.asciiCellW + m_groupSpacing / cast<f32>(m_groupSize);
				const u32 fit = cast<u32>(std::max(0.0f, (l.grid.w - l.addrW - 30.0f) / std::max(1.0f, perByte)));
				l.bytesPerRow = 8;
				while (l.bytesPerRow < 64 && l.bytesPerRow * 2 <= fit)
					l.bytesPerRow *= 2;
			}
			l.asciiX = hex_x(l.bytesPerRow) + 12.0f;
			l.rows = std::max(0, cast<i32>((l.grid.h - l.headerH) / std::max(1.0f, l.rowH)));
			if (m_rowCount > 0)
				l.rows = std::min<i32>(l.rows, m_rowCount);
		}

		f32 HexEditor::hex_x(u32 column) const
		{
			return m_layout.hexX + cast<f32>(column) * m_metrics.hexCellW + cast<f32>(column / m_groupSize) * m_groupSpacing
				- (column > 0 && column % m_groupSize == 0 && column == m_layout.bytesPerRow ? m_groupSpacing : 0.0f);
		}

		i64 HexEditor::index_at(const Vec2& pos) const
		{
			const Layout& l = m_layout;
			const f32 top = l.grid.y + l.headerH;
			if (pos.y < top || pos.y >= top + l.rows * l.rowH || pos.x < l.grid.x || pos.x >= l.grid.x + l.grid.w)
				return -1;
			const i64 row = cast<i64>((pos.y - top) / l.rowH);
			i64 column = -1;
			const f32 groupW = m_groupSize * m_metrics.hexCellW + m_groupSpacing;
			if (pos.x >= l.hexX && pos.x < l.asciiX - 12.0f)
			{
				const f32 rel = pos.x - l.hexX;
				const i64 group = cast<i64>(rel / groupW);
				const f32 inGroup = rel - group * groupW;
				if (inGroup >= m_groupSize * m_metrics.hexCellW)
					return -1; // the gap between groups
				column = group * m_groupSize + cast<i64>(inGroup / m_metrics.hexCellW);
			}
			else if (pos.x >= l.asciiX)
				column = cast<i64>((pos.x - l.asciiX) / m_metrics.asciiCellW);
			if (column < 0 || column >= l.bytesPerRow)
				return -1;
			const i64 index = cast<i64>(m_start) + row * l.bytesPerRow + column;
			return index < cast<i64>(m_data.size()) ? index : -1;
		}

		Rectangle HexEditor::scrollbar_thumb(void) const
		{
			const Rectangle& track = m_layout.scrollbar;
			const f64 total = std::max<f64>(1.0, cast<f64>(m_data.size()));
			const f64 visible = cast<f64>(m_layout.rows) * m_layout.bytesPerRow;
			const f32 h = std::clamp(cast<f32>(track.h * std::min(1.0, visible / total)), std::min(24.0f, track.h), track.h);
			const f64 maxS = cast<f64>(max_start());
			const f32 t = maxS > 0 ? cast<f32>(cast<f64>(m_start) / maxS) : 0.0f;
			return { track.x + 2, track.y + (track.h - h) * std::clamp(t, 0.0f, 1.0f), track.w - 4, h };
		}

		void HexEditor::scroll_to_thumb(f32 mouseY)
		{
			const Rectangle& track = m_layout.scrollbar;
			const Rectangle thumb = scrollbar_thumb();
			const f32 range = track.h - thumb.h;
			const f32 t = range > 0 ? std::clamp((mouseY - m_scrollGrab - track.y) / range, 0.0f, 1.0f) : 0.0f;
			const u64 bpr = getBytesPerRow();
			const u64 target = cast<u64>(std::llround(t * cast<f64>(max_start())));
			m_start = cast<StreamIndex>(std::min<u64>(target - (target % bpr), max_start()));
		}

		// ============================ UPDATE ============================
		void HexEditor::onUpdate(void)
		{
			const u64 now = now_ms();
			const f32 dt = m_lastUpdateMs == 0 ? 0.0f : std::min(0.25f, cast<f32>(now - m_lastUpdateMs) / 1000.0f);
			m_lastUpdateMs = now;
			update_change_tracking(dt);
			checkWatches();
			if (m_editWindow.isVisible() && m_editInput.getText() != m_lastEditText)
				refresh_edit_window();
		}

		void HexEditor::update_change_tracking(f32 dt)
		{
			const u64 count = std::min<u64>(cast<u64>(m_layout.rows) * m_layout.bytesPerRow, m_data.size() > m_start ? m_data.size() - m_start : 0);
			// Scrolling or new data: start over (nothing flashes).
			if (!m_changeFlash || m_trackStart != m_start || m_trackBytes.size() != count || m_trackData.data() != m_data.data())
			{
				m_trackStart = m_start;
				m_trackData = m_data;
				m_trackBytes.resize(count);
				m_trackHeat.assign(count, 0.0f);
				for (u64 i = 0; i < count; i++)
					m_trackBytes[i] = cast<u8>(m_data[m_start + i]);
				return;
			}
			const f32 decay = m_changeFadeTime > 0 ? dt / m_changeFadeTime : 1.0f;
			for (u64 i = 0; i < count; i++)
			{
				const u8 b = cast<u8>(m_data[m_start + i]);
				if (b != m_trackBytes[i])
				{
					m_trackBytes[i] = b;
					m_trackHeat[i] = 1.0f;
				}
				else if (m_trackHeat[i] > 0.0f)
					m_trackHeat[i] = std::max(0.0f, m_trackHeat[i] - decay);
			}
		}

		// ============================ INPUT ============================
		void HexEditor::onMousePressed(const Event& event)
		{
			if (event.mouse->button != MouseEventData::eButton::Left)
				return;
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			const Rectangle& track = m_layout.scrollbar;
			if (track.contains(pos, true))
			{
				const Rectangle thumb = scrollbar_thumb();
				if (thumb.contains(pos, true))
				{
					m_scrollDragging = true;
					m_scrollGrab = pos.y - thumb.y;
				}
				else
					scrollRows(pos.y < thumb.y ? -m_layout.rows : m_layout.rows);
				return;
			}
			const i64 index = index_at(pos);
			if (index < 0)
				return;
			const StreamIndex start = unit_start(cast<StreamIndex>(index));
			const u64 now = now_ms();
			const bool doubleClick = (now - m_lastClickMs <= DoubleClickMs && m_lastClickIndex == cast<i64>(start));
			m_lastClickMs = doubleClick ? 0 : now;
			m_lastClickIndex = start;
			m_selection = start;
			notify_selection();
			if (doubleClick)
				openEditWindow();
		}

		void HexEditor::onMouseReleased(const Event& event)
		{
			m_scrollDragging = false;
		}

		void HexEditor::onMouseMoved(const Event& event)
		{
			m_hover = index_at({ event.mouse->position_x, event.mouse->position_y });
		}

		void HexEditor::onMouseDragged(const Event& event)
		{
			if (m_scrollDragging)
				scroll_to_thumb(event.mouse->position_y);
			else
				m_hover = index_at({ event.mouse->position_x, event.mouse->position_y });
		}

		void HexEditor::onMouseExited(const Event& event)
		{
			m_hover = -1;
		}

		void HexEditor::onMouseScrolled(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			const Rectangle area { m_layout.grid.x, m_layout.grid.y, m_layout.grid.w + m_layout.scrollbar.w, m_layout.grid.h };
			if (!area.contains(pos, true) || event.mouse->scrollAmount.y == 0)
				return;
			scrollRows(event.mouse->scrollAmount.y > 0 ? -3 : 3);
			m_hover = index_at(pos);
			event.handle();
		}

		void HexEditor::onKeyPressed(const Event& event)
		{
			const i32 key = event.keyboard->keyCode;
			const auto& mods = event.keyboard->modifiers;
			if (key == KeyCode::F && mods.ctrl)
			{
				openSearchWindow();
				event.handle();
				return;
			}
			if (key == KeyCode::F3)
			{
				if (m_searchPattern.empty())
					openSearchWindow();
				else
					findNext(mods.lshift || mods.rshift);
				update_search_status();
				event.handle();
				return;
			}
			const i64 bpr = getBytesPerRow();
			const i64 rows = std::max(1, m_layout.rows);
			const i64 size = cast<i64>(m_data.size());
			if (size == 0)
				return;
			i64 sel = hasSelection() ? m_selection : cast<i64>(m_start);
			if (key == KeyCode::Left) sel -= m_unitSize;
			else if (key == KeyCode::Right) sel += m_unitSize;
			else if (key == KeyCode::Up) sel -= bpr;
			else if (key == KeyCode::Down) sel += bpr;
			else if (key == KeyCode::Pageup) { scrollRows(-rows); sel -= rows * bpr; }
			else if (key == KeyCode::Pagedown) { scrollRows(rows); sel += rows * bpr; }
			else if (key == KeyCode::Home) { m_start = 0; sel = 0; }
			else if (key == KeyCode::End) { m_start = cast<StreamIndex>(max_start()); sel = size - 1; }
			else
				return;
			select(cast<StreamIndex>(std::clamp<i64>(sel, 0, size - 1)));
			event.handle();
		}

		void HexEditor::onKeyReleased(const Event& event)
		{
			// On release, so the Return that opens the window doesn't also apply it.
			const i32 key = event.keyboard->keyCode;
			if (key == KeyCode::Return || key == KeyCode::Return2 || key == KeyCode::KpEnter)
				openEditWindow();
		}

		// ============================ DRAWING ============================
		void HexEditor::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			update_metrics(gfx);
			update_layout();
			const Rectangle content = getGlobalContentBounds();
			if (m_topBar.isVisible())
				gfx.fillRect({ content.x - getPadding().left(), content.y - getPadding().top(), getw(), m_topBar.geth() + getPadding().top() + 3.0f }, m_topBarColor);
			gfx.pushClippingRect({ m_layout.grid.x, m_layout.grid.y, m_layout.grid.w + m_layout.scrollbar.w, m_layout.grid.h }, true);
			gfx.fillRect({ m_layout.grid.x, m_layout.grid.y, m_layout.grid.w + m_layout.scrollbar.w, m_layout.grid.h }, m_gridBackgroundColor);
			draw_header(gfx);
			draw_rows(gfx);
			draw_scrollbar(gfx);
			gfx.popClippingRect();
			gfx.pushClippingRect(m_layout.status, true);
			draw_status(gfx);
			gfx.popClippingRect();
		}

		void HexEditor::draw_header(ogfx::BasicRenderer2D& gfx)
		{
			const Layout& l = m_layout;
			const f32 y = l.grid.y + 2.0f;
			const i64 hoverColumn = m_hover >= 0 ? (m_hover - cast<i64>(m_start)) % l.bytesPerRow : -1;
			gfx.pushClippingRect({ l.grid.x, l.grid.y, l.addrW - 4.0f, l.headerH }, true);
			gfx.drawString(m_title, { l.grid.x + 6.0f, y }, m_titleColor, m_gridFontSize);
			gfx.popClippingRect();
			for (u32 c = 0; c < l.bytesPerRow; c++)
			{
				const u8 v = cast<u8>((m_start + c) & 0xFF);
				const f32 x = hex_x(c);
				const Color color = (cast<i64>(c) == hoverColumn) ? m_selectionTextColor : m_headerColor;
				gfx.drawString(m_hexStrings[v], { x + (m_metrics.hexCellW - m_hexWidths[v]) * 0.5f, y }, color, m_gridFontSize);
			}
			const f32 lineY = l.grid.y + l.headerH - 2.0f;
			gfx.drawLine({ { l.grid.x, lineY }, { l.grid.x + l.grid.w, lineY } }, m_separatorColor, 1, false);
			gfx.drawLine({ { l.hexX - 8.0f, l.grid.y }, { l.hexX - 8.0f, l.grid.y + l.grid.h } }, m_separatorColor, 1, false);
			gfx.drawLine({ { l.asciiX - 7.0f, l.grid.y }, { l.asciiX - 7.0f, l.grid.y + l.grid.h } }, m_separatorColor, 1, false);
		}

		void HexEditor::draw_rows(ogfx::BasicRenderer2D& gfx)
		{
			const Layout& l = m_layout;
			const u64 size = m_data.size();
			const f32 top = l.grid.y + l.headerH;
			const f32 hexW = m_metrics.hexCellW, asciiW = m_metrics.asciiCellW;
			const i64 hoverStart = m_hover >= 0 ? cast<i64>(unit_start(cast<StreamIndex>(m_hover))) : -1;
			const i64 hoverRow = m_hover >= 0 ? (m_hover - cast<i64>(m_start)) / l.bytesPerRow : -1;
			const i64 hoverColumn = m_hover >= 0 ? (m_hover - cast<i64>(m_start)) % l.bytesPerRow : -1;
			const bool tracking = m_changeFlash && m_trackStart == m_start && m_trackData.data() == m_data.data();

			// Search matches touching the visible bytes (re-checked against the live data).
			const u64 visibleCount = std::min<u64>(cast<u64>(l.rows) * l.bytesPerRow, size > m_start ? size - m_start : 0);
			const i64 patternLen = cast<i64>(m_searchPattern.size());
			m_searchFlags.assign(visibleCount, 0);
			auto l_flag = [&](i64 match, u8 value) {
				const i64 from = std::max<i64>(match, m_start) - m_start;
				const i64 to = std::min<i64>(match + patternLen, cast<i64>(m_start + visibleCount)) - m_start;
				for (i64 i = from; i < to; i++)
					m_searchFlags[i] = std::max(m_searchFlags[i], value);
			};
			if (patternLen > 0 && visibleCount > 0)
			{
				const i64 firstPossible = std::max<i64>(0, cast<i64>(m_start) - patternLen + 1);
				for (auto it = std::lower_bound(m_searchResults.begin(), m_searchResults.end(), cast<StreamIndex>(firstPossible));
					 it != m_searchResults.end() && cast<u64>(*it) < m_start + visibleCount; ++it)
				{
					if (match_at(*it))
						l_flag(*it, 1);
				}
				if (m_searchCurrent >= 0 && match_at(cast<StreamIndex>(m_searchCurrent)))
					l_flag(m_searchCurrent, 2);
			}

			// Alias tokens over the visible bytes.
			m_aliasFlags.assign(visibleCount, -1);
			if (m_aliasColors && visibleCount > 0)
			{
				resolve_aliases(m_start, m_start + visibleCount, [&](u64 start, u32 length, const Alias& a) {
					const i32 aliasIndex = cast<i32>(&a - m_aliases.data());
					const u64 from = std::max<u64>(start, m_start), to = std::min<u64>(start + length, m_start + visibleCount);
					for (u64 k = from; k < to; k++)
						m_aliasFlags[k - m_start] = aliasIndex;
				});
			}

			// Crosshair through the hovered byte.
			if (hoverColumn >= 0)
			{
				const f32 hx = hex_x(cast<u32>(hoverColumn));
				gfx.fillRect({ hx, top, hexW, l.rows * l.rowH }, m_crosshairColor);
				gfx.fillRect({ l.asciiX + hoverColumn * asciiW, top, asciiW, l.rows * l.rowH }, m_crosshairColor);
				gfx.fillRect({ l.grid.x, top + hoverRow * l.rowH, l.grid.w, l.rowH }, m_crosshairColor);
			}

			for (i32 r = 0; r < l.rows; r++)
			{
				const u64 rowStart = cast<u64>(m_start) + cast<u64>(r) * l.bytesPerRow;
				if (rowStart >= size)
					break;
				const f32 y = top + r * l.rowH;
				const f32 ty = y + m_rowSpacing * 0.5f;
				if (r % 2 == 1)
					gfx.fillRect({ l.grid.x, y, l.grid.w, l.rowH }, m_rowStripeColor);
				const String addr = formatAddress(m_baseAddress + rowStart);
				gfx.drawString(addr.new_substr(2), { l.grid.x + 6.0f, ty }, r == hoverRow ? m_selectionTextColor : m_addressColor, m_gridFontSize);

				for (u32 c = 0; c < l.bytesPerRow; c++)
				{
					const u64 index = rowStart + c;
					if (index >= size)
						break;
					const StreamIndex si = cast<StreamIndex>(index);
					const u8 b = cast<u8>(m_data[index]);
					const Rectangle hexCell { hex_x(c), y, hexW, l.rowH };
					const Rectangle asciiCell { l.asciiX + c * asciiW, y, asciiW, l.rowH };
					auto l_fill = [&](const Color& color) {
						gfx.fillRect(hexCell, color);
						gfx.fillRect(asciiCell, color);
					};

					if (const Highlight* h = highlight_at(si))
						l_fill(h->color);
					if (in_range(si, m_fixedHighlight, m_fixedHighlightRange))
						l_fill(m_highlightColor);
					if (tracking && index - m_start < m_trackHeat.size())
					{
						const f32 heat = m_trackHeat[index - m_start];
						if (heat > 0.0f)
						{
							Color c2 = m_changedColor;
							c2.a = cast<u8>(c2.a * heat);
							l_fill(c2);
						}
					}
					if (index - m_start < m_searchFlags.size() && m_searchFlags[index - m_start] != 0)
						l_fill(m_searchFlags[index - m_start] == 2 ? m_searchCurrentColor : m_searchMatchColor);
					const bool selected = in_range(si, m_selection, m_unitSize);
					if (selected)
						l_fill(m_selectionColor);
					if (in_range(si, hoverStart, m_unitSize))
						l_fill(m_hoverColor);
					if (const Watch* w = watch_at(si))
					{
						gfx.fillRect({ hexCell.x + 1, y + l.rowH - 2.0f, hexW - 2.0f, 2.0f }, w->color);
						gfx.fillRect({ asciiCell.x, y + l.rowH - 2.0f, asciiW, 2.0f }, w->color);
					}

					const i32 aliasIndex = (index - m_start < m_aliasFlags.size()) ? m_aliasFlags[index - m_start] : -1;
					Color hexColor = selected ? m_selectionTextColor : (b == 0 ? m_zeroColor : m_hexColor);
					const bool printable = (b >= 32 && b < 127);
					Color asciiColor = selected ? m_selectionTextColor : (printable ? m_asciiColor : m_asciiDotColor);
					if (aliasIndex >= 0 && !selected)
					{
						hexColor = m_aliases[aliasIndex].color;
						hexColor.a = 255;
						asciiColor = hexColor;
					}
					gfx.drawString(m_hexStrings[b], { hexCell.x + (hexW - m_hexWidths[b]) * 0.5f, ty }, hexColor, m_gridFontSize);
					gfx.drawString(m_asciiStrings[b], { asciiCell.x + (asciiW - m_asciiWidths[b]) * 0.5f, ty }, asciiColor, m_gridFontSize);
				}
			}

			if (m_searchCurrent >= 0 && match_at(cast<StreamIndex>(m_searchCurrent)))
				draw_range_outline(gfx, m_searchCurrent, patternLen, m_searchBorderColor);
			if (hoverStart >= 0)
				draw_range_outline(gfx, hoverStart, m_unitSize, m_hoverBorderColor);
		}

		void HexEditor::draw_range_outline(ogfx::BasicRenderer2D& gfx, i64 start, i64 length, const Color& color)
		{
			// One rectangle per row segment, in both columns.
			const Layout& l = m_layout;
			const f32 top = l.grid.y + l.headerH;
			const f32 hexW = m_metrics.hexCellW, asciiW = m_metrics.asciiCellW;
			const i64 end = std::min<i64>(start + length, cast<i64>(m_data.size()));
			for (i64 i = std::max<i64>(start, m_start); i < end; )
			{
				const i64 rel = i - cast<i64>(m_start);
				const i64 row = rel / l.bytesPerRow;
				const i64 col = rel % l.bytesPerRow;
				if (row >= l.rows)
					break;
				const i64 segEnd = std::min<i64>(end, i + (l.bytesPerRow - col));
				const f32 x0 = hex_x(cast<u32>(col));
				const f32 x1 = hex_x(cast<u32>(col + (segEnd - i) - 1)) + hexW;
				gfx.drawRect({ x0, top + row * l.rowH, x1 - x0, l.rowH }, color, 1);
				gfx.drawRect({ l.asciiX + col * asciiW, top + row * l.rowH, (segEnd - i) * asciiW, l.rowH }, color, 1);
				i = segEnd;
			}
		}

		void HexEditor::draw_scrollbar(ogfx::BasicRenderer2D& gfx)
		{
			const Rectangle& track = m_layout.scrollbar;
			if (track.h <= 0)
				return;
			gfx.fillRect(track, m_scrollTrackColor);
			// Markers: where the fixed highlight, the selection and the watches are.
			const f64 total = std::max<f64>(1.0, cast<f64>(m_data.size()));
			auto l_marker = [&](i64 index, const Color& color) {
				if (index < 0)
					return;
				const f32 y = track.y + cast<f32>(cast<f64>(index) / total) * track.h;
				gfx.fillRect({ track.x, std::min(y, track.y + track.h - 2.0f), track.w, 2.0f }, color);
			};
			for (const auto& h : m_highlights)
				l_marker(h.index, h.color);
			// Search results: at most one marker per pixel row, however many matches.
			i32 lastY = -1;
			Color searchMarker = m_searchMatchColor;
			searchMarker.a = 255;
			for (StreamIndex index : m_searchResults)
			{
				const i32 y = cast<i32>(cast<f64>(index) / total * track.h);
				if (y == lastY)
					continue;
				lastY = y;
				l_marker(index, searchMarker);
			}
			for (const auto& w : m_watches)
				l_marker(w.index, w.color);
			l_marker(m_selection, m_selectionColor);
			l_marker(m_fixedHighlight, m_scrollMarkerColor);
			const Rectangle thumb = scrollbar_thumb();
			Color thumbColor = m_scrollThumbColor;
			if (m_scrollDragging)
				thumbColor.a = 255;
			gfx.fillRoundRect(thumb, thumbColor, std::min(4.0f, thumb.w * 0.5f));
		}

		void HexEditor::draw_status(ogfx::BasicRenderer2D& gfx)
		{
			const Rectangle& s = m_layout.status;
			gfx.fillRect(s, m_statusBackgroundColor);
			const f32 lineH = m_metrics.charH + 2.0f;
			f32 x = s.x + 8.0f, y = s.y + 5.0f;
			auto l_part = [&](const String& label, const String& value) {
				if (label != "")
				{
					gfx.drawString(label, { x, y }, m_statusLabelColor, m_gridFontSize);
					x += gfx.getStringDimensions(label, m_gridFontSize).x + 5.0f;
				}
				gfx.drawString(value, { x, y }, m_statusTextColor, m_gridFontSize);
				x += gfx.getStringDimensions(value, m_gridFontSize).x + 18.0f;
			};
			auto l_newLine = [&](void) { x = s.x + 8.0f; y += lineH; };

			// Line 1: the selection and its value.
			if (hasSelection())
			{
				const StreamIndex sel = getSelection();
				const u8 n = cast<u8>(std::min<u64>(m_unitSize, m_data.size() - sel));
				const u64 v = readValue(sel, n);
				l_part("Sel", formatAddress(m_baseAddress + sel) + String(" [").add(cast<i32>(n)).add("B ").add(m_bigEndian ? "BE]" : "LE]"));
				l_part("hex", String("0x") + hex_string(v, n * 2));
				l_part("u", String(std::to_string(v)));
				l_part("s", String(std::to_string(sign_extend(v, n))));
				if (n == 4)
				{
					f32 f; u32 bits = cast<u32>(v);
					std::memcpy(&f, &bits, 4);
					l_part("f32", float_string(f, 9));
				}
				else if (n == 8)
				{
					f64 d;
					std::memcpy(&d, &v, 8);
					l_part("f64", float_string(d, 17));
				}
				else if (n == 1)
					l_part("chr", (v >= 32 && v < 127) ? String("'") + m_asciiStrings[v] + "'" : String("-"));
				StreamIndex aliasStart = 0;
				u32 aliasLength = 0;
				const Alias* alias = getAliasAt(sel, &aliasStart, &aliasLength);
				l_newLine();
				l_part("bin", binary_string(v, n, true));
				if (alias)
				{
					Color c = alias->color;
					c.a = 255;
					gfx.drawString("alias", { x, y }, m_statusLabelColor, m_gridFontSize);
					x += gfx.getStringDimensions("alias", m_gridFontSize).x + 5.0f;
					const String text = alias->name + String("  (") + alias->pattern + String(" at ") + formatAddress(m_baseAddress + aliasStart) + String(")");
					gfx.drawString(text, { x, y }, c, m_gridFontSize);
					x += gfx.getStringDimensions(text, m_gridFontSize).x + 18.0f;
				}
			}
			else
			{
				l_part("", "No selection: click a byte (double-click to edit)");
				l_newLine();
			}
			l_newLine();

			// Line 3: hover, view, data and messages.
			if (m_hover >= 0)
			{
				String info = formatAddress(m_baseAddress + cast<u64>(m_hover));
				info += String("  (+").add(std::to_string(m_hover)).add(")");
				if (const Highlight* h = highlight_at(cast<StreamIndex>(m_hover)); h && h->label != "")
					info += String("  ") + h->label;
				if (const Watch* w = watch_at(cast<StreamIndex>(m_hover)))
					info += String("  watch: ") + w->label;
				const i64 visibleIndex = m_hover - cast<i64>(m_start);
				if (visibleIndex >= 0 && visibleIndex < cast<i64>(m_aliasFlags.size()) && m_aliasFlags[visibleIndex] >= 0)
					info += String("  alias: ") + m_aliases[m_aliasFlags[visibleIndex]].name;
				l_part("Hover", info);
			}
			const u64 viewEnd = std::min<u64>(cast<u64>(m_start) + cast<u64>(m_layout.rows) * m_layout.bytesPerRow, m_data.size());
			l_part("View", formatAddress(m_baseAddress + m_start) + "-" + formatAddress(m_baseAddress + (viewEnd > 0 ? viewEnd - 1 : 0)));
			l_part("Data", String(std::to_string(m_data.size())).add(" B").add(isEditable() ? "" : " (read only)"));
			if (!m_searchResults.empty())
			{
				String found = String(std::to_string(m_searchResults.size())).add(m_searchTruncated ? "+" : "");
				auto it = std::lower_bound(m_searchResults.begin(), m_searchResults.end(), cast<StreamIndex>(std::max<i64>(0, m_searchCurrent)));
				if (m_searchCurrent >= 0 && it != m_searchResults.end() && cast<i64>(*it) == m_searchCurrent)
					found = String(std::to_string(it - m_searchResults.begin() + 1)).add("/") + found;
				l_part("Found", found);
			}
			if (m_message != "" && now_ms() < m_messageUntilMs)
				l_part("", m_message);
		}



		// ===================================================================
		// HexTableView
		// ===================================================================
		HexTableView::HexTableView(Window& window, HexEditor& editor) : Widget({ 0, 0, 400, 200 }, window), m_editor(editor)
		{
			setTypeName("ogfx::gui::HexTableView");
			setStylesheetCategoryName("hexWatchList");
			enableBackground();
			setBackgroundColor({ 20, 20, 24 });
			setFontSize(16);
			validate();
		}

		void HexTableView::setColumns(const stdvec<String>& titles)
		{
			m_columns = titles;
			resetColumnWidths();
		}

		i32 HexTableView::getRowCount(void) const
		{
			return m_rowCount ? std::max(0, m_rowCount()) : 0;
		}

		void HexTableView::setSelectedRow(i32 row, bool notify)
		{
			const i32 count = getRowCount();
			m_selectedRow = (row >= 0 && row < count) ? row : -1;
			if (m_selectedRow >= 0)
				ensureRowVisible(m_selectedRow);
			if (notify && m_selectedRow >= 0 && m_onSelect)
				m_onSelect(m_selectedRow);
		}

		void HexTableView::ensureRowVisible(i32 row)
		{
			const f32 rh = row_height();
			const f32 visible = geth() - rh;
			const f32 top = row * rh;
			if (top < m_scroll)
				m_scroll = top;
			else if (top + rh > m_scroll + visible)
				m_scroll = top + rh - visible;
			m_scroll = std::clamp(m_scroll, 0.0f, max_scroll());
		}

		void HexTableView::applyTheme(const ostd::Stylesheet& theme)
		{
		}

		Vec2 HexTableView::getContentMinSize(void) const
		{
			return { 300, row_height() * 4 };
		}

		f32 HexTableView::row_height(void) const
		{
			return cast<f32>(getFontSize()) + 10.0f;
		}

		f32 HexTableView::row_y(i32 row) const
		{
			return getGlobalPosition().y + row_height() * (row + 1) - m_scroll;
		}

		f32 HexTableView::max_scroll(void) const
		{
			const f32 rh = row_height();
			return std::max(0.0f, getRowCount() * rh - (geth() - rh));
		}

		i32 HexTableView::row_at(const Vec2& pos) const
		{
			const Rectangle b { getGlobalPosition(), getSize() };
			const f32 top = b.y + row_height();
			if (!b.contains(pos, true) || pos.y < top)
				return -1;
			const i32 row = cast<i32>((pos.y - top + m_scroll) / row_height());
			return row < getRowCount() ? row : -1;
		}

		Rectangle HexTableView::remove_rect(i32 row) const
		{
			if (!m_onRemove)
				return { 0, 0, 0, 0 };
			const Rectangle b { getGlobalPosition(), getSize() };
			const f32 rh = row_height();
			return { b.x + b.w - rh, row_y(row) + 3.0f, rh - 6.0f, rh - 6.0f };
		}

		Rectangle HexTableView::swatch_rect(i32 row) const
		{
			if (!m_swatch)
				return { 0, 0, 0, 0 };
			const f32 rh = row_height();
			return { getGlobalPosition().x + 6.0f, row_y(row) + 5.0f, rh - 10.0f, rh - 10.0f };
		}

		void HexTableView::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			const Rectangle b { getGlobalPosition(), getSize() };
			const f32 rh = row_height();
			const i32 fs = getFontSize();
			const HexEditor& ed = m_editor;
			const i32 rows = getRowCount();
			const i32 columns = cast<i32>(m_columns.size());
			m_scroll = std::clamp(m_scroll, 0.0f, max_scroll());
			const i32 firstRow = std::max(0, cast<i32>(m_scroll / rh));
			const i32 lastRow = std::min(rows - 1, cast<i32>((m_scroll + b.h) / rh));

			// Columns as wide as the widest entry seen (only visible rows are measured,
			// the widths only grow: results can be huge).
			if (cast<i32>(m_columnWidths.size()) != columns)
				m_columnWidths.assign(columns, 0.0f);
			for (i32 c = 0; c < columns; c++)
				m_columnWidths[c] = std::max(m_columnWidths[c], gfx.getStringDimensions(m_columns[c], fs).x);
			for (i32 r = firstRow; r <= lastRow && m_cell; r++)
			{
				for (i32 c = 0; c < columns; c++)
					m_columnWidths[c] = std::max(m_columnWidths[c], gfx.getStringDimensions(m_cell(r, c), fs).x);
			}
			const f32 gap = 18.0f;
			stdvec<f32> xs(columns + 1);
			xs[0] = b.x + 8.0f + (m_swatch ? rh : 0.0f);
			for (i32 c = 0; c < columns; c++)
				xs[c + 1] = xs[c] + m_columnWidths[c] + gap;
			const f32 right = b.x + b.w - (m_onRemove ? rh + 4.0f : 4.0f);
			auto l_cell = [&](i32 column, const String& text, f32 y, const Color& color) {
				const f32 x1 = std::min(xs[column + 1] - gap * 0.5f, right);
				if (x1 <= xs[column])
					return;
				gfx.pushClippingRect({ xs[column], y, x1 - xs[column], rh }, true);
				gfx.drawVCenteredString(text, { xs[column], y, x1 - xs[column], rh }, color, fs);
				gfx.popClippingRect();
			};

			gfx.fillRect({ b.x, b.y, b.w, rh }, ed.m_topBarColor);
			for (i32 c = 0; c < columns; c++)
				l_cell(c, m_columns[c], b.y, ed.m_headerColor);
			gfx.pushClippingRect({ b.x, b.y + rh, b.w, b.h - rh }, true);
			if (rows == 0)
				gfx.drawCenteredString(m_emptyText, { b.x, b.y + rh, b.w, b.h - rh }, ed.m_statusLabelColor, fs);
			for (i32 r = firstRow; r <= lastRow; r++)
			{
				const f32 y = row_y(r);
				if (r == m_selectedRow)
					gfx.fillRect({ b.x, y, b.w, rh }, ed.m_selectionColor);
				else if (r == m_hoveredRow)
					gfx.fillRect({ b.x, y, b.w, rh }, ed.m_hoverColor);
				if (m_swatch)
				{
					Color sc = m_swatch(r);
					sc.a = 255;
					const Rectangle sw = swatch_rect(r);
					gfx.fillRoundRect(sw, sc, 3.0f);
					gfx.drawRoundRect(sw, ed.m_statusTextColor, 3.0f, 1);
				}
				for (i32 c = 0; c < columns && m_cell; c++)
					l_cell(c, m_cell(r, c), y, m_cellColor ? m_cellColor(r, c) : ed.m_statusTextColor);
				if (m_onRemove)
				{
					const Rectangle x = remove_rect(r);
					if (r == m_hoveredRow && m_removeHovered)
						gfx.fillRoundRect(x, Color(200, 30, 40), 3.0f);
					const f32 in = 5.0f;
					gfx.drawLine({ { x.x + in, x.y + in }, { x.x + x.w - in, x.y + x.h - in } }, ed.m_statusTextColor, 2);
					gfx.drawLine({ { x.x + x.w - in, x.y + in }, { x.x + in, x.y + x.h - in } }, ed.m_statusTextColor, 2);
				}
			}
			gfx.popClippingRect();
		}

		void HexTableView::onMousePressed(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			const i32 row = row_at(pos);
			if (row < 0)
				return;
			if (m_onRemove && remove_rect(row).contains(pos, true))
			{
				m_onRemove(row);
				m_hoveredRow = -1;
				return;
			}
			setSelectedRow(row, true);
			if (m_onSwatch && swatch_rect(row).contains(pos, true))
			{
				m_onSwatch(row);
				return;
			}
			const u64 now = now_ms();
			if (row == m_lastClickRow && now - m_lastClickMs <= DoubleClickMs)
			{
				m_lastClickMs = 0;
				if (m_onActivate)
					m_onActivate(row);
				return;
			}
			m_lastClickMs = now;
			m_lastClickRow = row;
		}

		void HexTableView::onMouseMoved(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };
			m_hoveredRow = row_at(pos);
			m_removeHovered = m_hoveredRow >= 0 && m_onRemove && remove_rect(m_hoveredRow).contains(pos, true);
		}

		void HexTableView::onMouseExited(const Event& event)
		{
			m_hoveredRow = -1;
			m_removeHovered = false;
		}

		void HexTableView::onMouseScrolled(const Event& event)
		{
			const f32 rh = row_height();
			m_scroll = std::clamp(m_scroll - event.mouse->scrollAmount.y * rh * 3.0f, 0.0f, max_scroll());
			event.handle();
		}

		void HexTableView::onKeyPressed(const Event& event)
		{
			const i32 key = event.keyboard->keyCode;
			const i32 count = getRowCount();
			if (count == 0)
				return;
			const i32 visibleRows = std::max(1, cast<i32>(geth() / row_height()) - 1);
			i32 row = m_selectedRow;
			if (key == KeyCode::Up) row = std::max(0, row - 1);
			else if (key == KeyCode::Down) row = std::min(count - 1, row + 1);
			else if (key == KeyCode::Pageup) row = std::max(0, row - visibleRows);
			else if (key == KeyCode::Pagedown) row = std::min(count - 1, row + visibleRows);
			else if ((key == KeyCode::Return || key == KeyCode::KpEnter) && row >= 0)
			{
				if (m_onActivate)
					m_onActivate(row);
				event.handle();
				return;
			}
			else
				return;
			setSelectedRow(row, true);
			event.handle();
		}
	}
}
