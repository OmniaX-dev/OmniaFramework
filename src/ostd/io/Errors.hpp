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

#include <ostd/data_types/BaseObject.hpp>
#include <ostd/data_types/Types.hpp>
#include <ostd/utils/Utils.hpp>

#define ERROR_DATA() String(CPP_STR(__LINE__)), String(__FILE__)

namespace ostd
{
	struct tErrorLevel
	{
		inline static constexpr uint8_t NoError = 0x00;
		inline static constexpr uint8_t Warning = 0x01;
		inline static constexpr uint8_t Error = 0x02;
		inline static constexpr uint8_t Fatal = 0x03;
	};

	class RuntimeError : public BaseObject
	{
		public:
			inline RuntimeError(void) { invalidate(); }
			inline RuntimeError(uint8_t group, uint64_t code, uint8_t level, const String& msg) { create(group, code, level, msg); }
			RuntimeError& create(uint8_t group, uint64_t code, uint8_t level, const String& msg);

			void fire(const String& extraMessage = "", OutputHandlerBase* outputHandler = nullptr, BaseObject& userData = BaseObject::InvalidRef(), int32_t _line_num = 0, const String& _file_name = "");

		private:
			uint8_t m_errGroup { 0x00 };
			uint8_t m_errLevel { tErrorLevel::NoError };
			uint64_t m_errCode { 0x0000000000000000 };
			String m_message { "" };

	};
}
