#pragma once

#include <ostd/Color.hpp>
#include <unordered_map>

namespace ostd
{
	class TextStyleParser
	{
		public: enum class eBlockParserReturnValue { CloseBlock = 0, ValidBlock, InvalidBlock };

		public: struct tColor {
			Color fullColor;
			String consoleColor;

			void convertToBackground(void);
			void convertToForeground(void);
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

		private:
			static eBlockParserReturnValue parse_block(const String& blockString, tColor& outBackgroundColor, tColor& outForegroundColor);
			static const tColor parse_color(const String& colorStr);

		private:
			inline static tColor s_defaultBackgroundColor;
			inline static tColor s_defaultForegroundColor;
			inline static const String BlockString_Close = "/";
			inline static const String BlockString_Style = "style";

		public:
			inline static std::unordered_map<String, tColor> ConsoleColors {
				{ "red", { { 255, 0, 0, 255 }, "red" } },
				{ "brightred", { { 255, 70, 70, 255 }, "b-red" } },
				{ "green", { { 0, 255, 0, 255 }, "green" } },
				{ "brightgreen", { { 70, 255, 70, 255 }, "b-green" } },
				{ "blue", { { 0, 255, 0, 255 }, "blue" } },
				{ "brightblue", { { 70, 70, 255, 255 }, "b-blue" } },
				{ "magenta", { { 255, 0, 255, 255 }, "magenta" } },
				{ "brightmagenta", { { 255, 120, 255, 255 }, "b-magenta" } },
				{ "cyan", { { 0, 255, 255, 255 }, "cyan" } },
				{ "brightcyan", { { 170, 120, 255, 255 }, "b-cyan" } },
				{ "yellow", { { 255, 255, 0, 255 }, "yellow" } },
				{ "brightyellow", { { 255, 255, 170, 255 }, "b-yellow" } },
				{ "black", { { 0, 0, 0, 255 }, "gray" } },
				{ "Gray", { { 50, 50, 50, 255 }, "b-gray" } },
				{ "brightgray", { { 150, 150, 150, 255 }, "lgray" } },
				{ "white", { { 255, 255, 255, 255 }, "white" } }
			};
	};

	class TextStyleBuilder
	{
		public: class Console {
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

				Console& print(IOutputHandler& out);
				Console& print(void);

				inline TextStyleParser::tStyledString getStyledString(void) const { return m_styledString; }

				friend std::ostream& operator<<(std::ostream&, Console const&);

			private:
				TextStyleParser::tColor find_color(const String& consoleColor);

			private:
				TextStyleParser::tStyledString m_styledString;
				TextStyleParser::tColor m_backgroundColor;
				TextStyleParser::tColor m_foregroundColor;
		};

		//TODO:  Implement
		// public: class FullColor {

		// };
	};

	typedef TextStyleBuilder::Console ConsoleRichString;
}