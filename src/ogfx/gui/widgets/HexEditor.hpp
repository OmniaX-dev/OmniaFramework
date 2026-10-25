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
#include <ogfx/gui/widgets/Button.hpp>
#include <ogfx/gui/widgets/Label.hpp>
#include <ogfx/gui/widgets/CheckBox.hpp>
#include <ogfx/gui/widgets/Text.hpp>
#include <ogfx/gui/widgets/LayoutContainers.hpp>
#include <ogfx/gui/widgets/SubWindow.hpp>
#include <ogfx/gui/widgets/ColorPickerSkins.hpp>
#include <ostd/utils/Signals.hpp>
#include <array>
#include <span>

namespace ogfx
{
	namespace gui
	{
		using StreamIndex = ostd::StreamIndex;
		class HexEditor;

		// A small virtualized table used by the HexEditor's windows (watchlist,
		// regions, search results): rows come from callbacks, nothing is copied.
		// Optional color swatch column and per-row remove button. Double-click (or
		// Return when focused) activates a row; Up/Down move the selection.
		class HexTableView : public Widget
		{
			public: using RowCountFunction = std::function<i32(void)>;
			public: using CellFunction = std::function<String(i32 row, i32 column)>;
			public: using CellColorFunction = std::function<Color(i32 row, i32 column)>;
			public: using SwatchFunction = std::function<Color(i32 row)>;
			public: using RowCallback = std::function<void(i32 row)>;
			public:
				HexTableView(Window& window, HexEditor& editor);
				void setColumns(const stdvec<String>& titles);
				inline void setRowCountFunction(RowCountFunction fn) { m_rowCount = std::move(fn); }
				inline void setCellFunction(CellFunction fn) { m_cell = std::move(fn); }
				inline void setCellColorFunction(CellColorFunction fn) { m_cellColor = std::move(fn); }
				// nullptr = no swatch column.
				inline void setSwatchFunction(SwatchFunction fn) { m_swatch = std::move(fn); }
				inline void setSelectCallback(RowCallback cb) { m_onSelect = std::move(cb); }
				inline void setActivateCallback(RowCallback cb) { m_onActivate = std::move(cb); }
				// Setting a remove callback shows a remove button on every row.
				inline void setRemoveCallback(RowCallback cb) { m_onRemove = std::move(cb); }
				inline void setSwatchCallback(RowCallback cb) { m_onSwatch = std::move(cb); }
				inline void setEmptyText(const String& text) { m_emptyText = text; }
				i32 getRowCount(void) const;
				inline i32 getSelectedRow(void) const { return m_selectedRow; }
				void setSelectedRow(i32 row, bool notify = false);
				void ensureRowVisible(i32 row);
				// Forget the measured column widths (e.g. after new search results).
				inline void resetColumnWidths(void) { m_columnWidths.assign(m_columns.size(), 0.0f); }

				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMousePressed(const Event& event) override;
				void onMouseMoved(const Event& event) override;
				void onMouseExited(const Event& event) override;
				void onMouseScrolled(const Event& event) override;
				void onKeyPressed(const Event& event) override;
				Vec2 getContentMinSize(void) const override;

			private:
				i32 row_at(const Vec2& pos) const;
				Rectangle remove_rect(i32 row) const;
				Rectangle swatch_rect(i32 row) const;
				f32 row_height(void) const;
				f32 row_y(i32 row) const;
				f32 max_scroll(void) const;

			private:
				HexEditor& m_editor;
				stdvec<String> m_columns;
				stdvec<f32> m_columnWidths;
				RowCountFunction m_rowCount { nullptr };
				CellFunction m_cell { nullptr };
				CellColorFunction m_cellColor { nullptr };
				SwatchFunction m_swatch { nullptr };
				RowCallback m_onSelect { nullptr };
				RowCallback m_onActivate { nullptr };
				RowCallback m_onRemove { nullptr };
				RowCallback m_onSwatch { nullptr };
				String m_emptyText { "" };
				i32 m_selectedRow { -1 };
				i32 m_hoveredRow { -1 };
				bool m_removeHovered { false };
				f32 m_scroll { 0 };
				u64 m_lastClickMs { 0 };
				i32 m_lastClickRow { -1 };
		};

		// ============================================================================
		// HexEditor — a hex view/editor for byte streams, made for inspecting
		// memory (e.g. a virtual machine's RAM, stack or code).
		//
		//   editor.setData(ram, 0x0000, 16, 0, pc, 4, "RAM");  // like Memory::printByteStream
		//
		// setData() mirrors ostd::Memory::printByteStream() and is cheap (it only
		// stores a view of the data, nothing is copied), so it can be called every
		// frame. It also moves the view to `start`, though; to refresh the data
		// without fighting the user's scrolling, use updateData() / setHighlight().
		// The data must outlive the editor (or the next setData()/updateData()).
		//
		// Indices vs addresses: every index is an offset into the data, like in
		// printByteStream(). Addresses are what's displayed: baseAddress + index
		// (setBaseAddress()), e.g. when the span is a guest region at 0x8000.
		// jumpTo() and the address field take addresses.
		//
		// Mouse: hovering highlights `unit size` bytes (1, 2, 4 or 8) starting at the
		// byte under the cursor (in the hex or the ASCII column), clicking selects
		// them, double-clicking opens the edit window (dec / hex / bin input).
		// Wheel scrolls, the scrollbar on the right covers the whole data.
		// Keyboard (when focused): arrows move the selection, PageUp/PageDown and
		// Home/End scroll, Return edits the selection.
		//
		// Top bar: address field + Go / Back, row navigation (-10, -1, +1, +10),
		// unit size, endianness, alignment, follow-highlight, change flashing and
		// the watchlist buttons. Bottom bar: the selection's address and its value
		// as hex / unsigned / signed / float / binary / ASCII, plus hover info.
		//
		// Regions: the named highlights (addHighlight()) are listed in the Regions
		// window; double-clicking one jumps to its start.
		//
		// Search (Ctrl+F, F3 / Shift+F3): hex ("DE AD ?? EF", ?? = any byte) or
		// ASCII (optionally case-insensitive); find next / previous / all. Matches
		// get their own highlight, on the scrollbar too, and are re-checked against
		// the live data when drawn.
		//
		// Aliases: names for byte patterns ("B1" = "ret", "F1 0D" = "MemMap Int
		// Vector", "??" = any byte). Matching bytes are drawn in the alias' color and
		// the status bar names the alias under the selection / mouse. Overlaps are
		// resolved like a lexer: scanning left to right, the longest alias matching
		// at a position wins (then the one with fewer wildcards, then the older
		// one) and covers its bytes, so with "B1" and "B1 50" defined, B1 50 is one
		// "B1 50" token, and B1 B1 is two "B1" tokens (or one "B1 B1", if defined).
		// The scan starts a bit before the visible area, rounded down to a 1 KiB
		// boundary (not at 0, which would be too slow on big data), so in long
		// ambiguous runs the token boundaries can differ from a whole-data scan.
		//
		// Watches: addWatch() (or the "Watch" button for the selection) tracks a
		// value; when it changes, WatchEvent is emitted as the user data of the
		// signal getWatchSignalID() (WatchValueChangedSignal by default) and passed
		// to the watch callback. Values are compared once per frame; call
		// checkWatches() yourself (e.g. after every emulated instruction) to catch
		// every change.
		//
		// Theme category: "hexEditor" (see DefaultThemeDark.oss).
		// ============================================================================
		class HexEditor : public Widget
		{
			public: struct Highlight
			{
				i32 id { -1 };
				StreamIndex index { 0 };
				u32 size { 1 };
				Color color { 255, 0, 0, 120 };
				String label { "" };
			};
			public: struct Watch
			{
				StreamIndex index { 0 };
				u8 size { 1 };
				u64 value { 0 };
				String label { "" };
				Color color { 0, 220, 180 };
			};
			public: struct Alias
			{
				i32 id { -1 };
				String pattern { "" };		// as typed, e.g. "F1 0D"
				String name { "" };
				Color color { 255, 255, 255 };
				stdvec<i16> bytes;			// parsed pattern, -1 = any byte
				u32 wildcards { 0 };
			};
			public: class WatchEvent : public ostd::Object
			{
				public:
					inline WatchEvent(void) { setTypeName("ogfx::gui::HexEditor::WatchEvent"); validate(); }
					HexEditor* editor { nullptr };
					i32 watchIndex { -1 };
					StreamIndex index { 0 };	// offset into the data
					u64 address { 0 };			// baseAddress + index
					u8 size { 1 };				// bytes
					u64 oldValue { 0 };
					u64 newValue { 0 };
					bool bigEndian { false };
					String label { "" };
			};
			public: using WatchCallback = std::function<void(const WatchEvent&)>;
			// Called after the user edited a value. For read-only data (setData() with a
			// const span) it does the writing itself and returns whether it did.
			public: using WriteCallback = std::function<bool(StreamIndex index, u64 value, u8 size)>;
			public: using SelectionCallback = std::function<void(HexEditor&, StreamIndex index, u8 size)>;
			public: enum class eFormat : u8 { Dec = 0, Hex, Bin };
			public: enum class eSearchMode : u8 { Hex = 0, Ascii };

			public:
				inline static const u32 WatchValueChangedSignal { ostd::BuiltinSignals::CustomSignalBase + 0x4845 };
				inline static constexpr i32 MaxHistory { 64 };
				inline static constexpr size_t DefaultMaxSearchResults { 100000 };

			public:
				HexEditor(Window& window);
				~HexEditor(void) override;

				// ============================ DATA (printByteStream-like) ============================
				// lineLen = bytes per row (0 = the largest power of two that fits), nRows = rows shown
				// (0 = as many as fit), addrHighlight/highlightRange = the fixed highlight
				// (-1 = none), title = shown in the header's corner.
				void setData(std::span<const i8> data, StreamIndex start, u8 lineLen = 16, u16 nRows = 0, i32 addrHighlight = -1, u32 highlightRange = 1, const String& title = "");
				// Same, but the data can be edited from the editor.
				void setData(std::span<i8> data, StreamIndex start, u8 lineLen = 16, u16 nRows = 0, i32 addrHighlight = -1, u32 highlightRange = 1, const String& title = "");
				// Swaps the data without touching the view.
				void updateData(std::span<const i8> data);
				void updateData(std::span<i8> data);
				void setHighlight(i32 addrHighlight, u32 highlightRange = 1);
				inline void clearHighlight(void) { setHighlight(-1); }
				inline std::span<const i8> getData(void) const { return m_data; }
				inline bool isEditable(void) const { return m_mutableData != nullptr; }
				inline void setTitle(const String& title) { m_title = title; }
				inline String getTitle(void) const { return m_title; }
				inline void setLineLength(u8 lineLen) { m_lineLen = lineLen; }
				inline void setRowCount(u16 nRows) { m_rowCount = nRows; }
				inline void setBaseAddress(u64 base) { m_baseAddress = base; }
				inline u64 getBaseAddress(void) const { return m_baseAddress; }

				// =================================== VIEW ===================================
				// First index shown (top-left).
				void setStart(StreamIndex start);
				inline StreamIndex getStart(void) const { return m_start; }
				void scrollRows(i64 rows);
				// Scrolls so the address is in the top row (and selects it). Returns false if
				// the address is outside the data.
				bool jumpTo(u64 address, bool select = true, bool addToHistory = true);
				bool goBack(void);
				// Scrolls the least needed to show [index, index + size).
				void ensureVisible(StreamIndex index, u32 size = 1);
				// Global bounds of a byte's cell in the hex (or ASCII) column; empty when the
				// byte isn't visible. Handy for overlays and tooltips.
				Rectangle getCellBounds(StreamIndex index, bool ascii = false) const;
				// Bytes per row actually used (resolves lineLen = 0).
				u32 getBytesPerRow(void) const;
				i32 getVisibleRows(void) const;

				// ================================= SELECTION =================================
				void select(StreamIndex index);
				inline void clearSelection(void) { m_selection = -1; }
				inline bool hasSelection(void) const { return m_selection >= 0 && cast<u64>(m_selection) < m_data.size(); }
				inline StreamIndex getSelection(void) const { return cast<StreamIndex>(std::max<i64>(0, m_selection)); }
				// 1, 2, 4 or 8: hover/selection size and the size of edited/watched values.
				void setUnitSize(u8 size);
				inline u8 getUnitSize(void) const { return m_unitSize; }
				void setBigEndian(bool bigEndian = true);
				inline bool isBigEndian(void) const { return m_bigEndian; }
				void enableAlign(bool align = true);
				inline bool isAlignEnabled(void) const { return m_align; }
				void enableFollowHighlight(bool follow = true);
				inline bool isFollowHighlightEnabled(void) const { return m_followHighlight; }
				void enableChangeFlash(bool flash = true);
				inline bool isChangeFlashEnabled(void) const { return m_changeFlash; }

				// =================================== VALUES ===================================
				// Reads `size` bytes (clamped to the data) with the current endianness.
				u64 readValue(StreamIndex index, u8 size) const;
				// Writes with the current endianness (to editable data and/or through the
				// write callback). Returns false if nothing could be written.
				bool writeValue(StreamIndex index, u64 value, u8 size);

				// ================================= HIGHLIGHTS =================================
				// Extra colored ranges (stack, code, registers...); the label shows up in the
				// status bar when hovered. Returns an id for removeHighlight().
				i32 addHighlight(StreamIndex index, u32 size, const Color& color, const String& label = "");
				bool updateHighlight(i32 id, StreamIndex index, u32 size);
				bool removeHighlight(i32 id);
				inline void clearHighlights(void) { m_highlights.clear(); }

				// =================================== WATCHES ===================================
				// A color with alpha 0 picks a random one.
				i32 addWatch(StreamIndex index, u8 size, const String& label = "", const Color& color = Color(0, 0, 0, 0));
				bool setWatchLabel(i32 watchIndex, const String& label);
				bool setWatchColor(i32 watchIndex, const Color& color);
				bool removeWatch(i32 watchIndex);
				void clearWatches(void);
				inline const stdvec<Watch>& getWatches(void) const { return m_watches; }
				// Compares every watch with the data, emitting the signal for each change.
				void checkWatches(void);
				inline void setWatchSignalID(u32 id) { m_watchSignalID = id; }
				inline u32 getWatchSignalID(void) const { return m_watchSignalID; }

				// =================================== ALIASES ===================================
				// Hex pattern as in setSearchPattern(). Color alpha 0 = random. Returns the id,
				// or -1 for an invalid pattern.
				i32 addAlias(const String& pattern, const String& name, const Color& color = Color(0, 0, 0, 0));
				bool updateAlias(i32 id, const String& pattern, const String& name, const Color& color);
				bool removeAlias(i32 id);
				void clearAliases(void);
				inline const stdvec<Alias>& getAliases(void) const { return m_aliases; }
				// The alias token covering `index` (resolved as when drawing), or nullptr.
				const Alias* getAliasAt(StreamIndex index, StreamIndex* outStart = nullptr, u32* outLength = nullptr) const;
				inline void enableAliasColors(bool enable = true) { m_aliasColors = enable; }
				inline bool isAliasColorsEnabled(void) const { return m_aliasColors; }

				// =================================== SEARCH ===================================
				// Hex: byte pairs, optionally separated by spaces/commas, "??" = any byte
				// ("0x" prefixes are ignored). Ascii: the text's bytes. Returns false (and
				// clears the search) for an invalid or empty pattern.
				bool setSearchPattern(const String& text, eSearchMode mode, bool matchCase = true);
				inline u32 getSearchPatternLength(void) const { return cast<u32>(m_searchPattern.size()); }
				// First match at or after `from` (at or before, backwards); -1 if none.
				i64 find(i64 from, bool backwards = false, bool wrap = true) const;
				// Moves to (and selects) the next/previous match from the selection. -1 if none.
				i64 findNext(bool backwards = false);
				size_t findAll(size_t maxResults = DefaultMaxSearchResults);
				void clearSearch(void);
				inline const stdvec<StreamIndex>& getSearchResults(void) const { return m_searchResults; }
				inline i64 getCurrentSearchMatch(void) const { return m_searchCurrent; }

				// ================================== CALLBACKS ==================================
				inline void setWatchCallback(WatchCallback callback) { m_watchCallback = std::move(callback); }
				inline void setWriteCallback(WriteCallback callback) { m_writeCallback = std::move(callback); }
				inline void setSelectionCallback(SelectionCallback callback) { m_selectionCallback = std::move(callback); }

				// =================================== WINDOWS ===================================
				// Edit window for the current selection.
				void openEditWindow(void);
				void openWatchList(void);
				void openRegionList(void);
				void openAliasList(void);
				void openSearchWindow(void);
				inline SubWindow& getEditWindow(void) { return m_editWindow; }
				inline SubWindow& getWatchListWindow(void) { return m_watchWindow; }
				inline SubWindow& getRegionWindow(void) { return m_regionWindow; }
				inline SubWindow& getAliasWindow(void) { return m_aliasWindow; }
				inline HexTableView& getAliasListView(void) { return m_aliasList; }
				inline SubWindow& getSearchWindow(void) { return m_searchWindow; }
				inline HexTableView& getWatchListView(void) { return m_watchList; }
				inline HexTableView& getRegionListView(void) { return m_regionList; }
				inline HexTableView& getSearchResultView(void) { return m_searchList; }

				// ================================== HELPERS ==================================
				String formatAddress(u64 address) const;
				// "0x...", "0b..." or plain (in `format`); negative decimals are two's
				// complement. Returns false on invalid input or a value too big for `size`.
				static bool parseValue(const String& text, eFormat format, u8 size, u64& outValue);
				static String formatValue(u64 value, eFormat format, u8 size);

				// ================================== OVERRIDES ==================================
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;
				void onMousePressed(const Event& event) override;
				void onMouseReleased(const Event& event) override;
				void onMouseMoved(const Event& event) override;
				void onMouseDragged(const Event& event) override;
				void onMouseExited(const Event& event) override;
				void onMouseScrolled(const Event& event) override;
				void onKeyPressed(const Event& event) override;
				void onKeyReleased(const Event& event) override;
				Vec2 getContentMinSize(void) const override;

			private: struct Metrics
			{
				i32 fontSize { 0 };
				f32 hexCellW { 0 };
				f32 asciiCellW { 0 };
				f32 charH { 0 };
				f32 digitW { 0 };
			};
			private: struct Layout
			{
				Rectangle grid;			// header + rows (global)
				Rectangle status;		// bottom bar (global)
				Rectangle scrollbar;	// track (global)
				f32 rowH { 0 };
				f32 headerH { 0 };
				f32 addrW { 0 };
				f32 hexX { 0 };
				f32 asciiX { 0 };
				u32 bytesPerRow { 16 };
				i32 rows { 0 };
			};

			private:
				void build_top_bar(void);
				void build_edit_window(void);
				void build_watch_window(void);
				void build_region_window(void);
				void build_search_window(void);
				void build_alias_window(void);
				void rebuild_alias_index(void);
				const Alias* alias_match_at(u64 index) const;
				// Calls fn(start, length, alias) for every alias token touching [from, to).
				void resolve_aliases(u64 from, u64 to, const std::function<void(u64, u32, const Alias&)>& fn) const;
				void select_alias_row(i32 row);
				void set_alias_message(const String& message, bool error);
				static bool parse_hex_pattern(const String& text, stdvec<i16>& out);
				bool match_pattern(u64 index, const stdvec<i16>& pattern, bool ignoreCase) const;
				void select_watch_row(i32 row);
				void set_search_mode(eSearchMode mode);
				bool apply_search_input(void);
				void search_window_find(bool backwards);
				void update_search_status(void);
				bool match_at(StreamIndex index) const;
				void draw_range_outline(ogfx::BasicRenderer2D& gfx, i64 start, i64 length, const Color& color);
				Color next_watch_color(void);
				void update_metrics(ogfx::BasicRenderer2D& gfx);
				void update_layout(void);
				void update_toggles(void);
				void update_change_tracking(f32 dt);
				void refresh_edit_window(void);
				void set_edit_format(eFormat format);
				void apply_edit(void);
				void set_data(std::span<const i8> data, i8* mutableData);
				void notify_selection(void);
				void push_history(StreamIndex index);
				void show_message(const String& message);
				i64 index_at(const Vec2& pos) const;
				StreamIndex unit_start(StreamIndex index) const;
				f32 hex_x(u32 column) const;
				bool in_range(StreamIndex index, i64 start, u32 size) const;
				const Highlight* highlight_at(StreamIndex index) const;
				const Watch* watch_at(StreamIndex index) const;
				void draw_header(ogfx::BasicRenderer2D& gfx);
				void draw_rows(ogfx::BasicRenderer2D& gfx);
				void draw_scrollbar(ogfx::BasicRenderer2D& gfx);
				void draw_status(ogfx::BasicRenderer2D& gfx);
				Rectangle scrollbar_thumb(void) const;
				void scroll_to_thumb(f32 mouseY);
				u64 max_start(void) const;

			private:
				// ===== Data =====
				std::span<const i8> m_data;
				i8* m_mutableData { nullptr };
				StreamIndex m_start { 0 };
				u8 m_lineLen { 16 };
				u16 m_rowCount { 0 };
				i64 m_fixedHighlight { -1 };
				u32 m_fixedHighlightRange { 1 };
				String m_title { "" };
				u64 m_baseAddress { 0 };

				// ===== State =====
				i64 m_hover { -1 };
				i64 m_selection { -1 };
				u8 m_unitSize { 1 };
				bool m_bigEndian { false };
				bool m_align { false };
				bool m_followHighlight { false };
				bool m_changeFlash { true };
				bool m_scrollDragging { false };
				f32 m_scrollGrab { 0 };
				u64 m_lastClickMs { 0 };
				i64 m_lastClickIndex { -1 };
				stdvec<StreamIndex> m_history;
				stdvec<Highlight> m_highlights;
				i32 m_nextHighlightId { 0 };
				stdvec<Watch> m_watches;
				u32 m_watchSignalID { WatchValueChangedSignal };
				WatchCallback m_watchCallback { nullptr };
				WriteCallback m_writeCallback { nullptr };
				SelectionCallback m_selectionCallback { nullptr };
				Metrics m_metrics;
				Layout m_layout;

				// Change flashing: a copy of the visible bytes and how recently each changed.
				StreamIndex m_trackStart { 0 };
				stdvec<u8> m_trackBytes;
				stdvec<f32> m_trackHeat;
				std::span<const i8> m_trackData;
				u64 m_lastUpdateMs { 0 };

				// Formatting caches (strings and their widths, per byte value).
				stdvec<String> m_hexStrings;
				stdvec<String> m_asciiStrings;
				stdvec<f32> m_hexWidths;
				stdvec<f32> m_asciiWidths;

				// ===== Search =====
				stdvec<i16> m_searchPattern;	// -1 = any byte
				bool m_searchIgnoreCase { false };
				eSearchMode m_searchMode { eSearchMode::Hex };		// of the current pattern
				eSearchMode m_searchWindowMode { eSearchMode::Hex };	// selected in the search window
				stdvec<StreamIndex> m_searchResults;
				bool m_searchTruncated { false };
				i64 m_searchCurrent { -1 };
				stdvec<u8> m_searchFlags;		// per visible byte: 0, 1 = match, 2 = current match
				String m_lastSearchText { "" };
				f32 m_nextWatchHue { 0 };

				// ===== Aliases =====
				stdvec<Alias> m_aliases;
				i32 m_nextAliasId { 0 };
				bool m_aliasColors { true };
				// Alias indices by first byte (and those starting with ??), best first.
				std::array<stdvec<i32>, 256> m_aliasByFirstByte;
				stdvec<i32> m_aliasWildFirst;
				u32 m_maxAliasLength { 0 };
				stdvec<i32> m_aliasFlags;		// per visible byte: alias index or -1

				// Short message in the status bar (e.g. an invalid address).
				String m_message { "" };
				u64 m_messageUntilMs { 0 };

				// ===== Top bar =====
				VBoxContainer m_topBar;
				HBoxContainer m_navRow;
				HBoxContainer m_optRow;
				Label m_addrLabel;
				TextEdit m_addrInput;
				Button m_goButton;
				Button m_backButton;
				Button m_up10Button;
				Button m_upButton;
				Button m_downButton;
				Button m_down10Button;
				Label m_sizeLabel;
				Button m_sizeButtons[4];
				Button m_endianButton;
				CheckBox m_alignCheck;
				CheckBox m_followCheck;
				CheckBox m_flashCheck;
				Button m_watchButton;
				Button m_watchListButton;
				Button m_regionsButton;
				Button m_aliasesButton;
				Button m_searchButton;

				// ===== Edit window =====
				SubWindow m_editWindow;
				VBoxContainer m_editBox;
				Label m_editInfo;
				Label m_editCurrent;
				HBoxContainer m_editFormatRow;
				Button m_editFormatButtons[3];
				TextEdit m_editInput;
				Label m_editStatus;
				HBoxContainer m_editButtonRow;
				Button m_editApply;
				Button m_editCancel;
				StreamIndex m_editIndex { 0 };
				u8 m_editSize { 1 };
				eFormat m_editFormat { eFormat::Hex };
				String m_lastEditText { "" };
				std::unique_ptr<TextEdit::CharacterFilter> m_binaryFilter;

				// ===== Watch window =====
				SubWindow m_watchWindow;
				VBoxContainer m_watchBox;
				HexTableView m_watchList;
				HBoxContainer m_watchEditRow;
				Label m_watchNameLabel;
				TextEdit m_watchNameInput;
				Button m_watchRename;
				Label m_watchColorLabel;
				ColorButton m_watchColorButton;
				HBoxContainer m_watchButtonRow;
				Button m_watchJump;
				Button m_watchRemove;
				Button m_watchClear;

				// ===== Region window =====
				SubWindow m_regionWindow;
				VBoxContainer m_regionBox;
				HexTableView m_regionList;
				HBoxContainer m_regionButtonRow;
				Button m_regionJump;

				// ===== Alias window =====
				SubWindow m_aliasWindow;
				VBoxContainer m_aliasBox;
				HexTableView m_aliasList;
				HBoxContainer m_aliasEditRow;
				Label m_aliasPatternLabel;
				TextEdit m_aliasPatternInput;
				Label m_aliasNameLabel;
				TextEdit m_aliasNameInput;
				ColorButton m_aliasColorButton;
				HBoxContainer m_aliasButtonRow;
				Button m_aliasFromSelection;
				Button m_aliasAdd;
				Button m_aliasUpdate;
				Button m_aliasRemove;
				Button m_aliasClear;
				Button m_aliasFind;
				Label m_aliasStatus;

				// ===== Search window =====
				SubWindow m_searchWindow;
				VBoxContainer m_searchBox;
				HBoxContainer m_searchModeRow;
				Button m_searchModeButtons[2];
				CheckBox m_searchCaseCheck;
				TextEdit m_searchInput;
				HBoxContainer m_searchButtonRow;
				Button m_searchPrev;
				Button m_searchNext;
				Button m_searchAll;
				Button m_searchClear;
				Label m_searchStatus;
				HexTableView m_searchList;

				// ===== Theme =====
				Color m_addressColor { 90, 170, 255 };
				Color m_headerColor { 60, 200, 90 };
				Color m_hexColor { 225, 225, 225 };
				Color m_zeroColor { 110, 110, 110 };
				Color m_asciiColor { 240, 220, 90 };
				Color m_asciiDotColor { 90, 90, 90 };
				Color m_titleColor { 220, 120, 230 };
				Color m_selectionColor { 47, 140, 220, 150 };
				Color m_selectionTextColor { 255, 255, 255 };
				Color m_hoverColor { 255, 255, 255, 40 };
				Color m_hoverBorderColor { 255, 255, 255, 140 };
				Color m_highlightColor { 220, 30, 50, 150 };
				Color m_changedColor { 255, 150, 0, 170 };
				Color m_watchColor { 0, 220, 180 };
				Color m_searchMatchColor { 230, 200, 40, 110 };
				Color m_searchCurrentColor { 255, 230, 60, 200 };
				Color m_searchBorderColor { 255, 240, 120 };
				Color m_crosshairColor { 255, 255, 255, 14 };
				Color m_rowStripeColor { 255, 255, 255, 6 };
				Color m_separatorColor { 70, 70, 80 };
				Color m_gridBackgroundColor { 20, 20, 24 };
				Color m_topBarColor { 30, 34, 30 };
				Color m_statusBackgroundColor { 30, 34, 30 };
				Color m_statusTextColor { 210, 220, 210 };
				Color m_statusLabelColor { 120, 150, 120 };
				Color m_scrollTrackColor { 15, 15, 18 };
				Color m_scrollThumbColor { 70, 80, 70 };
				Color m_scrollMarkerColor { 220, 30, 50 };
				i32 m_gridFontSize { 16 };
				f32 m_rowSpacing { 4 };
				f32 m_groupSpacing { 8 };
				u32 m_groupSize { 8 };
				f32 m_scrollbarWidth { 14 };
				f32 m_changeFadeTime { 1.0f };

				friend class HexTableView;
		};
	}
}
