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
		m_data.erase(m_data.begin(), std::find_if(m_data.begin(), m_data.end(), [](uchar ch) {
			return !std::isspace(ch);
		}));
		return *this;
	}

	String& String::rtrim(void)
	{
		m_data.erase(std::find_if(m_data.rbegin(), m_data.rend(), [](uchar ch) {
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
		std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](uchar c){ return std::tolower(c); });
		return *this;
	}

	String& String::toUpper(void)
	{
		std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](uchar c){ return std::toupper(c); });
		return *this;
	}

	String& String::addPadding(u32 new_string_length, char c, ePaddingBehavior padding_behavior)
	{
		if (len() - 1 >= new_string_length) return *this;
		u32 diff = new_string_length - len();
		i32 leftPadding = 0;
		i32 rightPadding = 0;
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

	String& String::addLeftPadding(u32 new_string_length, char c)
	{
		reverse();
		while (len() < new_string_length)
			m_data += c;
		reverse();
		return *this;
	}

	String& String::addRightPadding(u32 new_string_length, char c)
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
		i32 index = indexOf(what);
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

	String& String::put(u32 index, char c)
	{
		if (index < m_data.length())
			m_data[index] = c;
		return *this;
	}

	String& String::substr(u32 start, i32 end)
	{
		if (end < 0) m_data = m_data.substr(start);
		else m_data = m_data.substr(start, end - start);
		return *this;
	}

	String& String::fixedLength(u32 length, char fill_character, const String& truncate_indicator)
	{
		if (length < truncate_indicator.len()) return *this;
		if (len() > length)
		{
			i32 tr_len = truncate_indicator.len();
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

	String& String::add(u8 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(i8 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(u16 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(i16 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(u32 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(i32 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(u64 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(i64 i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	String& String::add(f32 f, u8 precision)
	{
		std::stringstream stream;
		if (precision != 0)
			stream << std::fixed << std::setprecision(precision) << f;
		else
			stream << f;
		cpp_string s = stream.str();
		return add(s);
	}

	String& String::add(f64 f, u8 precision)
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

	String String::new_addPadding(u32 new_string_length, char c, ePaddingBehavior padding_behavior) const
	{
		String __str = m_data;
		return __str.addPadding(new_string_length, c, padding_behavior);
	}

	String String::new_addLeftPadding(u32 new_string_length, char c) const
	{
		String __str = m_data;
		return __str.addLeftPadding(new_string_length, c);
	}

	String String::new_addRightPadding(u32 new_string_length, char c) const
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

	String String::new_put(u32 index, char c) const
	{
		String __str = m_data;
		return __str.put(index, c);
	}

	String String::new_substr(u32 start, i32 end) const
	{
		String __str = m_data;
		return __str.substr(start, end);
	}

	String String::new_fixedLength(u32 length, char fill_character, const String& truncate_indicator) const
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

	String String::new_add(u8 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(i8 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(u16 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(i16 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(u32 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(i32 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(u64 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(i64 i) const
	{
		String __str = m_data;
		return __str.add(i);
	}

	String String::new_add(f32 f, u8 precision) const
	{
		String __str = m_data;
		return __str.add(f, precision);
	}

	String String::new_add(f64 f, u8 precision) const
	{
		String __str = m_data;
		return __str.add(f, precision);
	}



	//Utility
	i64 String::toInt(void) const
	{
		if (!isNumeric(false)) return 0;
		String str = String(m_data).trim().toLower();
		if (!str.isInt()) return 0;
		i32 base = 10;
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
		return strtoul(str.c_str(), NULL, base);
	}

	f32 String::toFloat(void) const
	{
		if (!isNumeric(true)) return 0;
		return std::stof(m_data);
	}

	f64 String::toDouble(void) const
	{
		if (!isNumeric(true)) return 0;
		return std::stod(m_data);
	}

	bool String::toBool(void) const
	{
		String str = new_toLower().trim();
		return str == "true";
	}

	bool String::isBool(void) const
	{
		String str = new_toLower().trim();
		return str == "true" || str == "false";
	}

	bool String::isNumeric(bool decimal) const
	{
		if (decimal)
		{
			std::istringstream iss(m_data);
			f64 f;
			iss >> std::noskipws >> f;
			return iss.eof() && !iss.fail();
		}
		return isInt();
	}

	bool String::isInt(void) const
	{
		String str = String(m_data).trim().toLower();
		bool isNumber = std::ranges::all_of(str.begin(), str.end(), [](char c){ return isdigit(c) != 0; });
		return str.isHex() || str.isBin() || isNumber;
	}

	bool String::isHex(void) const
	{
		String hex = String(m_data).trim().toLower();
		return hex.cpp_str().compare(0, 2, "0x") == 0 && hex.cpp_str().size() > 2 && hex.cpp_str().find_first_not_of("0123456789abcdef", 2) == std::string::npos;
	}

	bool String::isBin(void) const
	{
		String bin = String(m_data).trim().toLower();
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

	bool String::regexMatches(const String& regex_pattern, bool case_insensitive) const
	{
		try
		{
			boost::regex rgx(regex_pattern.cpp_str(), case_insensitive ? boost::regex_constants::icase : boost::regex_constants::normal);
			return boost::regex_search(m_data, rgx);
		}
		catch (const boost::regex_error& err)
		{
			std::cerr << err.what() << '\n'; //TODO: Better error handling
			return false;
		}
	}

	u32 String::count(const String& str) const
	{
		Tokens tok = tokenize(str, false, true);
		if (tok.count() < 1) return 0;
		return tok.count() - 1;
	}

	i32 String::indexOf(char c, u32 start) const
	{
		cpp_string cc = "";
		cc += c;
		i32 pos = m_data.find(cc.c_str(), start);
		if (pos == std::string::npos) return -1;
		return pos;
	}

	i32 String::indexOf(const String& str, u32 start) const
	{
		i32 pos = m_data.find(str.c_str(), start);
		if (pos == std::string::npos) return -1;
		return pos;
	}

	i32 String::lastIndexOf(char c) const
	{
		String se(m_data);
		se.reverse();
		i32 pos = se.indexOf(c);
		if (pos < 0) return -1;
		return len() - pos - 1;
	}

	i32 String::lastIndexOf(const String& str) const
	{
		String se(m_data);
		se.reverse();
		String se2(str);
		i32 pos = se.indexOf(se2.new_reverse());
		if (pos < 0) return -1;
		return len() - pos - str.len();
	}

	String::Tokens String::tokenize(const String& delimiter, bool trim_tokens, bool allow_white_space_only_tokens) const
	{
		Tokens tokens;
		i32 sindex = 0;
		i32 eindex = 0;
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
				if (allow_white_space_only_tokens && __token == "")
					tokens.m_tokens.push_back(__token.cpp_str());
				else if (__token.new_trim() != "")
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

	String String::getHexStr(u64 value, bool prefix, u8 nbytes)
	{
		union {
			u64 val;
			u8 bytes[8];
		} __tmp_editor;
		__tmp_editor.val = value;
		if (nbytes < 1 || nbytes > 8) nbytes = 1;
		std::ostringstream oss;
		if (prefix) oss << "0x";
		for (i8 b = nbytes - 1; b >= 0; b--)
			oss << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (i32)__tmp_editor.bytes[b];
		return oss.str();
	}

	String String::getBinStr(u64 value, bool prefix, u8 nbytes)
	{
		union {
			u64 val;
			u8 bytes[8];
		} __tmp_editor;
		__tmp_editor.val = value;
		if (nbytes < 1 || nbytes > 8) nbytes = 1;
		std::ostringstream oss;
		if (prefix) oss << "0b ";
		for (i8 b = nbytes - 1; b >= 0; b--)
			oss << std::bitset<8>((char)__tmp_editor.bytes[b]) << " ";
		return oss.str();
	}

	String String::duplicateChar(uchar c, u16 count)
	{
		String str = "";
		for (u16 i = 0; i < count; i++)
			str = str += c;
		return str;
	}

	stdvec<u32> String::decodeUTF8(const String& s)
	{
		stdvec<u32> out;
		const char* p = s.m_data.data();
		const char* end = p + s.m_data.size();

		while (p < end)
			out.push_back(utf8_next(p, end));

		return out;
	}


	u32 String::utf8_next(const char*& p, const char* end)
	{
		u8 c = cast<u8>(*p++);

		// ASCII fast path
		if (c < 0x80)
			return c;

		// 2-byte sequence
		if ((c >> 5) == 0x6) {
			u32 cp = (c & 0x1F) << 6;
			cp |= (cast<u8>(*p++) & 0x3F);
			return cp;
		}

		// 3-byte sequence
		if ((c >> 4) == 0xE) {
			u32 cp = (c & 0x0F) << 12;
			cp |= (cast<u8>(*p++) & 0x3F) << 6;
			cp |= (cast<u8>(*p++) & 0x3F);
			return cp;
		}

		// 4-byte sequence
		if ((c >> 3) == 0x1E) {
			u32 cp = (c & 0x07) << 18;
			cp |= (cast<u8>(*p++) & 0x3F) << 12;
			cp |= (cast<u8>(*p++) & 0x3F) << 6;
			cp |= (cast<u8>(*p++) & 0x3F);
			return cp;
		}

		// Invalid byte → return replacement character
		return 0xFFFD;
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
