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

#include <ostd/data/Types.hpp>
#include <ostd/string/String.hpp>
#include <ostd/io/IOHandlers.hpp>

#define STDVEC_CONTAINS(vec, elem) (std::find(vec.begin(), vec.end(), elem) != vec.end())
#define STDVEC_REMOVE(vec, elem) (vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end()))

namespace ostd
{
	class Memory
	{
		public:
			static void printByteStream(const ByteStream& data, StreamIndex start, u8 line_len, u16 n_rows, OutputHandlerBase& out, i32 addrHighlight = -1, u32 highlightRange = 1, const String& title = "");
			static bool saveByteStreamToFile(const ByteStream& stream, const String& filePath);
			static bool loadByteStreamFromFile(const String& filePath, ByteStream& outStream);
			static ByteStream stringToByteStream(const String& data);
			static String byteStreamToString(const ByteStream& data);

			//Array helpers
			template<typename T>
			static inline T* createArray(size_t size)
			{
				T* array = (T*)malloc(size * sizeof(T));
				return array;
			}
			template<typename T>
			static inline T* resizeArray(T* array, size_t new_size)
			{
				T* new_array = (T*)realloc(array, new_size * sizeof(T));
				return new_array;
			}
			static inline void destroyArray(void* array)
			{
				free(array);
			}
	};
}
