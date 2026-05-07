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
					inline virtual bool isValidChar(const String& utf8) { return true; }
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
			public:
				inline TextEdit(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				TextEdit& create(void);
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onUpdate(void) override;
				void onFocusGained(const Event& event) override;
				void onFocusLost(const Event& event) override;
				void onTextEntered(const Event& event) override;
				void onKeyPressed(const Event& event) override;
				void onKeyReleased(const Event& event) override;
				void onMousePressed(const Event& event) override;
				void onMouseReleased(const Event& event) override;
				void onMouseDragged(const Event& event) override;

				inline void setText(const String& text) { m_buffer.setText(text); }

			private:
				void rebuild_layout(BasicRenderer2D& gfx);
				void ensure_cursor_visible(const Rectangle& bounds);
				f32 layout_x_for_byte(u32 byte_offset) const;
				u32 byte_offset_for_pixel_x(f32 widget_local_x) const;

			private:
				ostd::TextBuffer m_buffer { "", true };
				ostd::BasicCounter m_cursorBlinkTimer { 30 };
				ostd::BasicCounter m_keyRepeatTimer { 1 };
				ostd::BasicCounter m_doubleClickTimer { 24 };  // ~400ms at 60fps
				f32 m_lastClickLocalX { 0.0f };
				bool m_lastClickValid { false };
				bool m_mouseSelecting { false };
				f32  m_lastMouseLocalX { 0.0f };
				CharacterFilter* m_charFilter { nullptr };
				String m_lastChar { "" };
				i32 m_lastKeyCode { 0 };
				bool m_cursorBlink { true };
				bool m_cursorState { false };
				f32 m_cursorWidth { 2 };
				Color m_cursorColor { Colors::Red };
				GlyphLayout  m_layout;
				f32 m_scrollX { 0.0f };
				TextPadding m_textPadding;
				Color m_selectionColor { 60, 110, 200, 200 };  // bluish, semi-transparent
		};
	}
}
