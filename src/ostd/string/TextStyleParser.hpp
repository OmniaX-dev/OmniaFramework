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

#include <ostd/data/Color.hpp>

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
			stdvec<tColor> backgroundColors;
			stdvec<tColor> foregroundColors;

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

				inline void setDefaultBackgroundColor(TextStyleParser::tColor color) { m_defaultBackgroundColor = color; }
				inline void setDefaultBackgroundColor(const Color& color) { m_defaultBackgroundColor = { color, "", true }; }
				inline void setDefaultForegroundColor(TextStyleParser::tColor color) { m_defaultForegroundColor = color; }
				inline void setDefaultForegroundColor(const Color& color) { m_defaultForegroundColor = { color, "", true }; }

				inline TextStyleParser::tColor getDefaultBackgroundColor(void) { return m_defaultBackgroundColor; }
				inline TextStyleParser::tColor getDefaultForegroundColor(void) { return m_defaultForegroundColor; }

			protected:
				TextStyleParser::tColor m_defaultBackgroundColor { { 0, 0, 0, 255 }, "black", true };
				TextStyleParser::tColor m_defaultForegroundColor { { 255, 255, 255, 255 }, "white", false };
		};

		public: class Console : public IRichStringBase {
			public:
				Console(void);
				Console& bg(const String& consoleColor);
				Console& fg(const String& consoleColor);
				Console& clr(void);
				Console& add(const String& text);
				Console& add(u8 i);
				Console& add(i8 i);
				Console& add(u16 i);
				Console& add(i16 i);
				Console& add(u32 i);
				Console& add(i32 i);
				Console& add(u64 i);
				Console& add(i64 i);
				Console& add(f32 f);
				Console& add(f64 f);
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
	};

	typedef TextStyleBuilder::Console ConsoleRichString;
	typedef TextStyleBuilder::Regex RegexRichString;
}
