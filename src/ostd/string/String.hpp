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

#include <cstring>
#include <filesystem>
#include <iostream>
#include <ostd/data/Types.hpp>

#define STR_BOOL(b) (b ? "true" : "false")

namespace ostd
{
	class String
	{
		public: class Tokens
		{
			public:
				inline bool hasNext(void) { return m_tokens.size() > 0 && m_current_index < m_tokens.size(); }
				inline bool hasPrevious(void) { return m_tokens.size() > 0 && m_current_index > 0; }
				String next(void);
				String previous(void);
				inline u32 count(void) { return m_tokens.size(); }
				inline stdvec<String> getRawData(void) { return m_tokens; }
				inline u32 getCurrentIndex(void) { return m_current_index; }
				inline void cycle(void) { m_current_index = 0; }

				inline auto begin(void) { return m_tokens.begin(); }
				inline auto end(void) { return m_tokens.end(); }
				inline auto cbegin(void) const { return m_tokens.begin(); }
				inline auto cend(void) const { return m_tokens.end(); }
				inline auto begin(void) const { return m_tokens.begin(); }
				inline auto end(void) const { return m_tokens.end(); }

			private:
				inline Tokens(void) { m_current_index = 0; }

			private:
				stdvec<String> m_tokens;
				u32 m_current_index;

				friend class String;
		};
		public: enum class ePaddingBehavior { ForceEvenPositive = 1, ForceEvenNegative, AllowOddExtraLeft, AllowOddExtraRight };
		public:
			inline String(void) { m_data = ""; }
			inline String(const cpp_string& str) { m_data = str; }
			inline String(const char* str) { m_data = str; }
			inline String(const std::filesystem::path& path) { m_data = path.string(); }

			inline cpp_string cpp_str(void) const { return m_data; }
			inline cpp_string& cpp_str_ref(void) { return m_data; }
			inline const char* c_str(void) const { return m_data.c_str(); }
			inline char at(u32 index) const { return m_data[index]; }
			inline u32 len(void) const { return m_data.length(); }
			inline char operator[](u32 index) const { return m_data[index]; }
			inline bool operator== (const char* str2) const { return std::strcmp(c_str(), str2) == 0; }
			   inline bool operator!= (const char* str2) const { return std::strcmp(c_str(), str2) != 0; }
			inline String operator+(const String& str2) const { return m_data + str2.cpp_str(); }
			friend String operator+(const cpp_string& str1, const String& str);
			inline String& operator+=(const String& str2) { m_data += str2.cpp_str(); return *this; }
			inline String& operator+=(const char& c) { m_data += c; return *this; }
			inline operator std::string() const { return m_data; }
			inline operator const char*() const { return c_str(); }
			inline operator std::filesystem::path() const { return cpp_str(); }
			inline String& clr(void) { m_data = ""; return *this; }
			inline String& set(const cpp_string& str) { m_data = str; return *this; }
			inline stdvec<u32> getUTF8Codepoints(void) const { return decodeUTF8(m_data); }

			inline auto begin(void) { return m_data.begin(); }
			inline auto end(void) { return m_data.end(); }
			inline auto cbegin(void) const { return m_data.begin(); }
			inline auto cend(void) const { return m_data.end(); }
			inline auto begin(void) const { return m_data.begin(); }
			inline auto end(void) const { return m_data.end(); }

			//Modifiers
			String& ltrim(void);
			String& rtrim(void);
			String& trim(void);
			String& toLower(void);
			String& toUpper(void);
			String& addPadding(u32 new_string_length, char c = ' ', ePaddingBehavior padding_behavior = ePaddingBehavior::ForceEvenPositive);
			String& addLeftPadding(u32 new_string_length, char c = ' ');
			String& addRightPadding(u32 new_string_length, char c = ' ');
			String& reverse(void);
			String& replaceAll(const String& what, const String& with);
			String& replaceFirst(const String& what, const String& with);
			String& regexReplace(const String& regex_pattern, const String& replace_with, bool case_insensitive = false);
			String& put(u32 index, char c);
			String& substr(u32 start, i32 end = -1);
			String& fixedLength(u32 length, char fill_character = ' ', const String& truncate_indicator = "... ");

			String& addChar(char c);
			String& add(const String& se);
			String& add(u8 i);
			String& add(i8 i);
			String& add(u16 i);
			String& add(i16 i);
			String& add(u32 i);
			String& add(i32 i);
			String& add(u64 i);
			String& add(i64 i);
			String& add(f32 f, u8 precision = 0);
			String& add(f64 f, u8 precision = 0);

			//New String
			String new_ltrim(void) const;
			String new_rtrim(void) const;
			String new_trim(void) const;
			String new_toLower(void) const;
			String new_toUpper(void) const;
			String new_addPadding(u32 new_string_length, char c = ' ', ePaddingBehavior padding_behavior = ePaddingBehavior::ForceEvenPositive) const;
			String new_addLeftPadding(u32 new_string_length, char c = ' ') const;
			String new_addRightPadding(u32 new_string_length, char c = ' ') const;
			String new_reverse(void) const;
			String new_replaceAll(const String& what, const String& with) const;
			String new_replaceFirst(const String& what, const String& with) const;
			String new_regexReplace(const String& regex_pattern, const String& replace_with, bool case_insensitive = false) const;
			String new_put(u32 index, char c) const;
			String new_substr(u32 start, i32 end = -1) const;
			String new_fixedLength(u32 length, char fill_character = ' ', const String& truncate_indicator = "... ") const;

			String new_addChar(char c) const;
			String new_add(const String& se) const;
			String new_add(u8 i) const;
			String new_add(i8 i) const;
			String new_add(u16 i) const;
			String new_add(i16 i) const;
			String new_add(u32 i) const;
			String new_add(i32 i) const;
			String new_add(u64 i) const;
			String new_add(i64 i) const;
			String new_add(f32 f, u8 precision = 0) const;
			String new_add(f64 f, u8 precision = 0) const;

			//Utility
			i64 toInt(void) const;
			f32 toFloat(void) const;
			f64 toDouble(void) const;
			bool toBool(void) const;
			bool isBool(void) const;
			bool isNumeric(bool decimal = false) const;
			bool isInt(void) const;
			bool isHex(void) const;
			bool isBin(void) const;
			bool contains(char c) const;
			bool contains(const String& str) const;
			bool startsWith(const String& str) const;
			bool endsWith(const String& str) const;
			bool regexMatches(const String& regex_pattern, bool case_insensitive = false) const;
			u32 count(const String& str) const;
			i32 indexOf(char c, u32 start = 0) const;
			i32 indexOf(const String& str, u32 start = 0) const;
			i32 lastIndexOf(char c) const;
			i32 lastIndexOf(const String& str) const;
			Tokens tokenize(const String& delimiter = " ", bool trim_tokens = true, bool allow_white_space_only_tokens = false) const;

			static String getHexStr(u64 value, bool prefix = true, u8 nbytes = 1);
			static String getBinStr(u64 value, bool prefix = true, u8 nbytes = 1);
			static String duplicateChar(uchar c, u16 count);
			static stdvec<u32> decodeUTF8(const String& s);

			friend std::ostream& operator<<(std::ostream& out, const String& val);

		private:
			static u32 utf8_next(const char*& p, const char* end);

		private:
			ostd::cpp_string m_data;
	};

	struct __i_stringeable
	{
		virtual String toString(void) const = 0;
		friend std::ostream& operator<<(std::ostream& out, const __i_stringeable& val);
		inline operator String() const { return toString(); }
		inline operator std::string() const { return toString(); }
	};

	inline std::ostream& operator<<(std::ostream& out, const __i_stringeable& val)
	{
		out << val.toString();
		return out;
	}
}

using String = ostd::String;

template <>
struct std::hash<String>
{
	std::size_t operator()(const String& str) const
	{
		hash<std::string> hasher;
		return hasher(str.cpp_str());
	}
};
