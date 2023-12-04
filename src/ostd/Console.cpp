#include <Utils.hpp>

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

void Utils::getConsoleSize(int32_t& outRows, int32_t& outColumns)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns, rows;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	outColumns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	outRows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
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

void Utils::getConsoleSize(int32_t& outRows, int32_t& outColumns)
{
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	outRows = w.ws_row;
	outColumns = w.ws_col;
}

#endif

int32_t Utils::getConsoleWidth(void)
{
	int32_t rows = 0, cols = 0;
	getConsoleSize(rows, cols);
	return cols;
}

int32_t Utils::getConsoleHeight(void)
{
	int32_t rows = 0, cols = 0;
	getConsoleSize(rows, cols);
	return rows;
}

}