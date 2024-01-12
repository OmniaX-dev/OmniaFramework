#pragma once

#include <ostd/Types.hpp>
#include <cstring>
#include <filesystem>

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
				inline uint32_t count(void) { return m_tokens.size(); }
				inline std::vector<String> getRawData(void) { return m_tokens; }
				inline uint32_t getCurrentIndex(void) { return m_current_index; }
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
				std::vector<String> m_tokens;
				uint32_t m_current_index;

				friend class String;
		};
		public: enum class ePaddingBehavior { ForceEvenPositive = 1, ForceEvenNegative, AllowOddExtraLeft, AllowOddExtraRight };
		public:
			inline String(void) { m_data = ""; }
			inline String(const cpp_string& str) { m_data = str; }
			inline String(const char* str) { m_data = str; }

			inline cpp_string cpp_str(void) const { return m_data; }
			inline cpp_string& cpp_str_ref(void) { return m_data; }
			inline const char* c_str(void) const { return m_data.c_str(); }
			inline char at(uint32_t index) const { return m_data[index]; }
			inline uint32_t len(void) const { return m_data.length(); }
			inline char operator[](uint32_t index) const { return m_data[index]; }
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
			String& addPadding(uint32_t new_string_length, char c = ' ', ePaddingBehavior padding_behavior = ePaddingBehavior::ForceEvenPositive);
			String& addLeftPadding(uint32_t new_string_length, char c = ' ');
			String& addRightPadding(uint32_t new_string_length, char c = ' ');
			String& reverse(void);
			String& replaceAll(const String& what, const String& with);
			String& replaceFirst(const String& what, const String& with);
			String& regexReplace(const String& regex_pattern, const String& replace_with, bool case_insensitive = false);
			String& put(uint32_t index, char c);
			String& substr(uint32_t start, int32_t end = -1);
			String& fixedLength(uint32_t length, char fill_character = ' ', const String& truncate_indicator = "... ");

			String& addChar(char c);
			String& add(const String& se);
			String& add(uint8_t i);
			String& add(int8_t i);
			String& add(uint16_t i);
			String& add(int16_t i);
			String& add(uint32_t i);
			String& add(int32_t i);
			String& add(uint64_t i);
			String& add(int64_t i);
			String& add(float f, uint8_t precision = 0);
			String& add(double f, uint8_t precision = 0);

			//New String
			String new_ltrim(void) const;
			String new_rtrim(void) const;
			String new_trim(void) const;
			String new_toLower(void) const;
			String new_toUpper(void) const;
			String new_addPadding(uint32_t new_string_length, char c = ' ', ePaddingBehavior padding_behavior = ePaddingBehavior::ForceEvenPositive) const;
			String new_addLeftPadding(uint32_t new_string_length, char c = ' ') const;
			String new_addRightPadding(uint32_t new_string_length, char c = ' ') const;
			String new_reverse(void) const;
			String new_replaceAll(const String& what, const String& with) const;
			String new_replaceFirst(const String& what, const String& with) const;
			String new_regexReplace(const String& regex_pattern, const String& replace_with, bool case_insensitive = false) const;
			String new_put(uint32_t index, char c) const;
			String new_substr(uint32_t start, int32_t end = -1) const;
			String new_fixedLength(uint32_t length, char fill_character = ' ', const String& truncate_indicator = "... ") const;

			String new_addChar(char c) const;
			String new_add(const String& se) const;
			String new_add(uint8_t i) const;
			String new_add(int8_t i) const;
			String new_add(uint16_t i) const;
			String new_add(int16_t i) const;
			String new_add(uint32_t i) const;
			String new_add(int32_t i) const;
			String new_add(uint64_t i) const;
			String new_add(int64_t i) const;
			String new_add(float f, uint8_t precision = 0) const;
			String new_add(double f, uint8_t precision = 0) const;

			//Utility
			int64_t toInt(void) const;
			float toFloat(void) const;
			double toDouble(void) const;
			bool isNumeric(bool decimal = false) const;
			bool contains(char c) const;
			bool contains(const String& str) const;
			bool startsWith(const String& str) const;
			bool endsWith(const String& str) const;
			uint32_t count(const String& str) const;
			int32_t indexOf(char c, uint32_t start = 0) const;
			int32_t indexOf(const String& str, uint32_t start = 0) const;
			int32_t lastIndexOf(char c) const;
			int32_t lastIndexOf(const String& str) const;
			Tokens tokenize(const String& delimiter = " ", bool trim_tokens = true, bool allow_white_space_only_tokens = false) const;

			friend std::ostream& operator<<(std::ostream& out, const String& val);

		private:
			ostd::cpp_string m_data;
	};





	namespace legacy
	{
		class StringEditor
		{
			public: class Tokens
			{
				public:
					inline bool hasNext(void) { return m_tokens.size() > 0 && m_current_index < m_tokens.size(); }
					inline bool hasPrevious(void) { return m_tokens.size() > 0 && m_current_index > 0; }
					String next(void);
					String previous(void);
					inline uint32_t count(void) { return m_tokens.size(); }
					inline std::vector<String> getRawData(void) { return m_tokens; }
					inline uint32_t getCurrentIndex(void) { return m_current_index; }
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
					std::vector<String> m_tokens;
					uint32_t m_current_index;

				public:
					inline static const String END = "%END%";

					friend class StringEditor;
			};

			public:
				inline StringEditor(void) { m_data = ""; }
				inline StringEditor(const String& str) { m_data = str; }
				inline StringEditor(const char* str) { m_data = str; }
				inline String str(void) const { return m_data; }
				inline uint32_t len(void) const { return m_data.length(); }
				inline StringEditor& clr(void) { m_data = ""; return *this; }
				inline const char* c_str(void) const { return m_data.c_str(); }
				inline char at(uint32_t index) const { return m_data[index]; }
				inline char operator[](uint32_t index) const { return m_data[index]; }
				inline StringEditor& set(String str) { m_data = str; return *this; }

				StringEditor& ltrim(void);
				StringEditor& rtrim(void);
				StringEditor& trim(void);

				StringEditor& toLower(void);
				StringEditor& toUpper(void);

				StringEditor& addPadding(char c = ' ', uint32_t count = 10);
				//StringEditor& add(const StringEditor& se);
				StringEditor& add(String str);
				StringEditor& add(char c);
				StringEditor& addi(uint8_t i);
				StringEditor& addi(int8_t i);
				StringEditor& addi(uint16_t i);
				StringEditor& addi(int16_t i);
				StringEditor& addi(uint32_t i);
				StringEditor& addi(int32_t i);
				StringEditor& addi(uint64_t i);
				StringEditor& addi(int64_t i);
				StringEditor& addf(float f);
				StringEditor& addf(double f);

				StringEditor& reverse(void);
				StringEditor& replaceAll(String what, String with);
				StringEditor& replaceFirst(String what, String with);
				StringEditor& regexReplace(String regex_pattern, String replace_with, bool case_insensitive = false);

				StringEditor& put(uint32_t index, char c);

				int64_t toInt(void);
				float toFloat(void);
				double toDouble(void);
				bool isNumeric(bool decimal = false);
				bool contains(char c);
				bool contains(String str);
				bool startsWith(String str);
				bool endsWith(String str);
				String getReverse(void);

				uint32_t count(String str);
				
				int32_t indexOf(char c, uint32_t start = 0);
				int32_t indexOf(String str, uint32_t start = 0);
				int32_t lastIndexOf(char c);
				int32_t lastIndexOf(String str);

				String substr(uint32_t start, int32_t end = -1);
				Tokens tokenize(String delimiter = " ", bool trim_tokens = true, bool allow_white_space_only_tokens = false);

				friend std::ostream& operator<<(std::ostream& out, const StringEditor& val);

			private:
				String m_data;
		};
	}

}



template <>
struct std::hash<ostd::String>
{
	std::size_t operator()(const ostd::String& str) const
	{
		hash<std::string> hasher;
		return hasher(str.cpp_str());
	}
};