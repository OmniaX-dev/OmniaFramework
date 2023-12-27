#include "TextStyleParser.hpp"
#include "Utils.hpp"
#include "IOHandlers.hpp"

namespace ostd
{
	std::ostream &operator<<(std::ostream &os, TextStyleParser::tStyledString const &str)
	{ 
		if (!str.validate()) return os;
		ostd::ConsoleOutputHandler out;
		for (int32_t i = 0; i < str.text.length(); i++)
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
		for (int32_t i = 0; i < str.length(); i++)
		{
			backgroundColors.push_back(background);
			foregroundColors.push_back(foreground);
		}
	}

	bool TextStyleParser::tStyledString::validate(void) const
	{
		return text.length() > 0 && text.length() == backgroundColors.size() && text.length() == foregroundColors.size();
	}

	TextStyleParser::tStyledString TextStyleParser::parse(const StringEditor& styledString)
	{
		return parse(styledString, convertColor("black"), convertColor("white"));
	}

	TextStyleParser::tStyledString TextStyleParser::parse(const StringEditor& styledString, tColor defaultBackgorundColor, tColor defaultForegroundColor)
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
		String _styledString = StringEditor(styledString).trim().str();
		for (int32_t i = 0; i < _styledString.length(); i++)
		{
			char c = _styledString[i];
			if (c == '[')
			{
				if (insideBlock)
					return tStyledString(); //TODO: Error, no nested blocks allowed
				if (test_for_block(StringEditor(_styledString).substr(i)))
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
				rstring.add(StringEditor("").add(c).str(), bgcol, fgcol);
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

	TextStyleParser::tColor TextStyleParser::convertColor(const StringEditor& name)
	{
		StringEditor colorStrEditor = name;
		colorStrEditor.trim().toLower();
		if (ConsoleColors::isConsoleColor(colorStrEditor))
		{
			auto& cc = ConsoleColors::getFromName(colorStrEditor);
			return { cc.fullColor, cc.name, cc.background };
		}
		tColor col;
		col.consoleColor = "black";
		col.fullColor.set(colorStrEditor.str());
		col.background = false;
		return col;
	}

	bool TextStyleParser::test_for_block(const String& block_part)
	{
		if (block_part.length() < 3) return false;
		if (block_part.starts_with("[@@"))
		{
			return true;
		}
		return false;
	}

	TextStyleParser::eBlockParserReturnValue TextStyleParser::parse_block(const String& blockString, tColor& outBackgroundColor, tColor& outForegroundColor)
	{
		StringEditor blockEditor = blockString;
		blockEditor.trim().toLower();
		if (blockEditor.str() == BlockString_Close)
		{
			outBackgroundColor = s_defaultBackgroundColor;
			outForegroundColor = s_defaultForegroundColor;
			return eBlockParserReturnValue::CloseBlock;
		}
		if (blockEditor.startsWith(BlockString_Style))
		{
			blockEditor = blockEditor.substr(BlockString_Style.length());
			blockEditor.trim();
			blockEditor.replaceAll(" ", "");
			auto tokens = blockEditor.tokenize(",");
			for (auto param : tokens)
			{
				StringEditor paramEdit = param;
				if (!paramEdit.contains(":"))
					continue; //TODO: Error, invalid param
				StringEditor name = paramEdit.substr(0, paramEdit.indexOf(":"));
				name.trim();
				StringEditor value = paramEdit.substr(paramEdit.indexOf(":") + 1);
				value.trim();
				if (name.str() == "background")
				{
					auto col = convertColor(value.str());
					outBackgroundColor = col;
				}
				else if (name.str() == "foreground")
				{
					auto col = convertColor(value.str());
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
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int8_t i)
	{
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(uint16_t i)
	{
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int16_t i)
	{
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(uint32_t i)
	{
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int32_t i)
	{
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(uint64_t i)
	{
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(int64_t i)
	{
		StringEditor edit("");
		edit.addi(i);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(float f)
	{
		StringEditor edit("");
		edit.addf(f);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::add(double f)
	{
		StringEditor edit("");
		edit.addf(f);
		return add(edit.str());
	}

	TextStyleBuilder::Console& TextStyleBuilder::Console::addc(char c)
	{
		StringEditor edit("");
		edit.add(c);
		return add(edit.str());
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



	
	TextStyleBuilder::Regex& TextStyleBuilder::Regex::setRawString(const StringEditor& rawString)
	{
		m_rawString = rawString.str();
		return *this;
	}

	TextStyleBuilder::Regex& TextStyleBuilder::Regex::fg(const StringEditor& regex, const StringEditor& foreground_color, bool case_insensitive)
	{
		String replace_pattern = "[@@ style foreground:" + foreground_color.str();
		replace_pattern += "]$&[@@/]";
		m_rawString = StringEditor(m_rawString).regexReplace(regex.str(), replace_pattern, case_insensitive).str();
		return *this;
	}

	TextStyleBuilder::Regex& TextStyleBuilder::Regex::bg(const StringEditor& regex, const StringEditor& background_color, bool case_insensitive)
	{
		String replace_pattern = "[@@ style background:" + background_color.str();
		replace_pattern += "]$&[@@/]";
		m_rawString = StringEditor(m_rawString).regexReplace(regex.str(), replace_pattern, case_insensitive).str();
		return *this;
	}
	
	TextStyleBuilder::Regex& TextStyleBuilder::Regex::col(const StringEditor& regex, const StringEditor& foreground_color, const StringEditor& background_color, bool case_insensitive)
	{
		String replace_pattern = "[@@ style background:" + background_color.str();
		replace_pattern += ", foreground:" + foreground_color.str();
		replace_pattern += "]$&[@@/]";
		m_rawString = StringEditor(m_rawString).regexReplace(regex.str(), replace_pattern, case_insensitive).str();
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