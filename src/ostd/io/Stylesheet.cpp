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

#include "Stylesheet.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"

namespace ostd
{
	Stylesheet::Stylesheet(void)
	{
	}

	Stylesheet& Stylesheet::clear(void)
	{
		m_values.clear();
		return *this;
	}

	Stylesheet& Stylesheet::loadFromFile(const ostd::String& filePath, bool clearCurrentRules, VariableList variable)
	{
		ostd::String outContent = "";
		ostd::FileSystem::readTextFileRaw(filePath, outContent);
		return loadFromString(outContent, filePath, clearCurrentRules, variable);
	}

	Stylesheet& Stylesheet::loadFromString(const ostd::String& content, const ostd::String& filePath, bool clearCurrentRules, VariableList variables)
	{
		if (clearCurrentRules)
			m_values.clear();
		std::vector<ostd::String> lines = content.tokenize("\n", false, true).getRawData();
		std::vector<ostd::String> originalLines = lines;
		auto richLines = getRichStringLines(lines);
		uint32_t lineNumber = 0;
		auto l_warn = [&](const ostd::String& msg) -> void {
			OX_WARN("%s in theme line. File: <%s:%d>", msg.c_str(), filePath.c_str(), lineNumber, originalLines[lineNumber - 1].c_str());
		};
		auto l_parseLine = [&](ostd::String& line) -> void {
			if (!parseThemeFileLine(line, variables))
				l_warn("Error");
		};
		uint8_t lineNumberMaxWidth = ostd::String("").add(lines.size()).len();
		ostd::String groupSelector = "";
		bool groupLines = true;
		std::vector<ostd::String> group;
		bool debug_print = false;
		for (auto& line : lines)
		{
			lineNumber++;
			line.trim();
			if (line.startsWith("%"))
				goto custom_continue;
			if (line.contains("%"))
				line.substr(0, line.indexOf("%")).trim();
			if (line == "")
				goto custom_continue;
			if (line.startsWith("const ") || line.startsWith("$"))
			{
				bool is_const = false;
				if (line.startsWith("const "))
				{
					line.substr(6).trim();
					is_const = true;
				}
				line.substr(1);
				if (line.count("=") != 1 || line.endsWith("="))
				{
					l_warn("Invalid variable");
					goto custom_continue;
				}
				ostd::String varName = line.new_substr(0, line.indexOf("=")).trim();
				ostd::String varValue = line.new_substr(line.indexOf("=") + 1).trim();
				if (!varName.regexMatches(m_validNameRegex))
				{
					l_warn("Invalid variable name");
					goto custom_continue;
				}
				auto var = variables.find("$" + varName);
				if (var != variables.end() && var->second.second)
				{
					l_warn("Trying to re-assign a const variable");
					goto custom_continue;
				}
				variables["$" + varName] = { varValue, is_const };
				goto custom_continue;
			}
			if (groupSelector != "")
			{
				if (groupLines)
				{
					if (line == "}")
					{
						groupLines = false;
						auto newLines = parseGroup(groupSelector, group);
						lineNumber -= newLines.size();
						for (int32_t i = 0; i < newLines.size(); i++)
						{
							auto& l = newLines[i];
							l_parseLine(l);
							lineNumber++;
						}
						groupSelector = "";
						group.clear();
						goto custom_continue;
					}
					group.push_back(line);
				}
				else
				{
					if (line == "{")
						groupLines = true;
				}
				goto custom_continue;
			}
			if (line.startsWith("("))
			{
				if (line.count(")") != 1 || line.indexOf(")") < 3)
				{
					l_warn("Invalid group selector");
					goto custom_continue;
				}
				ostd::String rawSelector = line.new_substr(1, line.indexOf(")")).trim();
				groupSelector = parseGroupSelector(rawSelector);
				if (groupSelector == "")
				{
					l_warn("Invalid group selector");
					goto custom_continue;
				}
				if (line.contains("{"))
				{
					line.substr(line.indexOf("{")).trim();
					if (line != "{")
					{
						l_warn("Invalid group selector");
						goto custom_continue;
					}
					groupLines = true;
				}
				goto custom_continue;
			}
			l_parseLine(line);
			continue;
custom_continue:
			if (debug_print)
				std::cout << ostd::String("").add(lineNumber).addLeftPadding(lineNumberMaxWidth, ' ') << "|  " << richLines[lineNumber - 1] << "\n";
		}
		return *this;
	}

	void Stylesheet::set(const std::string& key, TypeVariant value, const ostd::String& themeID)
	{
		ostd::String fullKey = "@" + themeID + "." + key;
		// std::cout << "SET: " << fullKey << "\n";
		setFull(fullKey, value);
	}

	void Stylesheet::removeRule(const ostd::String& fullKey)
	{
		m_values.erase(fullKey);
	}

	void Stylesheet::setFull(const ostd::String& fullKey, TypeVariant value)
	{
		m_values[fullKey] = std::move(value);
	}

	const Stylesheet::TypeVariant* Stylesheet::getVariant(const ostd::String& key, const std::vector<ostd::String>& themeIDList, const QualifierList& qualifierList) const
	{
		std::vector<ostd::String> emptyThemeIDList { "" };
		const std::vector<ostd::String>& __themeIDList = (themeIDList.size() == 0 ? emptyThemeIDList : themeIDList);
		for (const auto&[qualifier, state] : qualifierList)
		{
			const TypeVariant* v = nullptr;
			for (int32_t i = __themeIDList.size() - 1; i >= 0; i--)
			{
				const ostd::String& themeID = __themeIDList[i];
				v = (state ? getFull("@" + themeID + ":" + qualifier + "." + key) : nullptr);
				if (v)
					return v;
			}
		}
		for (const auto&[qualifier, state] : qualifierList)
		{
			const TypeVariant* v = (state ? getFull("@:" + qualifier + "." + key) : nullptr);
			if (v)
				return v;
		}
		for (int32_t i = __themeIDList.size() - 1; i >= 0; i--)
		{
			const ostd::String& themeID = __themeIDList[i];
			if (auto v = getFull("@" + themeID + "." + key))
				return v;
		}
		if (auto v = getFull("@." + key))
			return v;
		return nullptr;
	}

	const Stylesheet::TypeVariant* Stylesheet::getFull(const ostd::String& fullKey) const
	{
		auto it = m_values.find(fullKey);
		// if (it != m_values.end())
		// 	std::cout << "    GET: " << fullKey << "\n";
		return it != m_values.end() ? &it->second : nullptr;
	}

	bool Stylesheet::parseThemeFileLine(const ostd::String& line, const VariableList& variables, bool exitCondition)
	{
		if (!line.contains("="))
			return false;
		ostd::String key = line.new_substr(0, line.indexOf("=")).trim();
		ostd::String valuePreserveCase = line.new_substr(line.indexOf("=") + 1).trim();
		ostd::String value = line.new_substr(line.indexOf("=") + 1).trim().toLower();
		if (key == "")
			return false;
		ostd::String themeID = "";
		if (key.startsWith("@"))
		{
			if (key.indexOf(" ") < 2)
				return false;
			themeID = key.new_substr(1, key.indexOf(" ")).trim();
			key.substr(key.indexOf(" ") + 1).trim();
		}
		if (value.isInt())
			set(key, static_cast<int32_t>(value.toInt()), themeID);
		else if (value.isNumeric(true))
			set(key, value.toFloat(), themeID);
		else if (value == "true" || value == "false")
			set(key, value == "true", themeID);
		else if (value.startsWith("\"") && value.endsWith("\""))
		{
			valuePreserveCase.substr(1, value.len() - 1);
			set(key, valuePreserveCase, themeID);
		}
		else if (value.startsWith("color(") && value.endsWith(")"))
		{
			value.substr(6, value.len() - 1).trim();
			set(key, ostd::Color(value), themeID);
		}
		else if (value.startsWith("vec2(") && value.endsWith(")"))
		{
			value.substr(4, value.len() - 1).trim();
			auto tokens = value.tokenize(",");
			if (tokens.count() != 2)
				return false;
			std::vector<float> vec;
			for (const auto& tok : tokens)
			{
				if (!tok.isNumeric(true))
					return false;
				vec.push_back(tok.toFloat());
			}
			set(key, ostd::Vec2(vec[0], vec[1]), themeID);
		}
		else if (value.startsWith("rect(") && value.endsWith(")"))
		{
			value.substr(5, value.len() - 1).trim();
			auto tokens = value.tokenize(",");
			if (tokens.count() != 4)
				return false;
			std::vector<float> vec;
			for (const auto& tok : tokens)
			{
				if (!tok.isNumeric(true))
					return false;
				vec.push_back(tok.toFloat());
			}
			set(key, ostd::Rectangle(vec[0], vec[1], vec[2], vec[3]), themeID);
		}
		else
		{
			if (exitCondition)
				return false;
			ostd::String lineCopy = line;
			for (const auto&[var, val] : variables)
			{
				if (lineCopy.contains(var))
				{
					lineCopy.replaceAll(var, val.first);
					break;
				}
			}
			return parseThemeFileLine(lineCopy, variables, true);
		}
		return true;
	}

	ostd::String Stylesheet::parseGroupSelector(const ostd::String& rawSelector) const
	{
		ostd::String sel = rawSelector.new_trim();
		if (sel == "") return "";
		ostd::String id = "";
		ostd::String name = sel;
		ostd::String qual = "";
		if (sel.contains(" "))
		{
			id = sel.new_substr(0, sel.indexOf(" ")).trim();
			name = sel.new_substr(sel.indexOf(" ") + 1).trim();
		}
		if (name.contains(":"))
		{
			if (name.startsWith(":") || name.endsWith(":") || name.count(":") != 1)
				return "";
			qual = name.new_substr(name.indexOf(":") + 1).trim();
			name.substr(0, name.indexOf(":")).trim();
		}
		if (qual != "")
		{
			if (!id.startsWith("@"))
				id = "@" + id;
			id += ":" + qual;
		}
		id += " " + name;
		id.trim();
		return id;
	}

	std::vector<ostd::String> Stylesheet::parseGroup(const ostd::String& selector, const std::vector<ostd::String>& group)
	{
		if (selector.new_trim() == "" || group.size() == 0)
			return {};
		std::vector<ostd::String> newLines;
		for (const auto& property : group)
			newLines.push_back(selector.new_add(".").new_add(property));
		return newLines;
	}

	void Stylesheet::debugPrint(void)
	{
		for (const auto&[key, value] : m_values)
		{
			std::cout << key << "  =  " << typeVariantToString(value) << "\n";
		}
		std::cout << "\n";
	}

	ostd::String Stylesheet::typeVariantToString(const TypeVariant& v)
	{
		if (auto p = std::get_if<int32_t>(&v))
			return String("").add(*p);
		else if (auto p = std::get_if<float>(&v))
			return String("").add(*p);
		else if (auto p = std::get_if<bool>(&v))
			return STR_BOOL(*p);
		else if (auto p = std::get_if<ostd::String>(&v))
			return *p;
		else if (auto p = std::get_if<ostd::Color>(&v))
			return *p;
		else if (auto p = std::get_if<ostd::Rectangle>(&v))
			return *p;
		else if (auto p = std::get_if<ostd::Vec2>(&v))
			return *p;
		return "";
	}

	std::vector<ostd::RegexRichString> Stylesheet::getRichStringLines(const std::vector<ostd::String>& lines)
	{
		std::vector<ostd::RegexRichString> richLines;
		for (auto line : lines)
		{
			ostd::RegexRichString rgxrstr(line);
			rgxrstr.fg("\\{|\\}|\\+|\\*|\\-|\\/|\\(|\\)|\\[|\\]", "Red"); //Operators
			rgxrstr.fg("0x[0-9A-Fa-f]+|0b[0-1]+|(?<!\\w)[0-9]+(?!\\w)", "Blue"); //Number Constants
			rgxrstr.fg("(?<!\\w)(r[1-9]|r10|fl|pp|rv|fp|sp|ip|acc)(?!\\w)", "BrightGreen", true); //Registers
			rgxrstr.fg("\\%low", "Magenta");
			rgxrstr.fg("\\$\\w+", "Cyan"); //Labels
			richLines.push_back(rgxrstr);
		}
		return richLines;
	}
}
