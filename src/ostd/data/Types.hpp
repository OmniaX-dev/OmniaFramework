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
#include <unordered_map>
#include <ostd/utils/Defines.hpp>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef i8 b8;
typedef float f32;
typedef double f64;
typedef unsigned char uchar;

template<typename T>
using stdvec = std::vector<T>;
template<typename Key, typename Value>
using stdumap = std::unordered_map<Key, Value>;

#define cast static_cast

namespace ostd
{
	typedef std::string cpp_string;

	typedef i64 QWord;
	typedef i32 DWord;
	typedef i16 Word;
	typedef i8 Byte;

	typedef u64 UQWord;
	typedef u32 UDWord;
	typedef u16 UWord;
	typedef u8 UByte;


	typedef uint32_t StreamIndex;

	typedef union {
		f32 val;
		i8 data[4];
		i32 raw;
	} __float_parser;
	typedef union {
		f64 val;
		i8 data[8];
		i64 raw;
	} __double_parser;

	struct tTypeSize
	{
		static inline const u8 BYTE = 1;
		static inline const u8 WORD = 2;
		static inline const u8 DWORD = 4;
		static inline const u8 QWORD = 8;
		static inline const u8 ADDR = 4;
		static inline const u8 FLOAT = 4;
		static inline const u8 DOUBLE = 8;
	};

	typedef stdvec<i8> ByteStream;
}
