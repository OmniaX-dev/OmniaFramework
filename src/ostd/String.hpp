#pragma once

#include <ostd/Types.hpp>

namespace ostd
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