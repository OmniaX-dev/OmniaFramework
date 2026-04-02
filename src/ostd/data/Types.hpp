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

#include <string>
#include <cstdint>
#include <vector>
#include <ostd/utils/Defines.hpp>

namespace ostd
{
	typedef std::string cpp_string;

	typedef int64_t QWord;
	typedef int32_t DWord;
	typedef int16_t Word;
	typedef int8_t Byte;

	typedef uint64_t UQWord;
	typedef uint32_t UDWord;
	typedef uint16_t UWord;
	typedef uint8_t UByte;

	typedef uint32_t StreamIndex;

	typedef union {
		float val;
		Byte data[4];
		int32_t raw;
	} __float_parser;
	typedef union {
		double val;
		Byte data[8];
		int64_t raw;
	} __double_parser;

	struct tTypeSize
	{
		static inline const uint8_t BYTE = 1;
		static inline const uint8_t WORD = 2;
		static inline const uint8_t DWORD = 4;
		static inline const uint8_t QWORD = 8;
		static inline const uint8_t ADDR = 4;
		static inline const uint8_t FLOAT = 4;
		static inline const uint8_t DOUBLE = 8;
	};

	typedef std::vector<Byte> ByteStream;
}
