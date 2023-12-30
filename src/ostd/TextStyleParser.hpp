#pragma once

#include <ostd/Color.hpp>
#include <unordered_map>

namespace ostd
{
	class String;
	class TextStyleParser
	{
		public: enum class eBlockParserReturnValue { CloseBlock = 0, ValidBlock, InvalidBlock };

		public: struct tColor {
			Color fullColor;
			String consoleColor;
			bool background;
		};

		public: struct tStyledString {
			String text { "" };
			std::vector<tColor> backgroundColors;
			std::vector<tColor> foregroundColors;

			void add(const String& str, tColor background, tColor foreground);
			bool validate(void) const;

			friend std::ostream& operator<<(std::ostream&, tStyledString const&);
		};

		public:
			static tStyledString parse(const String& styledString);
			static tStyledString parse(const String& styledString, tColor defaultBackgorundColor, tColor defaultForegroundColor);

			static tColor convertColor(const String& name);

		private:
			static bool test_for_block(const String& block_part);
			static eBlockParserReturnValue parse_block(const String& blockString, tColor& outBackgroundColor, tColor& outForegroundColor);

		private:
			inline static tColor s_defaultBackgroundColor;
			inline static tColor s_defaultForegroundColor;
			inline static const String BlockString_Close = "/";
			inline static const String BlockString_Style = "style";
	};

	class TextStyleBuilder
	{
		public: class IRichStringBase {
			public:
				virtual TextStyleParser::tStyledString getStyledString(void) const = 0;
				friend std::ostream& operator<<(std::ostream&, IRichStringBase const&);
		};

		public: class Console : public IRichStringBase {
			public:
				Console(void);
				Console& bg(const String& consoleColor);
				Console& fg(const String& consoleColor);
				Console& clr(void);
				Console& add(const String& text);
				Console& add(uint8_t i);
				Console& add(int8_t i);
				Console& add(uint16_t i);
				Console& add(int16_t i);
				Console& add(uint32_t i);
				Console& add(int32_t i);
				Console& add(uint64_t i);
				Console& add(int64_t i);
				Console& add(float f);
				Console& add(double f);
				Console& addc(char c);

				Console& print(OutputHandlerBase& out);
				Console& print(void);

				inline TextStyleParser::tStyledString getStyledString(void) const override { return m_styledString; }

				friend std::ostream& operator<<(std::ostream&, Console const&);

			private:
				TextStyleParser::tColor find_color(const String& consoleColor);

			private:
				TextStyleParser::tStyledString m_styledString;
				TextStyleParser::tColor m_backgroundColor;
				TextStyleParser::tColor m_foregroundColor;
		};

		public: class Regex : public IRichStringBase {
			public:
				inline Regex(void) { m_rawString = ""; }
				inline Regex(const String& rawString) { setRawString(rawString); }
				inline String getRawString(void) const { return m_rawString; }
				Regex& setRawString(const String& rawString);
				
				Regex& fg(const String& regex, const String& foreground_color, bool case_insensitive = false);
				Regex& bg(const String& regex, const String& background_color, bool case_insensitive = false);
				Regex& col(const String& regex, const String& foreground_color, const String& background_color, bool case_insensitive = false);

				TextStyleParser::tStyledString getStyledString(void) const override;

				Regex& print(void);
				Regex& print(OutputHandlerBase& out);

				friend std::ostream& operator<<(std::ostream&, Regex const&);

			private:
				String m_rawString { "" };
		};

		//TODO:  Implement
		// public: class FullColor {

		// };
	};

	typedef TextStyleBuilder::Console ConsoleRichString;
	typedef TextStyleBuilder::Regex RegexRichString;
}