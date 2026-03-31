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

#include "WindowOutputHandler.hpp"
#include "../render/BasicRenderer.hpp"
#include "../../string/TextStyleParser.hpp"

namespace ogfx
{
	using namespace ostd;

	GraphicsWindowOutputHandler::GraphicsWindowOutputHandler(void)
	{
	}

	void GraphicsWindowOutputHandler::attachWindow(ogfx::GraphicsWindow& window)
	{
		if (m_window != nullptr) return;
		m_window = &window;
		m_renderer.init(window);
	}

	void GraphicsWindowOutputHandler::setMonospaceFont(const String& filePath)
	{
		m_renderer.setFont(filePath);
		m_renderer.setFontSize(m_fontSize);
		__update_char_size();
	}

	Vec2 GraphicsWindowOutputHandler::getStringSize(const String& str)
	{
		Vec2 size = { 0.0f, m_charSize.y };
		if (str.len() == 0) return size;
		size.x = str.len() * m_charSize.x;
		return size;
	}

	bool GraphicsWindowOutputHandler::isReady(void)
	{
		return m_window != nullptr && m_renderer.getTTFRenderer().hasOpenFont() && m_fontSize > 0;
	}

	void GraphicsWindowOutputHandler::resetCursorPosition(void)
	{
		m_curosrPosition = { 0, 0 };
	}

	void GraphicsWindowOutputHandler::resetColors(void)
	{
		m_foregroundColor = m_defaultForegroundColor;
		m_backgroundColor = m_defaultBackgroundColor;
	}

	void GraphicsWindowOutputHandler::beginFrame(void)
	{
		resetColors();
		resetCursorPosition();
	}




	void GraphicsWindowOutputHandler::setConsoleMaxCharacters(const IPoint& size)
	{
		m_consoleSize = size;
	}

	void GraphicsWindowOutputHandler::setConsolePosition(const Vec2& pos)
	{
		m_consolePosition = pos;
	}

	void GraphicsWindowOutputHandler::setWrapMode(eWrapMode wrapMode)
	{
		m_wrapMode = wrapMode;
	}

	void GraphicsWindowOutputHandler::setPadding(const Rectangle& rect)
	{
		m_padding = rect;
	}

	void GraphicsWindowOutputHandler::setDefaultBackgorundColor(const Color& color)
	{
		m_defaultBackgroundColor = color;
	}

	void GraphicsWindowOutputHandler::setDefaultForegroundColor(const Color& color)
	{
		m_defaultForegroundColor = color;
	}

	void GraphicsWindowOutputHandler::setTabWidth(uint8_t tw)
	{
		m_tabWidth = tw;
	}




	void GraphicsWindowOutputHandler::setFontSize(int32_t fontSize)
	{
		m_fontSize = fontSize;
		m_renderer.setFontSize(m_fontSize);
		__update_char_size();
	}

	int32_t GraphicsWindowOutputHandler::getFontSize(void)
	{
		return m_fontSize;
	}

	Vec2 GraphicsWindowOutputHandler::getCharacterSize(int32_t fontSize)
	{
		if (fontSize > 0)
		{
			auto size = m_renderer.getStringSize("A", fontSize);
			return { (float)size.x, (float)size.y };
		}
		return m_charSize;
	}

	Vec2 GraphicsWindowOutputHandler::getConsolePosition(void)
	{
		return m_consolePosition;
	}

	GraphicsWindowOutputHandler::eWrapMode GraphicsWindowOutputHandler::getWrapMode(void)
	{
		return m_wrapMode;
	}

	Rectangle GraphicsWindowOutputHandler::getPadding(void)
	{
		return m_padding;
	}

	Color GraphicsWindowOutputHandler::getDefaultBackgroundColor(void)
	{
		return m_defaultBackgroundColor;
	}

	Color GraphicsWindowOutputHandler::getDefaultForegroundColor(void)
	{
		return m_defaultForegroundColor;
	}

	uint8_t GraphicsWindowOutputHandler::getTabWidth(void)
	{
		return m_tabWidth;
	}

	Rectangle GraphicsWindowOutputHandler::getConsoleBounds(void)
	{
		float console_w = ((float)m_consoleSize.x * getCharacterSize().x) + getPadding().x + getPadding().w;
		float console_h = ((float)m_consoleSize.y * getCharacterSize().y) + getPadding().y + getPadding().h;
		return { getConsolePosition().x - getPadding().x, getConsolePosition().y - getPadding().y, console_w, console_h };
	}




	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::bg(const Color& color)
	{
		m_backgroundColor = color;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::bg(const ConsoleColors::tConsoleColor& color)
	{
		m_backgroundColor = color.fullColor;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::bg(const String& color)
	{
		m_backgroundColor = ConsoleColors::getFromName(color).fullColor;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::fg(const Color& color)
	{
		m_foregroundColor = color;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::fg(const ConsoleColors::tConsoleColor& color)
	{
		m_foregroundColor = color.fullColor;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::fg(const String& color)
	{
		m_foregroundColor = ConsoleColors::getFromName(color).fullColor;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::pChar(char c)
	{
		__print_string(ostd::String("").addChar(c));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::pStyled(const String& styled)
	{
		return pStyled(TextStyleParser::parse(styled, { m_backgroundColor, "", true }, { m_foregroundColor, "", false }));
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::pStyled(const TextStyleParser::tStyledString& styled)
	{
		if (!styled.validate()) return *this;
		Color oldBgCol = m_backgroundColor;
		Color oldFgCol = m_foregroundColor;
		for (int32_t i = 0; i < styled.text.len(); i++)
			bg(styled.backgroundColors[i].fullColor).fg(styled.foregroundColors[i].fullColor).pChar(styled.text[i]);
		m_backgroundColor = oldBgCol;
		m_foregroundColor = oldFgCol;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::pStyled(TextStyleBuilder::IRichStringBase& styled)
	{
		auto oldBg = styled.getDefaultBackgroundColor();
		auto oldFg = styled.getDefaultForegroundColor();
		styled.setDefaultBackgroundColor(m_backgroundColor);
		styled.setDefaultForegroundColor(m_foregroundColor);
		pStyled(styled.getStyledString());
		styled.setDefaultBackgroundColor(oldBg);
		styled.setDefaultForegroundColor(oldFg);
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::pObject(const BaseObject& bo)
	{
		__print_string(bo.toString());
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(const String& se)
	{
		__print_string(se);
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(uint8_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(int8_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(uint16_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(int16_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(uint32_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(int32_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(uint64_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(int64_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(float f, uint8_t precision)
	{
		__print_string(ostd::String("").add(f, precision));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::p(double f, uint8_t precision)
	{
		__print_string(ostd::String("").add(f, precision));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::nl(void)
	{
		if (m_curosrPosition.y >= getConsoleSize().y) return *this;
		m_curosrPosition.y++;
		m_curosrPosition.x = 0;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::tab(void)
	{
		if (m_curosrPosition.x == 0)
		{
			__print_string(ostd::String("").fixedLength(m_tabWidth, ' ', ""));
			return *this;
		}
		int32_t restTab = m_tabWidth - ((int32_t)m_curosrPosition.x % m_tabWidth);
		if (m_curosrPosition.x + restTab > m_consoleSize.x)
			restTab = m_consoleSize.x - m_curosrPosition.x;
		if (restTab == 0) return *this;
		__print_string(ostd::String("").fixedLength(restTab, ' ', ""));
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::flush(void)
	{
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::clear(void)
	{
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::reset(void)
	{
		resetColors();
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::xy(IPoint position)
	{
		m_curosrPosition = { (float)position.x, (float)position.y };
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::xy(int32_t x, int32_t y)
	{
		m_curosrPosition = { (float)x, (float)y };
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::x(int32_t x)
	{
		m_curosrPosition.x = (float)x;
		return *this;
	}

	GraphicsWindowOutputHandler& GraphicsWindowOutputHandler::y(int32_t y)
	{
		m_curosrPosition.y = (float)y;
		return *this;
	}

	IPoint GraphicsWindowOutputHandler::getCursorPosition(void)
	{
		return { (int32_t)std::round(m_curosrPosition.x), (int32_t)std::round(m_curosrPosition.y) };
	}

	void GraphicsWindowOutputHandler::getCursorPosition(int32_t& outX, int32_t& outY)
	{
		outX = (int32_t)std::round(m_curosrPosition.x);
		outY = (int32_t)std::round(m_curosrPosition.y);
	}

	int32_t GraphicsWindowOutputHandler::getCursorX(void)
	{
		return (int32_t)std::round(m_curosrPosition.x);
	}

	int32_t GraphicsWindowOutputHandler::getCursorY(void)
	{
		return (int32_t)std::round(m_curosrPosition.y);
	}

	void GraphicsWindowOutputHandler::getConsoleSize(int32_t& outColumns, int32_t& outRows)
	{
		int32_t console_rows = std::numeric_limits<int>::max();
		int32_t console_cols = std::numeric_limits<int>::max();
		if (m_consoleSize.x > 0) console_cols = m_consoleSize.x;
		if (m_consoleSize.y > 0) console_rows = m_consoleSize.y;
		outColumns = console_cols;
		outRows = console_rows;
	}

	IPoint GraphicsWindowOutputHandler::getConsoleSize(void)
	{
		int32_t console_rows = std::numeric_limits<int>::max();
		int32_t console_cols = std::numeric_limits<int>::max();
		if (m_consoleSize.x > 0) console_cols = m_consoleSize.x;
		if (m_consoleSize.y > 0) console_rows = m_consoleSize.y;
		return { console_cols, console_rows };
	}

	void GraphicsWindowOutputHandler::__update_char_size(void)
	{
		auto size = m_renderer.getStringSize("A", m_fontSize);
		m_charSize = { (float)size.x, (float)size.y };
	}

	void GraphicsWindowOutputHandler::__print_string(const String& str)
	{
		if (!isReady()) return;
		auto l_endOfConsole = [&](void) -> bool {
			return m_curosrPosition.x >= getConsoleSize().x && m_curosrPosition.y >= getConsoleSize().y - 1;
		};
		auto l_print = [&](const String& string) {
			if (l_endOfConsole()) return;
			if (string.len() == 0) return;
			float vertical_margin = 4;
			Vec2 pos = m_consolePosition + ostd::Vec2 { m_charSize.x * m_curosrPosition.x, m_charSize.y * m_curosrPosition.y };
			if (m_backgroundColor.a > 0)
				m_renderer.fillRect({ pos.x, pos.y + vertical_margin, m_charSize.x * string.len(), m_charSize.y - vertical_margin }, m_backgroundColor);
			if (m_foregroundColor.a > 0)
				m_renderer.drawString(string, pos, m_foregroundColor);
			m_curosrPosition.x += string.len();
			if (m_curosrPosition.x >= getConsoleSize().x)
				nl();
		};
		int32_t string_length = str.len();
		int32_t lineLen = m_curosrPosition.x + string_length;
		if (lineLen <= getConsoleSize().x)
		{
			l_print(str);
			return;
		}
		if (m_wrapMode == eWrapMode::TripleDots)
		{
			int32_t fixedLen = getConsoleSize().x - m_curosrPosition.x;
			l_print(str.new_fixedLength(fixedLen, ' ', "..."));
		}
		else if (m_wrapMode == eWrapMode::NewLine)
		{
			int32_t restOfLine = getConsoleSize().x - m_curosrPosition.x;
			ostd::String tmp = str.new_substr(0, restOfLine);
			l_print(tmp);
			tmp = str.new_substr(restOfLine);
			int32_t nlines = tmp.len() / getConsoleSize().x;
			if (nlines == 0)
			{
				l_print(tmp);
				return;
			}
			for (int32_t i = 0; i < nlines; i++)
			{
				ostd::String line = tmp.new_substr(0, getConsoleSize().x);
				l_print(line);
				tmp.substr(getConsoleSize().x);
			}
			if (tmp.len() > 0)
				l_print(tmp);
		}
	}
}
