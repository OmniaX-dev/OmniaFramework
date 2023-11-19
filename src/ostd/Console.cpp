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

#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)

#include <unistd.h>
#include <term.h>

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

#endif
}