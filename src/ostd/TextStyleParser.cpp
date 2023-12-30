#include "TextStyleParser.hpp"
#include "Utils.hpp"
#include "IOHandlers.hpp"

namespace ostd
{
	std::ostream &operator<<(std::ostream &os, TextStyleParser::tStyledString const &str)
	{ 
		if (!str.validate()) return os;
		ostd::ConsoleOutputHandler out;
		for (int32_t i = 0; i < str.text.len(); i++)
			out.bg(str.backgroundColors[i].consoleColor).fg(str.foregroundColors[i].consoleColor).pChar(str.text[i]);
		out.reset();
		return os;
	}

	std::ostream &operator<<(std::ostream &os, TextStyleBuilder::Console const &builder)
	{ 
		return os << builder.getStyledString();
	}

	std::ostream &operator<<(std::ostream &os, TextStyleBuilder::Regex const &regex)
	{ 
		return os << regex.getStyledString();
	}

	std::ostream &operator<<(std::ostream &os, TextStyleBuilder::IRichStringBase const &rstr)
	{ 
		return os << rstr.getStyledString();
	}

	void TextStyleParser::tStyledString::add(const String& str, tColor background, tColor foreground)
	{
		text += str;
		background.background = true;
		foreground.background = false;
		for (int32_t i = 0; i < str.len(); i++)
		{
			backgroundColors.push_back(background);
			foregroundColors.push_back(foreground);
		}
	}

	bool TextStyleParser::tStyledString::validate(void) const
	{
		return text.len() > 0 && text.len() == backgroundColors.size() && text.len() == foregroundColors.size();
	}

	TextStyleParser::tStyledString TextStyleParser::parse(const String& styledString)
	{
		return parse(styledString, convertColor("black"), convertColor("white"));
	}

	TextStyleParser::tStyledString TextStyleParser::parse(const String& styledString, tColor defaultBackgorundColor, tColor defaultForegroundColor)
	{
		tStyledString rstring;
		bool insideBlock = false;
		bool validBlockStart = false;
		int32_t countBlockStart = 0;
		defaultBackgorundColor.background = true;
		defaultForegroundColor.background = false;
		s_defaultBackgroundColor = defaultBackgorundColor;
		s_defaultForegroundColor = defaultForegroundColor;
		tColor fgcol = defaultForegroundColor;
		tColor bgcol = defaultBackgorundColor;
		String blockText = "";
		String _styledString = String(styledString).trim();
		for (int32_t i = 0; i < _styledString.len(); i++)
		{
			char c = _styledString[i];
			if (c == '[')
			{
				if (insideBlock)
					return tStyledString(); //TODO: Error, no nested blocks allowed
				if (test_for_block(String(_styledString).substr(i)))
				{
					insideBlock = true;
					continue;
				}
			}
			if(c == ']')
			{
				if (insideBlock)
				{
					insideBlock = false;
					validBlockStart = false;
					countBlockStart = 0;
					auto blockParseResult = parse_block(blockText, bgcol, fgcol);
					if (blockParseResult == eBlockParserReturnValue::InvalidBlock)
						return tStyledString(); //TODO: Error, Invalid block
					blockText = "";
					continue;
				}
			}
			if (!insideBlock)
			{
				rstring.add(String("").addChar(c), bgcol, fgcol);
				continue;
			}
			if (validBlockStart)
			{
				blockText += c;
				continue;
			}
			if (c == '@')
			{
				countBlockStart++;
				if (countBlockStart == 2)
				{
					validBlockStart = true;
					continue;
				}
			}
			else
			{
				return tStyledString(); //TODO: Error, invalid block start sequence
			}
		}
		return rstring;
	}

	TextStyleParser::tColor TextStyleParser::convertColor(const String& name)
	{
		String colorStrEditor = name;
		colorStrEditor.trim().toLower();
		if (ConsoleColors::isConsoleColor(colorStrEditor))
		{
			auto& cc = ConsoleColors::getFromName(colorStrEditor);
			return { cc.fullColor, cc.name, cc.background };
		}
		tColor col;
		col.consoleColor = "black";
		col.fullColor.set(colorStrEditor);
		col.background = false;
		return col;
	}

	bool TextStyleParser::test_for_block(const String& block_part)
	{
		if (block_part.len() < 3) return false;
		if (block_part.startsWith("[@@"))
		{
			return true;
		}
		return false;
	}

	TextStyleParser::eBlockParserReturnValue TextStyleParser::parse_block(const String& blockString, tColor& outBackgroundColor, tColor& outForegroundColor)
	{
		String blockEditor = blockString;
		blockEditor.trim().toLower();
		if (blockEditor == BlockString_Close)
		{
			outBackgroundColor = s_defaultBackgroundColor;
			outForegroundColor = s_defaultForegroundColor;
			return eBlockParserReturnValue::CloseBlock;
		}
		if (blockEditor.startsWith(BlockString_Style))
		{
			blockEditor = blockEditor.substr(BlockString_Style.len());
			blockEditor.trim();
			blockEditor.replaceAll(" ", "");
			auto tokens = blockEditor.tokenize(",");
			for (auto param : tokens)
			{
				String paramEdit = param;
				if (!paramEdit.contains(":"))
					continue; //TODO: Error, invalid param
				String name = paramEdit.new_substr(0, paramEdit.indexOf(":"));
				name.trim();
				String value = paramEdit.new_substr(paramEdit.indexOf(":") + 1);
				value.trim();
				if (name == "background")
				{
					auto col = convertColor(value);
					outBackgroundColor = col;
				}
				else if (name == "foreground")
				{
					auto col = convertColor(value);
					outForegroundColor = col;
				}
				else continue; //TODO: Error, unknown style parameter
			}
		}
		return eBlockParserReturnValue::ValidBlock;
	}


	
	TextStyleBuilder::Console::Console(void)
	{
		m_backgroundColor = TextStyleParser::convertColor("black");
		m_backgroundColor.background = true;
		m_foregroundColor = TextStyleParser::convertColor("white");
		m_backgroundColor.background = false;
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::bg(const String& consoleColor)
	{
		m_backgroundColor = find_color(consoleColor);
		m_backgroundColor.background = true;
		return *this;
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::fg(const String& consoleColor)
	{
		m_foregroundColor = find_color(consoleColor);
		m_foregroundColor.background = false;
		return *this;
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::clr(void)
	{
		m_styledString.backgroundColors.clear();
		m_styledString.foregroundColors.clear();
		m_styledString.text = "";
		return *this;
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(const String& text)
	{
		m_styledString.add(text, m_backgroundColor, m_foregroundColor);
		return *this;
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(uint8_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int8_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(uint16_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int16_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(uint32_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int32_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(uint64_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int64_t i)
	{
		String edit("");
		edit.add(i);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(float f)
	{
		String edit("");
		edit.add(f);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(double f)
	{
		String edit("");
		edit.add(f);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::addc(char c)
	{
		String edit("");
		edit.addChar(c);
		return add(edit);
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::print(OutputHandlerBase& out)
	{
		out.pStyled(m_styledString);
		return *this;
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::print(void)
	{
		ConsoleOutputHandler out;
		out.pStyled(m_styledString);
		return *this;
	}

	TextStyleParser::tColor TextStyleBuilder::Console::find_color(const String& consoleColor)
	{
		return TextStyleParser::convertColor(consoleColor);
	}



	
	TextStyleBuilder::Regex& TextStyleBuilder::Regex::setRawString(const String& rawString)
	{
		m_rawString = rawString;
		return *this;
	}

	TextStyleBuilder::Regex& TextStyleBuilder::Regex::fg(const String& regex, const String& foreground_color, bool case_insensitive)
	{
		String replace_pattern = "[@@ style foreground:" + foreground_color;
		replace_pattern += "]$&[@@/]";
		m_rawString = String(m_rawString).regexReplace(regex, replace_pattern, case_insensitive);
		return *this;
	}

	TextStyleBuilder::Regex& TextStyleBuilder::Regex::bg(const String& regex, const String& background_color, bool case_insensitive)
	{
		String replace_pattern = "[@@ style background:" + background_color;
		replace_pattern += "]$&[@@/]";
		m_rawString = String(m_rawString).regexReplace(regex, replace_pattern, case_insensitive);
		return *this;
	}
	
	TextStyleBuilder::Regex& TextStyleBuilder::Regex::col(const String& regex, const String& foreground_color, const String& background_color, bool case_insensitive)
	{
		String replace_pattern = "[@@ style background:" + background_color;
		replace_pattern += ", foreground:" + foreground_color;
		replace_pattern += "]$&[@@/]";
		m_rawString = String(m_rawString).regexReplace(regex, replace_pattern, case_insensitive);
		return *this;
	}
	
	TextStyleParser::tStyledString TextStyleBuilder::Regex::getStyledString(void) const
	{
		return TextStyleParser::parse(m_rawString);
	}
	
	TextStyleBuilder::Regex& TextStyleBuilder::Regex::print(void)
	{
		ConsoleOutputHandler out;
		out.pStyled(m_rawString);
		return *this;
	}
	
	TextStyleBuilder::Regex& TextStyleBuilder::Regex::print(OutputHandlerBase& out)
	{
		out.pStyled(m_rawString);
		return *this;
	}
	

}