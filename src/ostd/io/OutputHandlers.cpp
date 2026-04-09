#include <iomanip>
#include <iostream>
#include "Logger.hpp"
#include "IOHandlers.hpp"
#include "../vendor/TermColor.hpp"
#include "../data/BaseObject.hpp"
#include "../string/TextStyleParser.hpp"
#include "../string/String.hpp"
#include "io/Console.hpp"

namespace ostd
{
	std::ostream &operator<<(std::ostream &os, ConsoleColors::tConsoleColor const &col)
	{
		if (col.background)
		{
			if (col.name == "red") os << termcolor::on_red;
			else if (col.name == "brightred") os << termcolor::on_bright_red;
			else if (col.name == "green") os << termcolor::on_green;
			else if (col.name == "brightgreen") os << termcolor::on_bright_green;
			else if (col.name == "blue") os << termcolor::on_blue;
			else if (col.name == "brightblue") os << termcolor::on_bright_blue;
			else if (col.name == "magenta") os << termcolor::on_magenta;
			else if (col.name == "brightmagenta") os << termcolor::on_bright_magenta;
			else if (col.name == "cyan") os << termcolor::on_cyan;
			else if (col.name == "brightcyan") os << termcolor::on_bright_cyan;
			else if (col.name == "yellow") os << termcolor::on_yellow;
			else if (col.name == "brightyellow") os << termcolor::on_bright_yellow;
			else if (col.name == "black") os << termcolor::on_grey;
			else if (col.name == "darkgray") os << termcolor::on_bright_grey;
			else if (col.name == "brightgray") os << termcolor::on_white;
			else if (col.name == "white") os << termcolor::on_bright_white;
			return os;
		}
		if (col.name == "red") os << termcolor::red;
		else if (col.name == "brightred") os << termcolor::bright_red;
		else if (col.name == "green") os << termcolor::green;
		else if (col.name == "brightgreen") os << termcolor::bright_green;
		else if (col.name == "blue") os << termcolor::blue;
		else if (col.name == "brightblue") os << termcolor::bright_blue;
		else if (col.name == "magenta") os << termcolor::magenta;
		else if (col.name == "brightmagenta") os << termcolor::bright_magenta;
		else if (col.name == "cyan") os << termcolor::cyan;
		else if (col.name == "brightcyan") os << termcolor::bright_cyan;
		else if (col.name == "yellow") os << termcolor::yellow;
		else if (col.name == "brightyellow") os << termcolor::bright_yellow;
		else if (col.name == "black") os << termcolor::grey;
		else if (col.name == "darkgray") os << termcolor::bright_grey;
		else if (col.name == "brightgray") os << termcolor::white;
		else if (col.name == "white") os << termcolor::bright_white;
		return os;
	}



	OutputHandlerBase& ConsoleOutputHandler::bg(const Color& color)
	{
		std::cout << "\033[48;2;" << (int)color.r << ";" << (int)color.g << ";" << (int)color.b << "m";
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::bg(const ConsoleColors::tConsoleColor& color)
	{
		std::cout << color.asBackground();
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::bg(const String& color)
	{
		std::cout << ConsoleColors::getFromName(color, true);
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::fg(const Color& color)
	{
		std::cout << "\033[38;2;" << (int)color.r << ";" << (int)color.g << ";" << (int)color.b << "m";
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::fg(const ConsoleColors::tConsoleColor& color)
	{
		std::cout << color.asForeground();
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::fg(const String& color)
	{
		std::cout << ConsoleColors::getFromName(color, false);
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::pChar(char c)
	{
		std::cout << (char)c;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::pStyled(const String& styled)
	{
		return pStyled(TextStyleParser::parse(styled));
	}

	OutputHandlerBase& ConsoleOutputHandler::pStyled(const TextStyleParser::tStyledString& styled)
	{
		std::cout << styled;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::pStyled(TextStyleBuilder::IRichStringBase& styled)
	{
		std::cout << styled;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::pObject(const BaseObject& bo)
	{
		std::cout << bo;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(const String& se)
	{
		std::cout << se;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(uint8_t i)
	{
		std::cout << (uint8_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(int8_t i)
	{
		std::cout << (int8_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(uint16_t i)
	{
		std::cout << (uint16_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(int16_t i)
	{
		std::cout << (int16_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(uint32_t i)
	{
		std::cout << (uint32_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(int32_t i)
	{
		std::cout << (int32_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(uint64_t i)
	{
		std::cout << (uint64_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(int64_t i)
	{
		std::cout << (int64_t)i;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::p(float f, uint8_t precision)
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

	OutputHandlerBase& ConsoleOutputHandler::p(double f, uint8_t precision)
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

	OutputHandlerBase& ConsoleOutputHandler::nl(void)
	{
		std::cout << "\n";
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::flush(void)
	{
		std::cout << std::flush;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::reset(void)
	{
		std::cout << termcolor::reset;
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::clear(void)
	{
		BasicConsole::clearConsole();
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::xy(IPoint position)
	{
		BasicConsole::setConsoleCursorPosition(position.x, position.y);
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::xy(int32_t x, int32_t y)
	{
		BasicConsole::setConsoleCursorPosition(x, y);
		return *this;
	}

	OutputHandlerBase& ConsoleOutputHandler::x(int32_t x)
	{
		int32_t y = getCursorPosition().y;
		return xy(x, y);
	}

	OutputHandlerBase& ConsoleOutputHandler::y(int32_t y)
	{
		int32_t x = getCursorPosition().x;
		return xy(x, y);
	}

	IPoint ConsoleOutputHandler::getCursorPosition(void)
	{
		return BasicConsole::getConsoleCursorPosition();
	}

	void ConsoleOutputHandler::getCursorPosition(int32_t& outX, int32_t& outY)
	{
		auto pos = BasicConsole::getConsoleCursorPosition();;
		outX = pos.x;
		outY = pos.y;
	}

	int32_t ConsoleOutputHandler::getCursorX(void)
	{
		auto pos = BasicConsole::getConsoleCursorPosition();;
		return pos.x;
	}

	int32_t ConsoleOutputHandler::getCursorY(void)
	{
		auto pos = BasicConsole::getConsoleCursorPosition();;
		return pos.y;
	}

	void ConsoleOutputHandler::getConsoleSize(int32_t& outColumns, int32_t& outRows)
	{
		BasicConsole::getConsoleSize(outColumns, outRows);
	}

	IPoint ConsoleOutputHandler::getConsoleSize(void)
	{
		IPoint size;
		BasicConsole::getConsoleSize(size.x, size.y);
		return size;
	}

}
