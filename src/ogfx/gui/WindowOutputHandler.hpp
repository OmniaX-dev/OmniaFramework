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
	class GraphicsWindowOutputHandler : public ostd::OutputHandlerBase
	{
		public: enum class eWrapMode { TripleDots, NewLine };
		public:
			GraphicsWindowOutputHandler(void);
			void attachWindow(GraphicsWindow& window);
			void setMonospaceFont(const ostd::String& filePath);
			ostd::Vec2 getStringSize(const ostd::String& str);
			bool isReady(void);
			void resetCursorPosition(void);
			void resetColors(void);
			void beginFrame(void);

			void setFontSize(int32_t fontSize);
			void setConsoleMaxCharacters(const ostd::IPoint& size);
			void setConsolePosition(const ostd::Vec2& pos);
			void setWrapMode(eWrapMode wrapMode);
			void setPadding(const ostd::Rectangle& rect);
			void setDefaultForegroundColor(const ostd::Color& color);
			void setDefaultBackgorundColor(const ostd::Color& color);
			void setTabWidth(uint8_t tw);

			int32_t getFontSize(void);
			ostd::Vec2 getCharacterSize(int32_t fontSize = 0);
			ostd::Vec2 getConsolePosition(void);
			eWrapMode getWrapMode(void);
			ostd::Rectangle getPadding(void);
			ostd::Color getDefaultBackgroundColor(void);
			ostd::Color getDefaultForegroundColor(void);
			uint8_t getTabWidth(void);
			ostd::Rectangle getConsoleBounds(void);

			GraphicsWindowOutputHandler& bg(const ostd::Color& color) override;
			GraphicsWindowOutputHandler& bg(const ostd::ConsoleColors::tConsoleColor& color) override;
			GraphicsWindowOutputHandler& bg(const ostd::String& color) override;
			GraphicsWindowOutputHandler& fg(const ostd::Color& color) override;
			GraphicsWindowOutputHandler& fg(const ostd::ConsoleColors::tConsoleColor& color) override;
			GraphicsWindowOutputHandler& fg(const ostd::String& color) override;

			GraphicsWindowOutputHandler& pChar(char c) override;
			GraphicsWindowOutputHandler& pStyled(const ostd::String& styled) override;
			GraphicsWindowOutputHandler& pStyled(const ostd::TextStyleParser::tStyledString& styled) override;
			GraphicsWindowOutputHandler& pStyled(ostd::TextStyleBuilder::IRichStringBase& styled) override;
			GraphicsWindowOutputHandler& pObject(const ostd::BaseObject& bo) override;

			GraphicsWindowOutputHandler& p(const ostd::String& se) override;
			GraphicsWindowOutputHandler& p(uint8_t i) override;
			GraphicsWindowOutputHandler& p(int8_t i) override;
			GraphicsWindowOutputHandler& p(uint16_t i) override;
			GraphicsWindowOutputHandler& p(int16_t i) override;
			GraphicsWindowOutputHandler& p(uint32_t i) override;
			GraphicsWindowOutputHandler& p(int32_t i) override;
			GraphicsWindowOutputHandler& p(uint64_t i) override;
			GraphicsWindowOutputHandler& p(int64_t i) override;
			GraphicsWindowOutputHandler& p(float f, uint8_t precision = 0) override;
			GraphicsWindowOutputHandler& p(double f, uint8_t precision = 0) override;

			GraphicsWindowOutputHandler& nl(void) override;
			GraphicsWindowOutputHandler& tab(void) override;
			GraphicsWindowOutputHandler& flush(void) override;
			GraphicsWindowOutputHandler& clear(void) override;
			GraphicsWindowOutputHandler& reset(void) override;

			GraphicsWindowOutputHandler& xy(ostd::IPoint position) override;
			GraphicsWindowOutputHandler& xy(int32_t x, int32_t y) override;
			GraphicsWindowOutputHandler& x(int32_t x) override;
			GraphicsWindowOutputHandler& y(int32_t y) override;

			ostd::IPoint getCursorPosition(void) override;
			void getCursorPosition(int32_t& outX, int32_t& outY) override;
			int32_t getCursorX(void) override;
			int32_t getCursorY(void) override;

			void getConsoleSize(int32_t& outColumns, int32_t& outRows) override;
			ostd::IPoint getConsoleSize(void) override;

		private:
			void __update_char_size(void);
			void __print_string(const ostd::String& str);

		private:
			ostd::Color m_backgroundColor;
			ostd::Color m_foregroundColor;
			ostd::Color m_defaultBackgroundColor { 0, 0, 0, 0 };
			ostd::Color m_defaultForegroundColor { 0, 220, 0, 255 };
			ostd::Vec2 m_curosrPosition;
			BasicRenderer2D m_renderer;
			GraphicsWindow* m_window { nullptr };
			int32_t m_fontSize { 20 };
			ostd::Vec2 m_charSize;
			ostd::IPoint m_consoleSize { 0, 0 };
			bool m_ready { false };
			ostd::Vec2 m_consolePosition { 0.0f, 0.0f };
			eWrapMode m_wrapMode { eWrapMode::TripleDots };
			ostd::Rectangle m_padding { 10, 10, 10, 10 };
			uint8_t m_tabWidth { 8 };
	};
}
