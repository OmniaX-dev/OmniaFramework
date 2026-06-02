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
#include <ostd/string/TextBuffer.hpp>
#include <ostd/utils/Time.hpp>

namespace ogfx
{
	namespace gui
	{
		class TextEdit : public Widget
		{
			public: class CharacterFilter
			{
				public:
					virtual ~CharacterFilter(void) = default;
					inline virtual bool isValidChar(const String& utf8, const ostd::TextBuffer& buffer) = 0;
				protected:
					// Helper: build the string that would result from inserting `utf8`
					// at the current cursor (replacing any selection). Use this in
					// subclasses to ask "is the resulting string a valid prefix?"
					static String predictResult(const String& utf8, const ostd::TextBuffer& buffer);
			};
			public: class IntegerFilter : public CharacterFilter
			{
				public:
					inline IntegerFilter(bool allowNegative = true, i32 maxDigits = -1) : m_allowNegative(allowNegative), m_maxDigits(maxDigits) {}
					bool isValidChar(const String& utf8, const ostd::TextBuffer& buffer) override;
				private:
					bool isValidIntegerPrefix(const String& s) const;
				private:
					bool m_allowNegative;
					i32  m_maxDigits;
			};
			public: class HexadecimalFilter : public CharacterFilter
			{
				public:
					inline HexadecimalFilter(i32 maxDigits = -1) : m_maxDigits(maxDigits) {}
					bool isValidChar(const String& utf8, const ostd::TextBuffer& buffer) override;
				private:
					bool isValidHexPrefix(const String& s) const;
					bool isHexDigit(char c) const;
				private:
					i32  m_maxDigits;
			};
			public: class DecimalFilter : public CharacterFilter
			{
				public:
					inline DecimalFilter(bool allowNegative = true, char separator = '.', i32 maxIntegerDigits = -1, i32 maxFractionDigits = -1) : m_allowNegative(allowNegative), m_separator(separator), m_maxIntDigits(maxIntegerDigits), m_maxFracDigits(maxFractionDigits) {}
					bool isValidChar(const String& utf8, const ostd::TextBuffer& buffer) override;
				private:
					bool isValidDecimalPrefix(const String& s) const;
				private:
					bool m_allowNegative;
					char m_separator;
					i32  m_maxIntDigits;
					i32  m_maxFracDigits;
			};
			public: class DateFilter : public CharacterFilter
			{
				public: enum class eFormat{ DDMMYY, DDMMYYYY, MMDDYY, MMDDYYYY, YYYYMMDD, YYMMDD };
				private: enum class eSlot { Day, Month, Year, Sep };

				public:
					DateFilter(eFormat format = eFormat::DDMMYYYY, char separator = '.') : m_format(format), m_separator(separator) { buildPositionMap(); }
					bool isValidChar(const String& utf8, const ostd::TextBuffer& buffer) override;
				private:
					void buildPositionMap(void);
					bool isValidDatePrefix(const String& s) const;
				private:
					stdvec<eSlot> m_slots;
					eFormat       m_format;
					char          m_separator;
			};
			public: struct TextPadding
			{
				f32 left   { 6.0f };
				f32 right  { 6.0f };
			};
			public: struct GlyphLayout
			{
				stdvec<u32> byteOffsets;   // size N+1: byte offset before each codepoint, plus end
				stdvec<f32> xPositions;    // size N+1: pixel x before each codepoint, plus end-x
				f32         totalWidth { 0 };
				f32         lineHeight { 0 };
				bool        dirty { true };
			};
			public: enum class CursorStyle { Line, Block, Box, Underscore };
			public:
				inline TextEdit(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				TextEdit& create(void);
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;

				void onFocusLost(const Event& event) override;
				void onTextEntered(const Event& event) override;
				void onKeyPressed(const Event& event) override;
				void onMousePressed(const Event& event) override;
				void onMouseReleased(const Event& event) override;
				void onMouseDragged(const Event& event) override;

				inline void useCustomFilter(CharacterFilter* filter) { m_owned_filter.reset(); m_filter = filter;  }
				inline void useDateFilter(DateFilter::eFormat format = DateFilter::eFormat::DDMMYYYY, char separator = '.') { use_filter(std::make_unique<DateFilter>(format, separator)); }
				inline void useIntegerFilter(bool allowNegative = true, i32 maxDigits = -1) { use_filter(std::make_unique<IntegerFilter>(allowNegative, maxDigits)); }
				inline void useHexadecimalFilter(i32 maxDigits = -1) { use_filter(std::make_unique<HexadecimalFilter>(maxDigits)); }
				inline void useDecimalFilter(bool allowNegative = true, char separator = '.', i32 maxIntegerDigits = -1, i32 maxFractionDigits = -1) { use_filter(std::make_unique<DecimalFilter>(allowNegative, separator, maxIntegerDigits, maxFractionDigits)); }

				void setText(const String& text);
				inline String getText(void) const { return m_buffer.text(); }
				inline String getSelectedText(void) const { return m_buffer.selectedText(); }
				void setMaxLength(i32 codepoints);
				inline i32 getMaxLength(void) const { return m_maxLength; }
				inline void setTextPadding(f32 left, f32 right) { m_textPadding.left = left; m_textPadding.right = right; }
				inline TextPadding getTextPadding(void) const { return m_textPadding; }
				inline void setCharacterMask(char mask) { m_charMask = mask; }
				inline char getCharacterMask(void) const { return m_charMask; }
				inline void clearCharacterMask(void) { m_charMask = '\0'; }
				inline bool hasCharacterMask(void) const { return std::isprint(m_charMask); }
				OSTD_BOOL_PARAM_GETSET_E(CursorBlink, m_cursorBlink);
				OSTD_BOOL_PARAM_GETSET_E(CopyOnMask, m_copyOnMask);
				OSTD_PARAM_GETSET(f32, CursorWidth, m_cursorWidth);
				OSTD_PARAM_GETSET(Color, CursorColor, m_cursorColor);
				OSTD_PARAM_GETSET(Color, SelectionColor, m_selectionColor);
				OSTD_PARAM_GETSET(CursorStyle, CursorStyle, m_cursorStyle);

			private:
				void rebuild_layout(BasicRenderer2D& gfx);
				void ensure_cursor_visible(const Rectangle& bounds);
				f32 layout_x_for_byte(u32 byte_offset) const;
				u32 byte_offset_for_pixel_x(f32 widget_local_x) const;
				String clamp_input_to_max_length(const String& utf8) const;
				inline void use_filter(std::unique_ptr<CharacterFilter> f) { m_owned_filter = std::move(f); m_filter = m_owned_filter.get(); }

			private:
				ostd::TextBuffer m_buffer { "", true };
				ostd::BasicCounter m_cursorBlinkTimer { 30 };
				ostd::BasicCounter m_keyRepeatTimer { 1 };
				ostd::BasicCounter m_doubleClickTimer { 24 };
				std::unique_ptr<CharacterFilter> m_owned_filter;  // may be null
				CharacterFilter* m_filter { nullptr }; // observer, always used

				f32 m_lastClickLocalX { 0.0f };
				bool m_lastClickValid { false };
				bool m_mouseSelecting { false };
				f32  m_lastMouseLocalX { 0.0f };
				f32 m_scrollX { 0.0f };
				String m_lastChar { "" };
				i32 m_lastKeyCode { 0 };
				bool m_cursorState { false };
				GlyphLayout m_layout;
				bool m_copyOnMask { false };

				i32 m_maxLength { -1 };
				TextPadding m_textPadding;
				char m_charMask { '\0' };
				bool m_cursorBlink { true };
				f32 m_cursorWidth { 2 };
				Color m_cursorColor { Colors::Crimson };
				Color m_selectionColor { 60, 110, 200, 200 };
				CursorStyle m_cursorStyle { CursorStyle::Line };
		};

	}
}
