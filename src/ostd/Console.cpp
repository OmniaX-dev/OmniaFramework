#include "Utils.hpp"
#include "Console.hpp"

#include <ostd/vendor/TermColor.hpp>

namespace ostd
{
#if defined _WIN32

#include <windows.h>

void Utils::clearConsole(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hStdOut;
	DWORD count;
	DWORD cellCount;
	COORD homeCoords = { 0, 0 };
	hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
	if (hStdOut == INVALID_HANDLE_VALUE)
		return;
	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo( hStdOut, &csbi))
		return;
	cellCount = csbi.dwSize.X *csbi.dwSize.Y;
	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(hStdOut, (TCHAR) ' ', cellCount, homeCoords, &count))
		return;
	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count))
		return;
	/* Move the cursor home */
	SetConsoleCursorPosition( hStdOut, homeCoords );
}

void Utils::getConsoleSize(int32_t& outColumns, int32_t& outRows)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	outColumns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	outRows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void Utils::setConsoleCursorPosition(int32_t x, int32_t y)
{
   COORD coord;
   coord.X = x; coord.Y = y;
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}

#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)

#include <unistd.h>
#include <term.h>
#include <sys/ioctl.h>

void Utils::clearConsole(void)
{
	if (!cur_term)
	{
		int result;
		setupterm(NULL, STDOUT_FILENO, &result);
		if (result <= 0) return;
	}

	putp(tigetstr( "clear" ));
}

void Utils::getConsoleSize(int32_t& outColumns, int32_t& outRows)
{
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	outRows = w.ws_row;
	outColumns = w.ws_col;
}

void Utils::setConsoleCursorPosition(int32_t x, int32_t y)
{
   printf("\033[%d;%dH",x+1,y+1);
}

#elif defined(__APPLE__)

#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>

void Utils::clearConsole(void)
{
    // ANSI escape sequence: clear screen + move cursor to top-left
    std::printf("\033[2J\033[H");
    std::fflush(stdout);
}

void Utils::getConsoleSize(int32_t& outColumns, int32_t& outRows)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    outRows = w.ws_row;
    outColumns = w.ws_col;
}

void Utils::setConsoleCursorPosition(int32_t x, int32_t y)
{
    // ANSI escape sequence: move cursor to (y+1, x+1)
    std::printf("\033[%d;%dH", y + 1, x + 1);
    std::fflush(stdout);
}

#endif


int32_t Utils::getConsoleWidth(void)
{
	int32_t rows = 0, cols = 0;
	getConsoleSize(cols, rows);
	return cols;
}

int32_t Utils::getConsoleHeight(void)
{
	int32_t rows = 0, cols = 0;
	getConsoleSize(rows, cols);
	return rows;
}




InteractiveConsole::InteractiveConsole(void)
{
	m_mode = eMode::Direct;
	m_validBuffer = false;
}

InteractiveConsole::InteractiveConsole(bool clearOnStart, eMode mode)
{
	if (clearOnStart)
		clear();
	m_mode = mode;
	m_validBuffer = false;
}

OutputHandlerBase& InteractiveConsole::bg(const ConsoleColors::tConsoleColor& color)
{
	std::cout << color.asBackground();
	return *this;
}

OutputHandlerBase& InteractiveConsole::bg(const String& color)
{
	std::cout << ConsoleColors::getFromName(color, true);
	return *this;
}

OutputHandlerBase& InteractiveConsole::fg(const ConsoleColors::tConsoleColor& color)
{
	std::cout << color.asForeground();
	return *this;
}

OutputHandlerBase& InteractiveConsole::fg(const String& color)
{
	std::cout << ConsoleColors::getFromName(color, false);
	return *this;
}

OutputHandlerBase& InteractiveConsole::pChar(char c)
{
	std::cout << (char)c;
	return *this;
}

OutputHandlerBase& InteractiveConsole::pStyled(const String& styled)
{
	return pStyled(TextStyleParser::parse(styled));
}

OutputHandlerBase& InteractiveConsole::pStyled(const TextStyleParser::tStyledString& styled)
{
	std::cout << styled;
	return *this;
}

OutputHandlerBase& InteractiveConsole::pStyled(TextStyleBuilder::IRichStringBase& styled)
{
	std::cout << styled;
	return *this;
}

OutputHandlerBase& InteractiveConsole::pObject(const BaseObject& bo)
{
	std::cout << bo;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(const String& se)
{
	std::cout << se;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(uint8_t i)
{
	std::cout << (uint8_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(int8_t i)
{
	std::cout << (int8_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(uint16_t i)
{
	std::cout << (uint16_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(int16_t i)
{
	std::cout << (int16_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(uint32_t i)
{
	std::cout << (uint32_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(int32_t i)
{
	std::cout << (int32_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(uint64_t i)
{
	std::cout << (uint64_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(int64_t i)
{
	std::cout << (int64_t)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(float f, uint8_t precision)
{
	if (precision == 0)
	{
		std::cout << f;
	}
	else
	{
		std::ios cout_state(nullptr);
		std::cout << std::fixed << std::setprecision(precision) << f;
		std::cout.copyfmt(cout_state);
	}
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(double f, uint8_t precision)
{
	if (precision == 0)
	{
		std::cout << f;
	}
	else
	{
		std::ios cout_state(nullptr);
		std::cout << std::fixed << std::setprecision(precision) << f;
		std::cout.copyfmt(cout_state);
	}
	return *this;
}

OutputHandlerBase& InteractiveConsole::nl(void)
{
	std::cout << "\n";
	return *this;
}

OutputHandlerBase& InteractiveConsole::flush(void)
{
	std::cout << std::endl;
	return *this;
}

OutputHandlerBase& InteractiveConsole::reset(void)
{
	std::cout << termcolor::reset;
	return *this;
}

OutputHandlerBase& InteractiveConsole::clear(void)
{
	Utils::clearConsole();
	return *this;
}

void InteractiveConsole::getConsoleSize(int32_t& outColumns, int32_t& outRows)
{
	Utils::getConsoleSize(outColumns, outRows);
}

IPoint InteractiveConsole::getConsoleSize(void)
{
	int32_t x = 0, y = 0;
	Utils::getConsoleSize(x, y);
	return { x, y };
}

void InteractiveConsole::update(void)
{
	if (m_mode != eMode::Buffered) return;
	__swap_buffers();
}

void InteractiveConsole::__set_cursor(void)
{
	Utils::setConsoleCursorPosition(m_cursorPosition.x, m_cursorPosition.y);
}

void InteractiveConsole::__construct_buffer(void)
{
	auto console_size = getConsoleSize();
	if (console_size == m_oldConsoleSize)
	{
		m_validBuffer = true;
		return;
	}
	m_oldConsoleSize = console_size;
	int32_t linear_size = m_oldConsoleSize.x * m_oldConsoleSize.y;
	m_buffer.resize(linear_size);
	m_secondBuffer.resize(linear_size);
	m_validBuffer = false;
	__validate_and_clear_buffers();
}

void InteractiveConsole::__swap_buffers(void)
{
	if (m_buffer.size() != m_secondBuffer.size())
		__construct_buffer();
	if (!m_validBuffer) return; //TODO: Error, shouldn't happen
}

void InteractiveConsole::__validate_and_clear_buffers(void)
{
	if (m_validBuffer) return;
	for (int32_t i = 0; i < m_buffer.size(); i++)
		m_buffer[i] = m_secondBuffer[i] = m_emptyChar;
	m_validBuffer = true;
}

}
