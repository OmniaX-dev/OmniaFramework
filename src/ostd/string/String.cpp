#include "String.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <bitset>
#include <boost/regex.hpp>

namespace ostd
{
	String String::Tokens::next(void)
	{
		if (!hasNext()) return ""; //TODO: Error
		return m_tokens[m_current_index++];
	}

	String String::Tokens::previous(void)
	{
		if (!hasPrevious()) return ""; //TODO: Error
		return m_tokens[--m_current_index];
	}


	String& String::ltrim(void)
	{
		m_data.erase(m_data.begin(), std::find_if(m_data.begin(), m_data.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
		return *this;
	}

	String& String::rtrim(void)
	{
		m_data.erase(std::find_if(m_data.rbegin(), m_data.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), m_data.end());
		return *this;
	}

	String& String::trim(void)
	{
		return ltrim().rtrim();
	}

	String& String::toLower(void)
	{
		std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](unsigned char c){ return std::tolower(c); });
		return *this;
	}

	String& String::toUpper(void)
	{
		std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](unsigned char c){ return std::toupper(c); });
		return *this;
	}

	String& String::addPadding(uint32_t new_string_length, char c, ePaddingBehavior padding_behavior)
	{
		if (len() - 1 >= new_string_length) return *this;
		uint32_t diff = new_string_length - len();
		int32_t leftPadding = 0;
		int32_t rightPadding = 0;
		switch (padding_behavior)
		{
			case ePaddingBehavior::ForceEvenNegative:
				if (diff % 2 != 0)
					diff--;
				leftPadding = rightPadding = (diff / 2);
			break;
			case ePaddingBehavior::ForceEvenPositive:
				if (diff % 2 != 0)
					diff++;
				leftPadding = rightPadding = (diff / 2);
			break;
			case ePaddingBehavior::AllowOddExtraLeft:
				if (diff % 2 != 0)
				{
					diff--;
					leftPadding = rightPadding = (diff / 2);
					leftPadding++;
				}
				else
					leftPadding = rightPadding = (diff / 2);
			break;
			case ePaddingBehavior::AllowOddExtraRight:
				if (diff % 2 != 0)
				{
					diff--;
					leftPadding = rightPadding = (diff / 2);
					rightPadding++;
				}
				else
					leftPadding = rightPadding = (diff / 2);
			break;
		}
		if (leftPadding < 0 || rightPadding < 0) return *this;
		return addLeftPadding(len() + leftPadding, c).addRightPadding(len() + rightPadding, c);
	}

	String& String::addLeftPadding(uint32_t new_string_length, char c)
	{
		reverse();
		while (len() < new_string_length)
			m_data += c;
		reverse();
		return *this;
	}

	String& String::addRightPadding(uint32_t new_string_length, char c)
	{
		while (len() < new_string_length)
			m_data += c;
		return *this;
	}

	String& String::reverse(void)
	{
		std::reverse(m_data.begin(), m_data.end());
		return *this;
	}

	String& String::replaceAll(const String& what, const String& with)
	{
		while (contains(what))
			replaceFirst(what, with);
		return *this;
	}

	String& String::replaceFirst(const String& what, const String& with)
	{
		int32_t index = indexOf(what);
		if (index == -1) return *this;
		m_data.replace(index, what.len(), with.cpp_str());
		return *this;
	}

	String& String::regexReplace(const String& regex_pattern, const String& replace_with, bool case_insensitive)
	{
		try
		{
			boost::regex rgx(regex_pattern.cpp_str(), (case_insensitive ? boost::regex_constants::icase : boost::regex_constants::normal));
			m_data = boost::regex_replace(m_data, rgx, replace_with.cpp_str());
			return *this;
		}
		catch(const boost::regex_error& err)
		{
			std::cerr << err.what() << '\n'; //TODO: Better error handling
			return *this;
		}
		return *this;
	}

	String& String::put(uint32_t index, char c)
	{
		if (index < m_data.length())
			m_data[index] = c;
		return *this;
	}

	String& String::substr(uint32_t start, int32_t end)
	{
		if (end < 0) m_data = m_data.substr(start);
		else m_data = m_data.substr(start, end - start);
		return *this;
	}

	String& String::fixedLength(uint32_t length, char fill_character, const String& truncate_indicator)
	{
		if (length < truncate_indicator.len()) return *this;
		if (len() > length)
		{
			int32_t tr_len = truncate_indicator.len();
			substr(0, length - tr_len);
			return add(truncate_indicator);
		}
		else if (len() < length)
			return addRightPadding(length, fill_character);
		return *this;
	}


	String& String::addChar(char c)
	{
		m_data += c;
		return *this;
	}

	String& String::add(const String& se)
	{
		m_data += se.cpp_str();
		return *this;
	}

	String& String::add(uint8_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(int8_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(uint16_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(int16_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(uint32_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(int32_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(uint64_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(int64_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(float f, uint8_t precision)
	{
		std::stringstream stream;
		if (precision != 0)
			stream << std::fixed << std::setprecision(precision) << f;
		else
			stream << f;
		cpp_string s = stream.str();
		return add(s);
	}

	String& String::add(double f, uint8_t precision)
	{
		std::stringstream stream;
		if (precision != 0)
			stream << std::fixed << std::setprecision(precision) << f;
		else
			stream << f;
		cpp_string s = stream.str();
		return add(s);
	}



	//New String
	String String::new_ltrim(void) const
	{
		String __str = m_data;
		return __str.ltrim();
	}

	String String::new_rtrim(void) const
	{
		String __str = m_data;
		return __str.rtrim();
	}

	String String::new_trim(void) const
	{
		String __str = m_data;
		return __str.trim();
	}

	String String::new_toLower(void) const
	{
		String __str = m_data;
		return __str.toLower();
	}

	String String::new_toUpper(void) const
	{
		String __str = m_data;
		return __str.toUpper();
	}

	String String::new_addPadding(uint32_t new_string_length, char c, ePaddingBehavior padding_behavior) const
	{
		String __str = m_data;
		return __str.addPadding(new_string_length, c, padding_behavior);
	}

	String String::new_addLeftPadding(uint32_t new_string_length, char c) const
	{
		String __str = m_data;
		return __str.addLeftPadding(new_string_length, c);
	}

	String String::new_addRightPadding(uint32_t new_string_length, char c) const
	{
		String __str = m_data;
		return __str.addRightPadding(new_string_length, c);
	}

	String String::new_reverse(void) const
	{
		String __str = m_data;
		return __str.reverse();
	}

	String String::new_replaceAll(const String& what, const String& with) const
	{
		String __str = m_data;
		return __str.replaceAll(what, with);
	}

	String String::new_replaceFirst(const String& what, const String& with) const
	{
		String __str = m_data;
		return __str.replaceFirst(what, with);
	}

	String String::new_regexReplace(const String& regex_pattern, const String& replace_with, bool case_insensitive) const
	{
		String __str = m_data;
		return __str.regexReplace(regex_pattern, replace_with, case_insensitive);
	}

	String String::new_put(uint32_t index, char c) const
	{
		String __str = m_data;
		return __str.put(index, c);
	}

	String String::new_substr(uint32_t start, int32_t end) const
	{
		String __str = m_data;
		return __str.substr(start, end);
	}

	String String::new_fixedLength(uint32_t length, char fill_character, const String& truncate_indicator) const
	{
		String __str = m_data;
		return __str.fixedLength(length, fill_character, truncate_indicator);
	}


	String String::new_addChar(char c) const
	{
		String __str = m_data;
		return __str.addChar(c);
	}

	String String::new_add(const String& se) const
	{
		String __str = m_data;
		return __str.add(se);
	}

	String String::new_add(uint8_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(int8_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(uint16_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(int16_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(uint32_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(int32_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(uint64_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(int64_t i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(float f, uint8_t precision) const
	{
		String __str = m_data;
		return __str.add(f, precision);
	}

	String String::new_add(double f, uint8_t precision) const
	{
		String __str = m_data;
		return __str.add(f, precision);
	}



	//Utility
	int64_t String::toInt(void) const
	{
		if (!isNumeric(false)) return 0;
		ostd::String str = String(m_data).trim().toLower();
		if (!str.isInt()) return 0;
		int32_t base = 10;
		if (str.cpp_str().rfind("0x", 0) == 0)
		{
			str = str.substr(2);
			base = 16;
		}
		else if (str.cpp_str().rfind("0b", 0) == 0)
		{
			str = str.substr(2);
			base = 2;
		}
		return strtol(str.c_str(), NULL, base);
	}

	float String::toFloat(void) const
	{
		if (!isNumeric(true)) return 0;
		return std::stof(m_data);
	}

	double String::toDouble(void) const
	{
		if (!isNumeric(true)) return 0;
		return std::stod(m_data);
	}

	bool String::isNumeric(bool decimal) const
	{
		if (decimal)
		{
			std::istringstream iss(m_data);
			double f;
			iss >> std::noskipws >> f;
			return iss.eof() && !iss.fail();
		}
		return isInt();
	}

	bool String::isInt(void) const
	{
		ostd::String str = String(m_data).trim().toLower();
		bool isNumber = std::ranges::all_of(str.begin(), str.end(), [](char c){ return isdigit(c) != 0; });
		return str.isHex() || str.isBin() || isNumber;
	}

	bool String::isHex(void) const
	{
		ostd::String hex = String(m_data).trim().toLower();
		return hex.cpp_str().compare(0, 2, "0x") == 0 && hex.cpp_str().size() > 2 && hex.cpp_str().find_first_not_of("0123456789abcdef", 2) == std::string::npos;
	}

	bool String::isBin(void) const
	{
		ostd::String bin = String(m_data).trim().toLower();
		return bin.cpp_str().compare(0, 2, "0b") == 0 && bin.cpp_str().size() > 2 && bin.cpp_str().find_first_not_of("01", 2) == std::string::npos;
	}

	bool String::contains(char c) const
	{
		return m_data.find(c) != std::string::npos;
	}

	bool String::contains(const String& str) const
	{
		return m_data.find(str.cpp_str()) != std::string::npos;
	}

	bool String::startsWith(const String& str) const
	{
		return m_data.starts_with(str);
	}

	bool String::endsWith(const String& str) const
	{
		return m_data.ends_with(str);
	}

	uint32_t String::count(const String& str) const
	{
		Tokens tok = tokenize(str, false, true);
		if (tok.count() < 1) return 0;
		return tok.count() - 1;
	}

	int32_t String::indexOf(char c, uint32_t start) const
	{
		cpp_string cc = "";
		cc += c;
		int32_t pos = m_data.find(cc.c_str(), start);
		if (pos == std::string::npos) return -1;
		return pos;
	}

	int32_t String::indexOf(const String& str, uint32_t start) const
	{
		int32_t pos = m_data.find(str.c_str(), start);
		if (pos == std::string::npos) return -1;
		return pos;
	}

	int32_t String::lastIndexOf(char c) const
	{
		String se(m_data);
		se.reverse();
		int32_t pos = se.indexOf(c);
		if (pos < 0) return -1;
		return len() - pos - 1;
	}

	int32_t String::lastIndexOf(const String& str) const
	{
		String se(m_data);
		se.reverse();
		String se2(str);
		int32_t pos = se.indexOf(se2.new_reverse());
		if (pos < 0) return -1;
		return len() - pos - str.len();
	}

	String::Tokens String::tokenize(const String& delimiter, bool trim_tokens, bool allow_white_space_only_tokens) const
	{
		Tokens tokens;
		int32_t sindex = 0;
		int32_t eindex = 0;
		String __token = "";
		while ((eindex = indexOf(delimiter, sindex)) != -1)
		{
			__token = new_substr(sindex, eindex);
			if (trim_tokens)
			{
				__token.trim();
				if (__token != "")
					tokens.m_tokens.push_back(__token.cpp_str());
			}
			else
			{
				if (allow_white_space_only_tokens && __token != "")
					tokens.m_tokens.push_back(__token.cpp_str());
				else if (!allow_white_space_only_tokens && __token.new_trim() != "")
					tokens.m_tokens.push_back(__token.cpp_str());
			}
			sindex = eindex + delimiter.len();
		}
		__token = new_substr(sindex);
		if (trim_tokens)
		{
			__token.trim();
			if (__token != "")
				tokens.m_tokens.push_back(__token.cpp_str());
		}
		else
		{
			if (allow_white_space_only_tokens && __token != "")
				tokens.m_tokens.push_back(__token.cpp_str());
			else if (!allow_white_space_only_tokens && __token.new_trim() != "")
				tokens.m_tokens.push_back(__token.cpp_str());
		}
		return tokens;
	}

	String String::getHexStr(uint64_t value, bool prefix, uint8_t nbytes)
	{
		union {
			uint64_t val;
			uint8_t bytes[8];
		} __tmp_editor;
		__tmp_editor.val = value;
		if (nbytes < 1 || nbytes > 8) nbytes = 1;
		std::ostringstream oss;
		if (prefix) oss << "0x";
		for (int8_t b = nbytes - 1; b >= 0; b--)
			oss << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (int)__tmp_editor.bytes[b];
		return oss.str();
	}

	String String::getBinStr(uint64_t value, bool prefix, uint8_t nbytes)
	{
		union {
			uint64_t val;
			uint8_t bytes[8];
		} __tmp_editor;
		__tmp_editor.val = value;
		if (nbytes < 1 || nbytes > 8) nbytes = 1;
		std::ostringstream oss;
		if (prefix) oss << "0b ";
		for (int8_t b = nbytes - 1; b >= 0; b--)
			oss << std::bitset<8>((char)__tmp_editor.bytes[b]) << " ";
		return oss.str();
	}

	String String::duplicateChar(unsigned char c, uint16_t count)
	{
		String str = "";
		for (uint16_t i = 0; i < count; i++)
			str = str += c;
		return str;
	}



	String operator+(const cpp_string& str1, const String& str)
	{
		return String(str1) + str;
	}

	std::ostream& operator<<(std::ostream& out, const String& val)
	{
		out << val.cpp_str();
		return out;
	}
}
