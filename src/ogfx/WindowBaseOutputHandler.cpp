#include "WindowBaseOutputHandler.hpp"
#include "BasicRenderer.hpp"
#include "TextStyleParser.hpp"

namespace ogfx
{
	using namespace ostd;

	WindowBaseOutputHandler::WindowBaseOutputHandler(void)
	{
	}

	void WindowBaseOutputHandler::attachWindow(ogfx::WindowBase& window)
	{
		if (m_window != nullptr) return;
		m_window = &window;
		m_renderer.init(window);
	}

	void WindowBaseOutputHandler::setMonospaceFont(const String& filePath)
	{
		m_renderer.setFont(filePath);
		m_renderer.setFontSize(m_fontSize);
		__update_char_size();
	}

	Vec2 WindowBaseOutputHandler::getStringSize(const String& str)
	{
		Vec2 size = { 0.0f, m_charSize.y };
		if (str.len() == 0) return size;
		size.x = str.len() * m_charSize.x;
		return size;
	}

	bool WindowBaseOutputHandler::isReady(void)
	{
		return m_window != nullptr && m_renderer.getTTFRenderer().hasOpenFont() && m_fontSize > 0;
	}

	void WindowBaseOutputHandler::resetCursorPosition(void)
	{
		m_curosrPosition = { 0, 0 };
	}

	void WindowBaseOutputHandler::resetColors(void)
	{
		m_foregroundColor = m_defaultForegroundColor;
		m_backgroundColor = m_defaultBackgroundColor;
	}

	void WindowBaseOutputHandler::beginFrame(void)
	{
		resetColors();
		resetCursorPosition();
	}




	void WindowBaseOutputHandler::setConsoleMaxCharacters(const IPoint& size)
	{
		m_consoleSize = size;
	}

	void WindowBaseOutputHandler::setConsolePosition(const Vec2& pos)
	{
		m_consolePosition = pos;
	}

	void WindowBaseOutputHandler::setWrapMode(eWrapMode wrapMode)
	{
		m_wrapMode = wrapMode;
	}

	void WindowBaseOutputHandler::setPadding(const Rectangle& rect)
	{
		m_padding = rect;
	}

	void WindowBaseOutputHandler::setDefaultBackgorundColor(const Color& color)
	{
		m_defaultBackgroundColor = color;
	}

	void WindowBaseOutputHandler::setDefaultForegroundColor(const Color& color)
	{
		m_defaultForegroundColor = color;
	}

	void WindowBaseOutputHandler::setTabWidth(uint8_t tw)
	{
		m_tabWidth = tw;
	}




	void WindowBaseOutputHandler::setFontSize(int32_t fontSize)
	{
		m_fontSize = fontSize;
		m_renderer.setFontSize(m_fontSize);
		__update_char_size();
	}

	int32_t WindowBaseOutputHandler::getFontSize(void)
	{
		return m_fontSize;
	}

	Vec2 WindowBaseOutputHandler::getCharacterSize(int32_t fontSize)
	{
		if (fontSize > 0)
		{
			auto size = m_renderer.getStringSize("A", fontSize);
			return { (float)size.x, (float)size.y };
		}
		return m_charSize;
	}

	Vec2 WindowBaseOutputHandler::getConsolePosition(void)
	{
		return m_consolePosition;
	}

	WindowBaseOutputHandler::eWrapMode WindowBaseOutputHandler::getWrapMode(void)
	{
		return m_wrapMode;
	}

	Rectangle WindowBaseOutputHandler::getPadding(void)
	{
		return m_padding;
	}

	Color WindowBaseOutputHandler::getDefaultBackgroundColor(void)
	{
		return m_defaultBackgroundColor;
	}

	Color WindowBaseOutputHandler::getDefaultForegroundColor(void)
	{
		return m_defaultForegroundColor;
	}

	uint8_t WindowBaseOutputHandler::getTabWidth(void)
	{
		return m_tabWidth;
	}

	Rectangle WindowBaseOutputHandler::getConsoleBounds(void)
	{
		float console_w = ((float)m_consoleSize.x * getCharacterSize().x) + getPadding().x + getPadding().w;
		float console_h = ((float)m_consoleSize.y * getCharacterSize().y) + getPadding().y + getPadding().h;
		return { getConsolePosition().x - getPadding().x, getConsolePosition().y - getPadding().y, console_w, console_h };
	}




	WindowBaseOutputHandler& WindowBaseOutputHandler::bg(const Color& color)
	{
		m_backgroundColor = color;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::bg(const ConsoleColors::tConsoleColor& color)
	{
		m_backgroundColor = color.fullColor;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::bg(const String& color)
	{
		m_backgroundColor = ConsoleColors::getFromName(color).fullColor;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::fg(const Color& color)
	{
		m_foregroundColor = color;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::fg(const ConsoleColors::tConsoleColor& color)
	{
		m_foregroundColor = color.fullColor;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::fg(const String& color)
	{
		m_foregroundColor = ConsoleColors::getFromName(color).fullColor;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::pChar(char c)
	{
		__print_string(ostd::String("").addChar(c));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::pStyled(const String& styled)
	{
		return pStyled(TextStyleParser::parse(styled, { m_backgroundColor, "", true }, { m_foregroundColor, "", false }));
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::pStyled(const TextStyleParser::tStyledString& styled)
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

	WindowBaseOutputHandler& WindowBaseOutputHandler::pStyled(TextStyleBuilder::IRichStringBase& styled)
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

	WindowBaseOutputHandler& WindowBaseOutputHandler::pObject(const BaseObject& bo)
	{
		__print_string(bo.toString());
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(const String& se)
	{
		__print_string(se);
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(uint8_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(int8_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(uint16_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(int16_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(uint32_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(int32_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(uint64_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(int64_t i)
	{
		__print_string(ostd::String("").add(i));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(float f, uint8_t precision)
	{
		__print_string(ostd::String("").add(f, precision));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::p(double f, uint8_t precision)
	{
		__print_string(ostd::String("").add(f, precision));
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::nl(void)
	{
		if (m_curosrPosition.y >= getConsoleSize().y) return *this;
		m_curosrPosition.y++;
		m_curosrPosition.x = 0;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::tab(void)
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

	WindowBaseOutputHandler& WindowBaseOutputHandler::flush(void)
	{
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::clear(void)
	{
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::reset(void)
	{
		resetColors();
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::xy(IPoint position)
	{
		m_curosrPosition = { (float)position.x, (float)position.y };
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::xy(int32_t x, int32_t y)
	{
		m_curosrPosition = { (float)x, (float)y };
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::x(int32_t x)
	{
		m_curosrPosition.x = (float)x;
		return *this;
	}

	WindowBaseOutputHandler& WindowBaseOutputHandler::y(int32_t y)
	{
		m_curosrPosition.y = (float)y;
		return *this;
	}

	IPoint WindowBaseOutputHandler::getCursorPosition(void)
	{
		return { (int32_t)std::round(m_curosrPosition.x), (int32_t)std::round(m_curosrPosition.y) };
	}

	void WindowBaseOutputHandler::getCursorPosition(int32_t& outX, int32_t& outY)
	{
		outX = (int32_t)std::round(m_curosrPosition.x);
		outY = (int32_t)std::round(m_curosrPosition.y);
	}

	int32_t WindowBaseOutputHandler::getCursorX(void)
	{
		return (int32_t)std::round(m_curosrPosition.x);
	}

	int32_t WindowBaseOutputHandler::getCursorY(void)
	{
		return (int32_t)std::round(m_curosrPosition.y);
	}

	void WindowBaseOutputHandler::getConsoleSize(int32_t& outColumns, int32_t& outRows)
	{
		int32_t console_rows = std::numeric_limits<int>::max();
		int32_t console_cols = std::numeric_limits<int>::max();
		if (m_consoleSize.x > 0) console_cols = m_consoleSize.x;
		if (m_consoleSize.y > 0) console_rows = m_consoleSize.y;
		outColumns = console_cols;
		outRows = console_rows;
	}

	IPoint WindowBaseOutputHandler::getConsoleSize(void)
	{
		int32_t console_rows = std::numeric_limits<int>::max();
		int32_t console_cols = std::numeric_limits<int>::max();
		if (m_consoleSize.x > 0) console_cols = m_consoleSize.x;
		if (m_consoleSize.y > 0) console_rows = m_consoleSize.y;
		return { console_cols, console_rows };
	}

	void WindowBaseOutputHandler::__update_char_size(void)
	{
		auto size = m_renderer.getStringSize("A", m_fontSize);
		m_charSize = { (float)size.x, (float)size.y };
	}

	void WindowBaseOutputHandler::__print_string(const String& str)
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
