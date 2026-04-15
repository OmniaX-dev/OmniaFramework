#include "Console.hpp"
#include "../vendor/TermColor.hpp"

namespace ostd
{
#if defined _WIN32

#include <windows.h>

void BasicConsole::clearConsole(void)
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

void BasicConsole::getConsoleSize(i32& outColumns, i32& outRows)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	outColumns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	outRows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void BasicConsole::setConsoleCursorPosition(i32 x, i32 y)
{
   COORD coord;
   coord.X = x; coord.Y = y;
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}

IPoint BasicConsole::getConsoleCursorPosition(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return { csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y };
}

#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)

#include <unistd.h>
#include <term.h>
#include <sys/ioctl.h>

void BasicConsole::clearConsole(void)
{
	if (!cur_term)
	{
		i32 result;
		setupterm(NULL, STDOUT_FILENO, &result);
		if (result <= 0) return;
	}

	putp(tigetstr( "clear" ));
}

void BasicConsole::getConsoleSize(i32& outColumns, i32& outRows)
{
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	outRows = w.ws_row;
	outColumns = w.ws_col;
}

void BasicConsole::setConsoleCursorPosition(i32 x, i32 y)
{
   printf("\033[%d;%dH",y+1,x+1);
}

IPoint BasicConsole::getConsoleCursorPosition(void)
{
	// Save terminal settings
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Request cursor position
    std::printf("\033[6n");
    std::fflush(stdout);

    // Read response: ESC [ row ; col R
    i32 row = 0, col = 0;
    if (std::scanf("\033[%d;%dR", &row, &col) != 2) {
        // Failed to parse
        row = col = -1;
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    // Convert to 0-based like Windows
    return { col - 1, row - 1 };
}

#elif defined(__APPLE__)

#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>

void BasicConsole::clearConsole(void)
{
    // ANSI escape sequence: clear screen + move cursor to top-left
    std::printf("\033[2J\033[H");
    std::fflush(stdout);
}

void BasicConsole::getConsoleSize(i32& outColumns, i32& outRows)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    outRows = w.ws_row;
    outColumns = w.ws_col;
}

void BasicConsole::setConsoleCursorPosition(i32 x, i32 y)
{
    // ANSI escape sequence: move cursor to (y+1, x+1)
    std::printf("\033[%d;%dH", y + 1, x + 1);
    std::fflush(stdout);
}

IPoint BasicConsole::getConsoleCursorPosition(void)
{
	// Save terminal settings
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Request cursor position
    std::printf("\033[6n");
    std::fflush(stdout);

    // Read response: ESC [ row ; col R
    i32 row = 0, col = 0;
    if (std::scanf("\033[%d;%dR", &row, &col) != 2) {
        // Failed to parse
        row = col = -1;
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    // Convert to 0-based like Windows
    return { col - 1, row - 1 };
}

#endif


i32 BasicConsole::getConsoleWidth(void)
{
	i32 rows = 0, cols = 0;
	getConsoleSize(cols, rows);
	return cols;
}

i32 BasicConsole::getConsoleHeight(void)
{
	i32 rows = 0, cols = 0;
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

OutputHandlerBase& InteractiveConsole::p(u8 i)
{
	std::cout << (u8)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(i8 i)
{
	std::cout << (i8)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(u16 i)
{
	std::cout << (u16)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(i16 i)
{
	std::cout << (i16)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(u32 i)
{
	std::cout << (u32)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(i32 i)
{
	std::cout << (i32)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(u64 i)
{
	std::cout << (u64)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(i64 i)
{
	std::cout << (i64)i;
	return *this;
}

OutputHandlerBase& InteractiveConsole::p(f32 f, u8 precision)
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

OutputHandlerBase& InteractiveConsole::p(f64 f, u8 precision)
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
	BasicConsole::clearConsole();
	return *this;
}

void InteractiveConsole::getConsoleSize(i32& outColumns, i32& outRows)
{
	BasicConsole::getConsoleSize(outColumns, outRows);
}

IPoint InteractiveConsole::getConsoleSize(void)
{
	i32 x = 0, y = 0;
	BasicConsole::getConsoleSize(x, y);
	return { x, y };
}

void InteractiveConsole::update(void)
{
	if (m_mode != eMode::Buffered) return;
	__swap_buffers();
}

void InteractiveConsole::__set_cursor(void)
{
	BasicConsole::setConsoleCursorPosition(m_cursorPosition.x, m_cursorPosition.y);
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
	i32 linear_size = m_oldConsoleSize.x * m_oldConsoleSize.y;
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
	for (i32 i = 0; i < m_buffer.size(); i++)
		m_buffer[i] = m_secondBuffer[i] = m_emptyChar;
	m_validBuffer = true;
}

}
