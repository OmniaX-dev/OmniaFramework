#include "Logger.hpp"

#include <cstdarg>
#include <cstring>
#include <iostream>
#include "vendor/TermColor.hpp"
#include "IOHandlers.hpp"

namespace ostd
{
	OutputHandlerBase* Logger::m_out = new ConsoleOutputHandler;

	void Logger::__log_output(uint8_t log_level, String message, ...)
	{
		String level_str[6] = { "[  FATAL  ]: ", "[  ERROR  ]: ", "[ WARNING ]: ", "[  INFO   ]: ", "[  DEBUG  ]: ", "[  TRACE  ]: " };

		char buffer[4096];
		std::memset(buffer, 0, sizeof(buffer));

		__builtin_va_list arg_ptr;
		va_start(arg_ptr, message);
			vsnprintf(buffer, sizeof(buffer), message.c_str(), arg_ptr);
		va_end(arg_ptr);
		
		switch (log_level)
		{
			case tLogLevel::Fatal:
				m_out->fg(ConsoleColors::Red);
				break;
			case tLogLevel::Error:
				m_out->fg(ConsoleColors::BrightRed);
				break;
			case tLogLevel::Warning:
				m_out->fg(ConsoleColors::BrightMagenta);
				break;
			case tLogLevel::Info:
				m_out->fg(ConsoleColors::Cyan);
				break;
			case tLogLevel::Debug:
				m_out->fg(ConsoleColors::BrightBlue);
				break;
			case tLogLevel::Trace:
				m_out->fg(ConsoleColors::Green);
				break;
		}
		m_out->p(level_str[log_level]).p(buffer).reset().nl();
	}

	void Logger::destroy(void)
	{
		delete m_out;
	}

	void Logger::setOutputHandler(OutputHandlerBase& __destination)
	{
		m_out = &__destination; //TODO: Destroy old (only if still default)
	}
} 
