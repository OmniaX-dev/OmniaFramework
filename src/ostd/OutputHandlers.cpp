#include "Utils.hpp"
#include "Color.hpp"
#include "BaseObject.hpp"
#include "Logger.hpp"
#include "TextStyleParser.hpp"
#include "IOHandlers.hpp"
#include "String.hpp"

#include "vendor/TermColor.hpp"

#include <iomanip>
#include <iostream>

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


		
	OutputHandlerBase& ConsoleOutputHandler::bg(const ConsoleColors::tConsoleColor& color)
	{
		std::cout << color.asBackground();
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::bg(const StringEditor& color)
	{
		std::cout << ConsoleColors::getFromName(color, true);
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::fg(const ConsoleColors::tConsoleColor& color)
	{
		std::cout << color.asForeground();
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::fg(const StringEditor& color)
	{
		std::cout << ConsoleColors::getFromName(color, false);
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::pChar(char c)
	{
		std::cout << (char)c;
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::pStyled(const StringEditor& styled)
	{
		return pStyled(TextStyleParser::parse(styled.str()));
	}
	
	OutputHandlerBase& ConsoleOutputHandler::pStyled(const TextStyleParser::tStyledString& styled)
	{
		std::cout << styled;
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::pStyled(const TextStyleBuilder::IRichStringBase& styled)
	{
		std::cout << styled;
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::pObject(const BaseObject& bo)
	{
		std::cout << bo;
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::p(const StringEditor& se)
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
		std::cout << std::endl;
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::reset(void)
	{
		std::cout << termcolor::reset;
		return *this;
	}
	
	OutputHandlerBase& ConsoleOutputHandler::clear(void)
	{
		Utils::clearConsole();
		return *this;
	}





	namespace legacy
	{
		IOutputHandler& ConsoleOutputHandler::col(String color)
		{
			StringEditor cse = color;
			cse.toLower().trim();
			if (cse.startsWith("@[") && cse.endsWith("]"))
			{
				OX_WARN("ox::ConsoleOutputHandler::col(ox::String): RGB Colors not supported for standard console output: %s", cse.c_str());
				return *this;
			}

			//RED
			if (cse.str() == ConsoleCol::Red) std::cout << termcolor::red;
			else if (cse.str() == ConsoleCol::BrightRed) std::cout << termcolor::bright_red;
			else if (cse.str() == ConsoleCol::OnRed) std::cout << termcolor::on_red;
			else if (cse.str() == ConsoleCol::OnBrightRed) std::cout << termcolor::on_bright_red;
			//GREEN
			else if (cse.str() == ConsoleCol::Green) std::cout << termcolor::green;
			else if (cse.str() == ConsoleCol::BrightGreen) std::cout << termcolor::bright_green;
			else if (cse.str() == ConsoleCol::OnGreen) std::cout << termcolor::on_green;
			else if (cse.str() == ConsoleCol::OnBrightGreen) std::cout << termcolor::on_bright_green;
			//BLUE
			else if (cse.str() == ConsoleCol::Blue) std::cout << termcolor::blue;
			else if (cse.str() == ConsoleCol::BrightBlue) std::cout << termcolor::bright_blue;
			else if (cse.str() == ConsoleCol::OnBlue) std::cout << termcolor::on_blue;
			else if (cse.str() == ConsoleCol::OnBrightBlue) std::cout << termcolor::on_bright_blue;
			//MAGENTA
			else if (cse.str() == ConsoleCol::Magenta) std::cout << termcolor::magenta;
			else if (cse.str() == ConsoleCol::BrightMagenta) std::cout << termcolor::bright_magenta;
			else if (cse.str() == ConsoleCol::OnMagenta) std::cout << termcolor::on_magenta;
			else if (cse.str() == ConsoleCol::OnBrightMagenta) std::cout << termcolor::on_bright_magenta;
			//CYAN
			else if (cse.str() == ConsoleCol::Cyan || cse.str() == "aqua") std::cout << termcolor::cyan;
			else if (cse.str() == ConsoleCol::BrightCyan) std::cout << termcolor::bright_cyan;
			else if (cse.str() == ConsoleCol::OnCyan || cse.str() == "o-aqua") std::cout << termcolor::on_cyan;
			else if (cse.str() == ConsoleCol::OnBrightCyan) std::cout << termcolor::on_bright_cyan;
			//YELLOW
			else if (cse.str() == ConsoleCol::Yellow) std::cout << termcolor::yellow;
			else if (cse.str() == ConsoleCol::BrightYellow) std::cout << termcolor::bright_yellow;
			else if (cse.str() == ConsoleCol::OnYellow) std::cout << termcolor::on_yellow;
			else if (cse.str() == ConsoleCol::OnBrightYellow) std::cout << termcolor::on_bright_yellow;
			//GRAY
			else if (cse.str() == ConsoleCol::Black || cse.str() == "black") std::cout << termcolor::grey;
			else if (cse.str() == ConsoleCol::BrightGray) std::cout << termcolor::bright_grey;
			else if (cse.str() == ConsoleCol::OnBlack || cse.str() == "o-black") std::cout << termcolor::on_grey;
			else if (cse.str() == ConsoleCol::OnBrightGray) std::cout << termcolor::on_bright_grey;
			//WHITE
			else if (cse.str() == ConsoleCol::Gray) std::cout << termcolor::white;
			else if (cse.str() == ConsoleCol::White || cse.str() == "b-lgray") std::cout << termcolor::bright_white;
			else if (cse.str() == ConsoleCol::onGray) std::cout << termcolor::on_white;
			else if (cse.str() == ConsoleCol::OnWhite || cse.str() == "o-white") std::cout << termcolor::on_bright_white;
			else
				OX_WARN("ox::ConsoleOutputHandler::col(ox::String): Unknown color string: %s", cse.c_str());
			return *this;
		}

		IOutputHandler& ConsoleOutputHandler::col(const Color& color)
		{
			StringEditor cse = "@[";
			cse.addi(color.r).add(", ");
			cse.addi(color.g).add(", ");
			cse.addi(color.b).add(", ");
			cse.addi(color.a).add("]");
			return col(cse.str());
		}
		
		IOutputHandler& ConsoleOutputHandler::p(char c)
		{
			std::cout << (char)c;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::p(const StringEditor& se)
		{
			std::cout << se;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(uint8_t i)
		{
			std::cout << (int64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(int8_t i)
		{
			std::cout << (int64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(uint16_t i)
		{
			std::cout << (int64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(int16_t i)
		{
			std::cout << (int64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(uint32_t i)
		{
			std::cout << (int64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(int32_t i)
		{
			std::cout << (int64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(uint64_t i)
		{
			std::cout << (uint64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pi(int64_t i)
		{
			std::cout << (int64_t)i;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::pf(float f, uint8_t precision)
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
		
		IOutputHandler& ConsoleOutputHandler::pf(double f, uint8_t precision)
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

		IOutputHandler& ConsoleOutputHandler::pStyled(const StringEditor& styled)
		{
			return pStyled(TextStyleParser::parse(styled.str()));
		}

		IOutputHandler& ConsoleOutputHandler::pStyled(const TextStyleParser::tStyledString& styled)
		{
			std::cout << styled;
			return *this;
		}

		IOutputHandler& ConsoleOutputHandler::pStyled(const TextStyleBuilder::IRichStringBase& styled)
		{
			std::cout << styled.getStyledString();
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::nl(void)
		{
			std::cout << "\n";
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::flush(void)
		{
			std::cout << std::endl;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::reset(void)
		{
			std::cout << termcolor::reset;
			return *this;
		}
		
		IOutputHandler& ConsoleOutputHandler::clear(void)
		{
			Utils::clearConsole();
			return *this;
		}





		IOutputHandler& BufferedOutputHandler::p(char c)
		{
			m_buffer.add(c);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::p(const StringEditor& se)
		{
			m_buffer.add(se.str());
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(uint8_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(int8_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(uint16_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(int16_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(uint32_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(int32_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(uint64_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pi(int64_t i)
		{
			m_buffer.addi(i);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pf(float f, uint8_t precision)
		{
			m_buffer.addf(f);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::pf(double f, uint8_t precision)
		{
			m_buffer.addf(f);
			return *this;
		}
		
		IOutputHandler& BufferedOutputHandler::nl(void)
		{
			m_buffer.add("\n");
			return *this;
		}
		
	}

}
