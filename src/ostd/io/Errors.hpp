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

#include <ostd/data/BaseObject.hpp>
#include <ostd/data/Types.hpp>

#define ERROR_DATA() String(CPP_STR(__LINE__)), String(__FILE__)

namespace ostd
{
	struct tErrorLevel
	{
		inline static constexpr u8 NoError = 0x00;
		inline static constexpr u8 Warning = 0x01;
		inline static constexpr u8 Error = 0x02;
		inline static constexpr u8 Fatal = 0x03;
	};

	class RuntimeError : public BaseObject
	{
		public:
			inline RuntimeError(void) { invalidate(); }
			inline RuntimeError(u8 group, u64 code, u8 level, const String& msg) { create(group, code, level, msg); }
			RuntimeError& create(u8 group, u64 code, u8 level, const String& msg);

			void fire(const String& extraMessage = "", OutputHandlerBase* outputHandler = nullptr, BaseObject& userData = BaseObject::InvalidRef(), i32 _line_num = 0, const String& _file_name = "");

		private:
			u8 m_errGroup { 0x00 };
			u8 m_errLevel { tErrorLevel::NoError };
			u64 m_errCode { 0x0000000000000000 };
			String m_message { "" };

	};
}
