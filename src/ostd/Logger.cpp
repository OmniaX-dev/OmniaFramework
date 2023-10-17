#include "Logger.hpp"

#include <cstdarg>
#include <cstring>
#include <iostream>
#include "TermColor.hpp"
#include "Utils.hpp"

namespace ostd
{
	IOutputHandler* Logger::m_out = new ConsoleOutputHandler;

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
				m_out->col("red");
				break;
			case tLogLevel::Error:
				m_out->col("b-red");
				break;
			case tLogLevel::Warning:
				m_out->col("b-magenta");
				break;
			case tLogLevel::Info:
				m_out->col("cyan");
				break;
			case tLogLevel::Debug:
				m_out->col("b-blue");
				break;
			case tLogLevel::Trace:
				m_out->col("green");
				break;
		}
		m_out->p(level_str[log_level]).p(buffer).reset().nl();
	}

	void Logger::destroy(void)
	{
		delete m_out;
	}

	void Logger::setOutputHandler(IOutputHandler& __destination)
	{
		m_out = &__destination; //TODO: Destroy old (only if still default)
	}
} //namesoace ox
