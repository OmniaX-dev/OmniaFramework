#include "Utils.hpp"
#include <sstream>
#include <algorithm>
#include <boost/regex.hpp>

namespace ostd
{
	String StringEditor::Tokens::next(void)
	{
		if (!hasNext()) return Tokens::END;
		return m_tokens[m_current_index++];
	}

	String StringEditor::Tokens::previous(void)
	{
		if (!hasPrevious()) return Tokens::END;
		return m_tokens[--m_current_index];
	}

	StringEditor& StringEditor::ltrim(void)
	{
		m_data.erase(m_data.begin(), std::find_if(m_data.begin(), m_data.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
		return *this;
	}
	StringEditor& StringEditor::rtrim(void)
	{
		m_data.erase(std::find_if(m_data.rbegin(), m_data.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), m_data.end());
		return *this;
	}
	StringEditor& StringEditor::trim(void)
	{
		return ltrim().rtrim();
	}
	StringEditor& StringEditor::toLower(void)
	{
		std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](unsigned char c){ return std::tolower(c); });
		return *this;
	}
	StringEditor& StringEditor::toUpper(void)
	{
		std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](unsigned char c){ return std::toupper(c); });
		return *this;
	}
	StringEditor& StringEditor::addPadding(char c, uint32_t count)
	{
		for (uint16_t i = 0; i < count; i++)
			m_data += c;
		return *this;
	}

	// StringEditor& StringEditor::add(const StringEditor& se)
	// {
	// 	m_data += se.m_data;
	// 	return *this;
	// }

	StringEditor& StringEditor::add(String str)
	{
		m_data += str;
		return *this;
	}

	StringEditor& StringEditor::add(char c)
	{
		m_data += c;
		return *this;
	}

	StringEditor& StringEditor::addi(uint8_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addi(int8_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addi(uint16_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addi(int16_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addi(uint32_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addi(int32_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addi(uint64_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addi(int64_t i)
	{
		m_data += std::to_string(i);
		return *this;
	}

	StringEditor& StringEditor::addf(float f)
	{
		m_data += std::to_string(f);
		return *this;
	}

	StringEditor& StringEditor::addf(double f)
	{
		m_data += std::to_string(f);
		return *this;
	}

	StringEditor& StringEditor::reverse(void)
	{
		std::reverse(m_data.begin(), m_data.end());
		return *this;
	}

	StringEditor& StringEditor::replaceAll(String what, String with)
	{
		while (contains(what))
			replaceFirst(what, with);
		return *this;
	}

	StringEditor& StringEditor::replaceFirst(String what, String with)
	{
		int32_t index = indexOf(what);
		if (index == -1) return *this;
		m_data.replace(index, what.length(), with);
		return *this;
	}

	StringEditor& StringEditor::regexReplace(String regex_pattern, String replace_with, bool case_insensitive)
	{
		try
		{
			boost::regex rgx(regex_pattern, (case_insensitive ? boost::regex_constants::icase : boost::regex_constants::normal));
			m_data = boost::regex_replace(m_data, rgx, replace_with);
			return *this;
		}
		catch(const boost::regex_error& err)
		{
			std::cerr << err.what() << '\n'; //TODO: Better error handling
			return *this;
		}
		return *this;
	}

	StringEditor& StringEditor::put(uint32_t index, char c)
	{
		if (index < m_data.length())
			m_data[index] = c;
		return *this;
	}

	int64_t StringEditor::toInt(void)
	{
		if (!isNumeric(false)) return 0;
		return Utils::strToInt(m_data);
	}

	float StringEditor::toFloat(void)
	{
		if (!isNumeric(true)) return 0;
		return std::stof(m_data);
	}

	double StringEditor::toDouble(void)
	{
		if (!isNumeric(true)) return 0;
		return std::stod(m_data);
	}

	bool StringEditor::isNumeric(bool decimal)
	{
		if (decimal)
		{
			std::istringstream iss(m_data);
			double f;
			iss >> std::noskipws >> f;
			return iss.eof() && !iss.fail(); 
		}
		return Utils::isInt(m_data);
	}

	bool StringEditor::contains(char c)
	{
		return m_data.find(c) != std::string::npos;
	}

	bool StringEditor::contains(String str)
	{
		return m_data.find(str) != std::string::npos;
	}

	bool StringEditor::startsWith(String str)
	{
		return m_data.starts_with(str);
	}

	bool StringEditor::endsWith(String str)
	{
		return m_data.ends_with(str);
	}

	String StringEditor::getReverse(void)
	{
		StringEditor se(m_data);
		return se.reverse().str();
	}

	uint32_t StringEditor::count(String str)
	{
		Tokens tok = tokenize(str, false, true);
		if (tok.count() < 1) return 0;
		return tok.count() - 1;
	}

	int32_t StringEditor::indexOf(char c, uint32_t start)
	{
		String cc = "";
		cc += c;
		int32_t pos = m_data.find(cc.c_str(), start);
		if (pos == std::string::npos) return -1;
		return pos;
	}

	int32_t StringEditor::indexOf(String str, uint32_t start)
	{
		int32_t pos = m_data.find(str.c_str(), start);
		if (pos == std::string::npos) return -1;
		return pos;
	}

	int32_t StringEditor::lastIndexOf(char c)
	{
		StringEditor se(m_data);
		se.reverse();
		int32_t pos = se.indexOf(c);
		if (pos < 0) return -1;
		return len() - pos - 1;
	}

	int32_t StringEditor::lastIndexOf(String str)
	{
		StringEditor se(m_data);
		se.reverse();
		StringEditor se2(str);
		int32_t pos = se.indexOf(se2.reverse().str());
		if (pos < 0) return -1;
		return len() - pos - str.length();
	}

	String StringEditor::substr(uint32_t start, int32_t end)
	{
		if (end < 0) return m_data.substr(start);
		return m_data.substr(start, end - start);
	}

	StringEditor::Tokens StringEditor::tokenize(String delimiter, bool trim_tokens, bool allow_white_space_only_tokens)
	{
		Tokens tokens;
		int32_t sindex = 0;
		int32_t eindex = 0;
		StringEditor __token = "";
		while ((eindex = indexOf(delimiter, sindex)) != -1)
		{
			__token = substr(sindex, eindex);
			if (trim_tokens)
			{
				__token.trim();
				if (__token.str() != "")
					tokens.m_tokens.push_back(__token.str());
			}
			else
			{
				if (allow_white_space_only_tokens && __token.str() != "")
					tokens.m_tokens.push_back(__token.str());
				else if (!allow_white_space_only_tokens && StringEditor(__token).trim().str() != "")
					tokens.m_tokens.push_back(__token.str());
			}
			sindex = eindex + delimiter.length();
		}
		__token = substr(sindex);
		if (trim_tokens)
		{
			__token.trim();
			if (__token.str() != "")
				tokens.m_tokens.push_back(__token.str());
		}
		else
		{
			if (allow_white_space_only_tokens && __token.str() != "")
				tokens.m_tokens.push_back(__token.str());
			else if (!allow_white_space_only_tokens && StringEditor(__token).trim().str() != "")
				tokens.m_tokens.push_back(__token.str());
		}
		return tokens;
	}

	std::ostream& operator<<(std::ostream& out, const StringEditor& val)
	{
		out << val.str();
		return out;
	}

} // namespace ox
