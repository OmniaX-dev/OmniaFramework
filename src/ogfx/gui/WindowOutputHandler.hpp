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

#include <ostd/io/IOHandlers.hpp>
#include <ostd/string/TextStyleParser.hpp>
#include <ogfx/render/BasicRenderer.hpp>

namespace ogfx
{
	class WindowCore;
	class GraphicsWindowOutputHandler : public ostd::OutputHandlerBase
	{
		public: enum class eWrapMode { TripleDots, NewLine };
		public:
			GraphicsWindowOutputHandler(void);
			void attachWindow(WindowCore& window);
			void setMonospaceFont(const String& filePath);
			Vec2 getStringSize(const String& str);
			bool isReady(void);
			void resetCursorPosition(void);
			void resetColors(void);
			void beginFrame(void);

			void setFontSize(i32 fontSize);
			void setConsoleMaxCharacters(const IPoint& size);
			void setConsolePosition(const Vec2& pos);
			void setWrapMode(eWrapMode wrapMode);
			void setPadding(const Rectangle& rect);
			void setDefaultForegroundColor(const Color& color);
			void setDefaultBackgorundColor(const Color& color);
			void setTabWidth(u8 tw);

			i32 getFontSize(void);
			Vec2 getCharacterSize(i32 fontSize = 0);
			Vec2 getConsolePosition(void);
			eWrapMode getWrapMode(void);
			Rectangle getPadding(void);
			Color getDefaultBackgroundColor(void);
			Color getDefaultForegroundColor(void);
			u8 getTabWidth(void);
			Rectangle getConsoleBounds(void);

			GraphicsWindowOutputHandler& bg(const Color& color) override;
			GraphicsWindowOutputHandler& bg(const ostd::ConsoleColors::tConsoleColor& color) override;
			GraphicsWindowOutputHandler& bg(const String& color) override;
			GraphicsWindowOutputHandler& fg(const Color& color) override;
			GraphicsWindowOutputHandler& fg(const ostd::ConsoleColors::tConsoleColor& color) override;
			GraphicsWindowOutputHandler& fg(const String& color) override;

			GraphicsWindowOutputHandler& pChar(char c) override;
			GraphicsWindowOutputHandler& pStyled(const String& styled) override;
			GraphicsWindowOutputHandler& pStyled(const ostd::TextStyleParser::tStyledString& styled) override;
			GraphicsWindowOutputHandler& pStyled(ostd::TextStyleBuilder::IRichStringBase& styled) override;
			GraphicsWindowOutputHandler& pObject(const ostd::BaseObject& bo) override;

			GraphicsWindowOutputHandler& p(const String& se) override;
			GraphicsWindowOutputHandler& p(u8 i) override;
			GraphicsWindowOutputHandler& p(i8 i) override;
			GraphicsWindowOutputHandler& p(u16 i) override;
			GraphicsWindowOutputHandler& p(i16 i) override;
			GraphicsWindowOutputHandler& p(u32 i) override;
			GraphicsWindowOutputHandler& p(i32 i) override;
			GraphicsWindowOutputHandler& p(u64 i) override;
			GraphicsWindowOutputHandler& p(i64 i) override;
			GraphicsWindowOutputHandler& p(f32 f, u8 precision = 0) override;
			GraphicsWindowOutputHandler& p(f64 f, u8 precision = 0) override;

			GraphicsWindowOutputHandler& nl(void) override;
			GraphicsWindowOutputHandler& tab(void) override;
			GraphicsWindowOutputHandler& flush(void) override;
			GraphicsWindowOutputHandler& clear(void) override;
			GraphicsWindowOutputHandler& reset(void) override;

			GraphicsWindowOutputHandler& xy(IPoint position) override;
			GraphicsWindowOutputHandler& xy(i32 x, i32 y) override;
			GraphicsWindowOutputHandler& x(i32 x) override;
			GraphicsWindowOutputHandler& y(i32 y) override;

			IPoint getCursorPosition(void) override;
			void getCursorPosition(i32& outX, i32& outY) override;
			i32 getCursorX(void) override;
			i32 getCursorY(void) override;

			void getConsoleSize(i32& outColumns, i32& outRows) override;
			IPoint getConsoleSize(void) override;

		private:
			void __update_char_size(void);
			void __print_string(const String& str);

		private:
			Color m_backgroundColor;
			Color m_foregroundColor;
			Color m_defaultBackgroundColor { 0, 0, 0, 0 };
			Color m_defaultForegroundColor { 0, 220, 0, 255 };
			Vec2 m_curosrPosition;
			BasicRenderer2D m_renderer;
			WindowCore* m_window { nullptr };
			i32 m_fontSize { 20 };
			Vec2 m_charSize;
			IPoint m_consoleSize { 0, 0 };
			bool m_ready { false };
			Vec2 m_consolePosition { 0.0f, 0.0f };
			eWrapMode m_wrapMode { eWrapMode::TripleDots };
			Rectangle m_padding { 10, 10, 10, 10 };
			u8 m_tabWidth { 8 };
	};
}
