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

#pragma once

#include <ostd/string/String.hpp>

namespace ostd
{
	struct tLogLevel
	{
		inline static const uint8_t Fatal = 0;
		inline static const uint8_t Error = 1;
		inline static const uint8_t Warning = 2;
		inline static const uint8_t Info = 3;
		inline static const uint8_t Debug = 4;
		inline static const uint8_t Trace = 5;
	};

	class OutputHandlerBase;
	class Logger
	{
		public:
			static void __log_output(uint8_t log_level, String message, ...);
			static void setOutputHandler(OutputHandlerBase& __destination);
			static void destroy(void);
			static inline OutputHandlerBase& getOutputHandler(void) { return *m_out; }

		private:
			static OutputHandlerBase* m_out;
	};
}

#define OX_LOG_WARN_ENABLED 1
#define OX_LOG_INFO_ENABLED 1
#define OX_LOG_DEBUG_ENABLED 1
#define OX_LOG_TRACE_ENABLED 1

#ifdef  OX_RELEASE_BUILD
	#define OX_LOG_DEBUG_ENABLED 0
	#define OX_LOG_TRACE_ENABLED 0
#endif

#define OX_FATAL(message, ...) ostd::Logger::__log_output(ostd::tLogLevel::Fatal, message, ##__VA_ARGS__)
#define OX_ERROR(message, ...) ostd::Logger::__log_output(ostd::tLogLevel::Error, message, ##__VA_ARGS__)
#if OX_LOG_WARN_ENABLED == 1
	#define OX_WARN(message, ...) ostd::Logger::__log_output(ostd::tLogLevel::Warning, message, ##__VA_ARGS__)
#else
	#define OX_WARN(message, ...)
#endif
#if OX_LOG_INFO_ENABLED == 1
	#define OX_INFO(message, ...) ostd::Logger::__log_output(ostd::tLogLevel::Info, message, ##__VA_ARGS__)
#else
	#define OX_INFO(message, ...)
#endif
#if OX_LOG_DEBUG_ENABLED == 1
	#define OX_DEBUG(message, ...) ostd::Logger::__log_output(ostd::tLogLevel::Debug, message, ##__VA_ARGS__)
#else
	#define OX_DEBUG(message, ...)
#endif
#if OX_LOG_TRACE_ENABLED == 1
	#define OX_TRACE(message, ...) ostd::Logger::__log_output(ostd::tLogLevel::Trace, message, ##__VA_ARGS__)
#else
	#define OX_TRACE(message, ...)
#endif
