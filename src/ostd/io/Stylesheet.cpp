/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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
#include <algorithm>
#include <optional>

namespace ostd
{
	Stylesheet::Stylesheet(void)
	{
		//
	}

	Stylesheet& Stylesheet::clear(void)
	{
		m_values.clear();
		return *this;
	}

	Stylesheet& Stylesheet::loadFromFile(const String& filePath, bool clearCurrentRules, VariableList variable)
	{
		String outContent = "";
		ostd::FileSystem::readTextFileRaw(filePath, outContent);
		return loadFromString(outContent, filePath, clearCurrentRules, variable);
	}

	Stylesheet& Stylesheet::loadFromString(const String& content, const String& filePath, bool clearCurrentRules, VariableList variables)
	{
		if (clearCurrentRules)
			m_values.clear();
		stdvec<String> lines = content.tokenize("\n", false, true).getRawData();
		stdvec<String> originalLines = lines;
		u32 lineNumber = 0;
		auto l_warn = [&](const String& msg) -> void {
			OX_WARN("%s in theme line. File: <%s:%d>\n  %s", msg.c_str(), filePath.c_str(), lineNumber, originalLines[lineNumber - 1].c_str());
		};
		auto l_parseLine = [&](String& line) -> void {
			if (!parseThemeFileLine(line, variables))
				l_warn("Error");
		};
		u8 lineNumberMaxWidth = String("").add(cast<u32>(lines.size())).len();
		String groupSelector = "";
		bool groupLines = true;
		stdvec<String> group;
		u32 groupSelectorCount = 0;
		bool debug_print = false;
		bool brokenLine = false;
		String brokenLineBuffer =  "";
		String macroCode = "";
		bool inMacro = false;
		for (auto& line : lines)
		{
			lineNumber++;
			line.trim();
			if (line.startsWith("%"))
				continue;
			if (line.contains("%"))
				line.substr(0, line.indexOf("%")).trim();
			if (line == "")
				continue;
			if (brokenLine)
			{
				line = brokenLineBuffer + line;
				brokenLine = false;
				brokenLineBuffer = "";
			}
			if  (line.endsWith("\\"))
			{
				brokenLine = true;
				brokenLineBuffer = line.new_trim().substr(0, line.len() - 1).trim();
				continue;
			}
			if (inMacro)
			{
				for (const auto& c : line)
				{
					macroCode += c;
					if (c == '}')
					{
						inMacro = false;
						macroCode.trim();
						if (!parseMacro(macroCode))
							l_warn("Invalid macro");
						macroCode = "";
						break;
					}
				}
				continue;
			}
			if (line.startsWith("macro "))
			{
				line.substr(6).trim();
				macroCode += line;
				inMacro = true;
				continue;
			}
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
					continue;
				}
				String varName = line.new_substr(0, line.indexOf("=")).trim();
				String varValue = line.new_substr(line.indexOf("=") + 1).trim();
				if (!varName.regexMatches(m_validNameRegex))
				{
					l_warn("Invalid variable name");
					continue;
				}
				auto var = variables.find("$" + varName);
				if (var != variables.end() && var->second.second)
				{
					l_warn("Trying to re-assign a const variable");
					continue;
				}
				variables["$" + varName] = { varValue, is_const };
				continue;
			}
			if (groupSelector != "")
			{
				if (groupLines)
				{
					if (line.endsWith("{"))
					{
						groupSelectorCount++;
					}
					else if (line == "}" && groupSelectorCount > 1)
					{
						groupSelectorCount--;
					}
					else if (line == "}" && groupSelectorCount == 1)
					{
						groupLines = false;
						auto newLines = parseGroup(groupSelector, group);
						lineNumber -= newLines.size();
						for (i32 i = 0; i < newLines.size(); i++)
						{
							auto& l = newLines[i];
							l_parseLine(l);
							lineNumber++;
						}
						groupSelector = "";
						groupSelectorCount = 0;
						group.clear();
						continue;
					}
					group.push_back(line);
				}
				else
				{
					if (line == "{")
					{
						groupLines = true;
						groupSelectorCount++;
					}
				}
				continue;
			}
			if (line.startsWith("("))
			{
				if (line.count(")") != 1 || line.indexOf(")") < 3)
				{
					l_warn("Invalid group selector");
					continue;
				}
				String rawSelector = line.new_substr(1, line.indexOf(")")).trim();
				groupSelector = parseGroupSelector(rawSelector);
				if (groupSelector == "")
				{
					l_warn("Invalid group selector");
					continue;
				}
				if (line.contains("{"))
				{
					line.substr(line.indexOf("{")).trim();
					if (line != "{")
					{
						l_warn("Invalid group selector");
						continue;
					}
					groupLines = true;
					groupSelectorCount = 1;
				}
				continue;
			}
			l_parseLine(line);
			continue;
		}
		return *this;
	}

	void Stylesheet::set(const std::string& key, TypeVariant value, const String& themeID)
	{
		String fullKey = "@" + themeID + "." + key;
		// std::cout << "SET: " << fullKey << "\n";
		setFull(fullKey, value);
	}

	void Stylesheet::removeRule(const String& fullKey)
	{
		m_values.erase(fullKey);
	}

	void Stylesheet::setFull(const String& fullKey, TypeVariant value)
	{
		m_values[fullKey] = std::move(value);
	}

	const Stylesheet::TypeVariant* Stylesheet::getVariant(const String& key, const stdvec<String>& themeIDList, const QualifierList& qualifierList) const
	{
		stdvec<String> emptyThemeIDList { "" };
		const stdvec<String>& __themeIDList = (themeIDList.size() == 0 ? emptyThemeIDList : themeIDList);
		for (const auto&[qualifier, state] : qualifierList)
		{
			const TypeVariant* v = nullptr;
			for (i32 i = __themeIDList.size() - 1; i >= 0; i--)
			{
				const String& themeID = __themeIDList[i];
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
		for (i32 i = __themeIDList.size() - 1; i >= 0; i--)
		{
			const String& themeID = __themeIDList[i];
			if (auto v = getFull("@" + themeID + "." + key))
				return v;
		}
		if (auto v = getFull("@." + key))
			return v;
		return nullptr;
	}

	const Stylesheet::TypeVariant* Stylesheet::getFull(const String& fullKey) const
	{
		auto it = m_values.find(fullKey);
		// if (it != m_values.end())
		//     std::cout << "    GET: " << fullKey << "\n";
		return it != m_values.end() ? &it->second : nullptr;
	}

	bool Stylesheet::parseThemeFileLine(const String& line, const VariableList& variables, bool exitCondition)
	{
		auto l_isVec2 = [this](String& value) -> bool {
			value.trim();
			if (value.new_toLower().startsWith("vec2(") && value.new_toLower().endsWith(")"))
			{
				value.substr(5, value.len() - 1).trim();
				return true;
			}
			return false;
		};
		auto l_isRect = [this](String& value) -> bool {
			value.trim();
			if (value.new_toLower().startsWith("rect(") && value.new_toLower().endsWith(")"))
			{
				value.substr(5, value.len() - 1).trim();
				return true;
			}
			return false;
		};
		auto l_isColor = [this](String& value) -> bool {
			value.trim();
			if (value.new_toLower().startsWith("color(") && value.new_toLower().endsWith(")"))
			{
				value.substr(6, value.len() - 1).trim();
				return true;
			}
			else if ((value.new_toLower().startsWith("#")) ||
					 (value.new_toLower().startsWith("rgb(")) ||
					 (value.new_toLower().startsWith("rgba(")))
			{
				return true;
			}
			return false;
		};
		auto l_isColorGradientValue = [this](const String& _value) -> bool {
			String value = _value.new_toLower().trim();
			if (value.startsWith("gradienth(") && value.endsWith(")"))
				return true;
			if (value.startsWith("w_gradienth(") && value.endsWith(")"))
				return true;
			if (value.startsWith("gradientv(") && value.endsWith(")"))
				return true;
			if (value.startsWith("w_gradientv(") && value.endsWith(")"))
				return true;
			return false;
		};
		auto l_isFile = [this](String& value) -> bool {
			value.trim();
			if (value.new_toLower().startsWith("file(") && value.endsWith(")"))
			{
				value.substr(5, value.len() - 1).trim();
				if (value.startsWith("\"") && value.endsWith("\""))
				{
					value.substr(1, value.len() - 1);
					return true;
				}
			}
			return false;
		};
		auto l_isAnim = [this](String& value) -> bool {
			value.trim();
			if (value.new_toLower().startsWith("anim(") && value.endsWith(")"))
			{
				value.substr(5, value.len() - 1).trim();
				return true;
			}
			return false;
		};
		auto l_findMacroCall = [this](String& value, String& outKey) -> std::optional<Macro> {
			for (const auto& m : m_macros)
			{
				stdvec<String> matches;
				stdvec<u32> indices = value.regexFind("^(.+\\.)?" + m.first + "\\b[ \\t]*\\(", false, &matches);

				if (!indices.empty())
				{
					u32 paren_index = indices[0] + matches[0].len() - 1;
					String from_paren = value.new_substr(paren_index);
					String match_text = value.new_substr(indices[0], paren_index);
					i32 dot_index = match_text.lastIndexOf('.');
					if (dot_index >= 0)
						outKey = value.new_substr(0, dot_index);
					if (from_paren.startsWith("(") && from_paren.endsWith(")"))
					{
						from_paren.substr(1, from_paren.len() - 1).trim();
						value = from_paren;
						return m.second;
					}
					return std::nullopt;
				}
			}
			return std::nullopt;
		};

		{
			String tmpLine = line.new_trim(), key = "";
			if (auto m = l_findMacroCall(tmpLine, key); m != std::nullopt)
			{
				auto& macro = m.value();
				auto expanded = parseMacroCall(tmpLine, macro);
				for (const auto& l : expanded)
				{
					if (!parseThemeFileLine(key + "." + l, variables, false))
						return false;
				}
				return true;
			}
		}

		if (!line.contains("="))
			return false;
		String key = line.new_substr(0, line.indexOf("=")).trim();
		String valuePreserveCase = line.new_substr(line.indexOf("=") + 1).trim();
		String value = valuePreserveCase.new_toLower();
		if (key == "")
			return false;
		String themeID = "";

		if (key.startsWith("@"))
		{
			if (key.indexOf(" ") < 2)
				return false;
			themeID = key.new_substr(1, key.indexOf(" ")).trim();
			key.substr(key.indexOf(" ") + 1).trim();
		}
		if (value.isNumeric(true))
			set(key, value.toFloat(), themeID);
		else if (value.isInt())
			set(key, cast<i32>(value.toInt()), themeID);
		else if (value == "true" || value == "false")
			set(key, value == "true", themeID);
		else if (value.startsWith("\"") && value.endsWith("\""))
		{
			valuePreserveCase.substr(1, value.len() - 1);
			set(key, valuePreserveCase, themeID);
		}
		else if (l_isColor(value))
		{
			set(key, parseColor(value, variables), themeID);
		}
		else if (l_isColorGradientValue(value))
		{
			ColorGradient grad = parseColorGradient(valuePreserveCase, variables);
			if (grad.isInvalid())
				return false;
			set(key, grad, themeID);
		}
		else if (l_isAnim(valuePreserveCase))
		{
			bool outError = false;
			AnimationData ad = parseAnim(valuePreserveCase, outError, variables);
			if (outError)
				return false;
			set(key, ad, themeID);
		}
		else if (l_isFile(valuePreserveCase))
		{
			bool exists = ostd::FileSystem::fileExists(valuePreserveCase) ||
						  ostd::FileSystem::fileExists("./" + valuePreserveCase);
			if (exists)
				set(key, valuePreserveCase, themeID);
			else
				return false;
		}
		else if (l_isVec2(value))
		{
			set(key, parseVec2(value, variables), themeID);
		}
		else if (l_isRect(value))
		{
			set(key, parseRect(value, variables), themeID);
		}
		else if (exitCondition)
		{
			//Defaulting anything that isn't recognized as any value_type/variable to String type
			set(key, valuePreserveCase, themeID);
		}
		else
		{
			if (exitCondition)
				return false;
			return parseThemeFileLine(replaceVariables(line, variables), variables, true);
		}
		return true;
	}

	String Stylesheet::parseGroupSelector(const String& rawSelector) const
	{
		String sel = rawSelector.new_trim();
		if (sel == "") return "";
		String id = "";
		String name = sel;
		String qual = "";
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

	stdvec<String> Stylesheet::parseGroup(const String& selector, const stdvec<String>& group)
	{
		if (selector.new_trim() == "" || group.size() == 0)
			return {};
		stdvec<String> newLines;
		stdvec<String> subSelectorStack;
		for (const auto& property : group)
		{
			String p = property.new_trim();
			if (p.startsWith("(") && p.lastIndexOf(")") > 1 && p.endsWith("{") && p.len() > 3)
			{
				String ss = p.substr(1, p.lastIndexOf(")")).trim();
				if (!ss.regexMatches(m_validNameRegex))
					return {};
				subSelectorStack.push_back(ss);
			}
			else if (p == "}")
			{
				if (subSelectorStack.size() > 0)
					subSelectorStack.pop_back();
				else
					return {};
			}
			else
			{
				String fullSelector = selector;
				for (const auto& ss : subSelectorStack)
					fullSelector.add(".").add(ss);
				newLines.push_back(fullSelector.new_add(".").new_add(property));
			}
		}
		// for (const auto& l : newLines)
		//     std::cout << l << "\n";
		// std::cout << "\n\n\n";
		return newLines;
	}

	Color Stylesheet::parseColor(const String& _value, const VariableList& variables)
	{
		String value = replaceVariables(_value, variables);
		return Color(value);
	}

	Vec2 Stylesheet::parseVec2(const String& _value, const VariableList& variables)
	{
		String value = replaceVariables(_value, variables);
		auto tokens = value.tokenize(",");
		if (tokens.count() != 2)
			return { 0, 0 };
		stdvec<f32> vec;
		for (const auto& tok : tokens)
		{
			if (!tok.isNumeric(true))
				return { 0, 0 };
			vec.push_back(tok.toFloat());
		}
		return { vec[0], vec[1] };
	}

	Rectangle Stylesheet::parseRect(const String& _value, const VariableList& variables)
	{
		String value = replaceVariables(_value, variables);
		auto tokens = value.tokenize(",");
		if (tokens.count() != 4)
			return { 0, 0, 0, 0 };
		stdvec<f32> vec;
		for (const auto& tok : tokens)
		{
			if (!tok.isNumeric(true))
				return { 0, 0, 0, 0 };
			vec.push_back(tok.toFloat());
		}
		return { vec[0], vec[1], vec[2], vec[3] };
	}

	ColorGradient Stylesheet::parseColorGradient(const String& _value, const VariableList& variables)
	{
		String value = _value.new_trim();
		ColorGradient grad;
		f32 angle = 0.0f;
		bool weighted = false;
		String gradientFunc = value.new_substr(0, value.indexOf("(")).trim().toLower();
		String gradientVal = value.substr(value.indexOf("(") + 1, value.lastIndexOf(")")).trim();

		if (gradientVal == "" || !gradientVal.contains("-"))
			return grad;

		weighted = gradientFunc.startsWith("w_");
		if (gradientFunc.endsWith("v"))
			angle = ColorGradient::VerticalDeg;
		else if (gradientFunc.endsWith("h"))
			angle = ColorGradient::HorizontalDeg;
		else
			return grad;

		auto tokens = gradientVal.tokenize("-");

		if (weighted && tokens.count() < 3)
			return grad;

		if (tokens.count() < 2)
			return grad;

		grad.setAngleDeg(angle);
		if (!weighted)
		{
			i32 index = 0;
			for (auto& token : tokens)
			{
				token = replaceVariables(token, variables);
				if (token.new_toLower().startsWith("color(") && token.new_toLower().endsWith(")"))
					token.substr(6, token.len() - 1).trim();
				grad.addColor(Color(token));
				if (index > 0)
					grad.addWeight(1.0f);
				index++;
			}
			return grad;
		}
		else
		{
			bool is_color = true;
			for (auto& token : tokens)
			{
				if (is_color)
				{
					token = replaceVariables(token, variables);
					if (token.new_toLower().startsWith("color(") && token.new_toLower().endsWith(")"))
						token.substr(6, token.len() - 1).trim();
					grad.addColor(Color(token));
				}
				else
				{
					token = replaceVariables(token, variables);
					if (!token.isNumeric(true))
						return ColorGradient();
					grad.addWeight(token.toFloat());
				}
				is_color = !is_color;
			}
			return grad;
		}

		return grad;
	}

	AnimationData Stylesheet::parseAnim(const String& _value, bool& outError, const VariableList& variables)
	{
		String value = _value.new_trim();
		AnimationData ad;
		auto tokens = value.tokenize(",");
		auto l_error = [&](bool err) -> AnimationData {
			outError = err;
			return ad;
		};
		for (auto& tok : tokens)
		{
			if (tok.count(":") != 1 || tok.startsWith(":") || tok.endsWith(":"))
				return l_error(true);
			String prop = tok.new_substr(0, tok.indexOf(":")).trim().toLower();
			String val = tok.new_substr(tok.indexOf(":") + 1).trim();
			val = replaceVariables(val, variables);
			if (prop == "framecount")
			{
				if (!val.isInt())
					return l_error(true);
				ad.frameCount = val.toInt();
			}
			else if (prop == "stillframe")
			{
				if (!val.isInt())
					return l_error(true);
				ad.stillFrame = val.toInt();
			}
			else if (prop == "fps")
			{
				if (!val.isNumeric(true))
					return l_error(true);
				ad.fps = val.toFloat();
			}
			else if (prop == "rowoffset")
			{
				if (!val.isInt())
					return l_error(true);
				ad.rowOffset = val.toInt();
			}
			else if (prop == "columnoffset")
			{
				if (!val.isInt())
					return l_error(true);
				ad.columnOffset = val.toInt();
			}
			else if (prop == "pixeloffsetx")
			{
				if (!val.isNumeric(true))
					return l_error(true);
				ad.pixelOffsetX = val.toFloat();
			}
			else if (prop == "pixeloffsety")
			{
				if (!val.isNumeric(true))
					return l_error(true);
				ad.pixelOffsetY = val.toFloat();
			}
			else if (prop == "spacingx")
			{
				if (!val.isNumeric(true))
					return l_error(true);
				ad.spacingX = val.toFloat();
			}
			else if (prop == "spacingy")
			{
				if (!val.isNumeric(true))
					return l_error(true);
				ad.spacingY = val.toFloat();
			}
			else if (prop == "rows")
			{
				if (!val.isInt())
					return l_error(true);
				ad.rows = val.toInt();
			}
			else if (prop == "columns")
			{
				if (!val.isInt())
					return l_error(true);
				ad.columns = val.toInt();
			}
			else if (prop == "framewidth")
			{
				if (!val.isNumeric(true))
					return l_error(true);
				ad.frameWidth = val.toFloat();
			}
			else if (prop == "frameheight")
			{
				if (!val.isNumeric(true))
					return l_error(true);
				ad.frameHeight = val.toFloat();
			}
			else if (prop == "still")
			{
				if (!val.isBool())
					return l_error(true);
				ad.still = val.toBool();
			}
			else if (prop == "turnback")
			{
				if (!val.isBool())
					return l_error(true);
				ad.turnBack = val.toBool();
			}
			else if (prop == "random")
			{
				if (!val.isBool())
					return l_error(true);
				ad.random = val.toBool();
			}
			else
			{
				return l_error(true);
			}
		}
		return l_error(false);
	}

	String Stylesheet::replaceVariables(const String& line, const VariableList& variables, bool stop_at_first_match)
	{
		String lineCopy = line;
		std::vector<std::pair<String, std::pair<String, bool>>> sortedVars(variables.begin(), variables.end());
		std::sort(sortedVars.begin(), sortedVars.end(), [](const auto& a, const auto& b) {
			return a.first.len() > b.first.len();
		});
		for (const auto&[var, val] : sortedVars)
		{
			if (lineCopy.contains(var))
			{
				lineCopy.replaceAll(var, val.first);
				if (stop_at_first_match)
					break;
			}
		}
		return lineCopy.trim();
	}

	bool Stylesheet::parseMacro(const String& macroCode)
	{
		auto l_isWhiteSpace = [](char c) { return c == ' ' || c == '\n' || c == '\t'; };

		String name = "";
		String params = "";
		String body = "";

		i32 index = 0;
		bool valid = false;
		for ( ; index < macroCode.len(); index++)
		{
			char c = macroCode[index];
			if (l_isWhiteSpace(c))
				continue;
			if (c == '(')
			{
				index++;
				if (index == macroCode.len())
					return false;
				valid = true;
				break;
			}
			name += c;
		}
		if (!valid) return false;
		if (!name.regexMatches(m_validNameRegex))
			return false;

		valid = false;
		bool inStr = false;
		for ( ; index < macroCode.len(); index++)
		{
			char c = macroCode[index];
			if (inStr)
			{
				params += c;
				inStr = inStr && c != '"';
				continue;
			}
			if (l_isWhiteSpace(c))
				continue;
			if (c == '"')
			{
				params += c;
				inStr = true;
				continue;
			}
			if (c == ',')
			{
				params += '\n';
				continue;
			}
			if (c == ')')
			{
				index++;
				if (index == macroCode.len())
					return false;
				valid = true;
				break;
			}
			params += c;
		}
		if (!valid) return false;

		valid = false;
		inStr = false;
		bool open = false;
		for ( ; index < macroCode.len(); index++)
		{
			char c = macroCode[index];
			if (!open)
			{
				if (c == '{')
					open = true;
				continue;
			}
			if (inStr)
			{
				body += c;
				inStr = inStr && c != '"';
				continue;
			}
			if (l_isWhiteSpace(c))
				continue;
			if (c == '"')
			{
				body += c;
				inStr = true;
				continue;
			}
			if (c == ';')
			{
				body += '\n';
				continue;
			}
			if (c == '}')
			{
				index++;
				if (index != macroCode.len())
					return false;
				valid = true;
				break;
			}
			body += c;
		}
		if (!valid) return false;

		Macro macro;
		macro.body = body;

		auto l_paramExists = [](const String& param, const stdvec<std::pair<String, String>>& list) -> bool {
			for (const auto& p : list)
			{
				if (p.first == param)
					return true;
			}
			return false;
		};

		auto tokens = params.tokenize("\n");
		bool defaultVal = false;
		for (auto& tok : tokens)
		{
			if (tok.contains("="))
			{
				if (tok.count("=") != 1)
					return false;
				i32 i = tok.indexOf("=");
				if (i < 1 || i > tok.len() - 2)
					return false;
				String pname = tok.new_substr(0, i).trim();
				String pval = tok.new_substr(i + 1).trim();
				if (pname == "" || pval == "")
					return false;
				if (!pname.regexMatches(m_validNameRegex))
					return false;
				if (l_paramExists("$" + pname, macro.params))
					return false;
				macro.params.push_back({ "$" + pname, pval });
				defaultVal = true;
			}
			else
			{
				if (defaultVal)
					return false;
				if (tok == "")
					return false;
				if (!tok.regexMatches(m_validNameRegex))
					return false;
				if (l_paramExists("$" + tok, macro.params))
					return false;
				macro.params.push_back({ "$" + tok, MacroParamDefault });
			}
		}
		m_macros[name] = macro;
		return true;
	}

	stdvec<String> Stylesheet::parseMacroCall(const String& call, const Macro& macro)
	{
		stdvec<String> lines;

		auto l_splitByTopLevelComma = [](const String& call) -> stdvec<String> {
			stdvec<String> callArgs;
			i32 openP = 0;
			bool inStr = false;
			String arg = "";
			for (i32 i = 0; i < call.len(); i++)
			{
				char c = call[i];
				if (inStr)
				{
					arg += c;
					if (c == '"')
						inStr = false;
					continue;
				}
				if (c == '"')
				{
					arg += c;
					inStr = true;
					continue;
				}
				if (c == '(')
					openP++;
				if (c == ')')
					openP--;
				if (openP < 0)
					return {};
				if (c == ',' && openP == 0)
				{
					if (arg.trim() == "")
						return {};
					callArgs.push_back(arg);
					arg = "";
					continue;
				}
				arg += c;
			}
			if (inStr)  // unterminated string
				return {};
			if (arg.trim() == "")
				return {};
			callArgs.push_back(arg);
			return callArgs;
		};
		auto l_escapeRegex = [](const String& s) -> String {
			ostd::String result = s;
			result.replaceAll("\\", "\\\\");
			result.replaceAll(".",  "\\.");
			result.replaceAll("^",  "\\^");
			result.replaceAll("$",  "\\$");
			result.replaceAll("|",  "\\|");
			result.replaceAll("(",  "\\(");
			result.replaceAll(")",  "\\)");
			result.replaceAll("[",  "\\[");
			result.replaceAll("]",  "\\]");
			result.replaceAll("{",  "\\{");
			result.replaceAll("}",  "\\}");
			result.replaceAll("*",  "\\*");
			result.replaceAll("+",  "\\+");
			result.replaceAll("?",  "\\?");
			result.replaceAll("/",  "\\/");
			return result;
		};

		auto callArgs = l_splitByTopLevelComma(call);
		auto tokens = macro.body.tokenize("\n");
		for (auto& line : tokens)
		{
			i32 argIndex = 0;
			for (const auto&[_pname, _pval] : macro.params)
			{
				String pname = l_escapeRegex(_pname) + "\\b";
				String pval = "";
				if (argIndex < callArgs.size())
				{
					pval = callArgs[argIndex++].new_replaceAll("$", "$$");
					line.regexReplace(pname, pval);
					continue;
				}
				if (_pval == MacroParamDefault)
					return {};
				pval = _pval.new_replaceAll("$", "$$");
				line.regexReplace(pname, pval);
			}
			lines.push_back(line);
		}
		return lines;
	}

	void Stylesheet::debugPrint(void)
	{
		for (const auto&[key, value] : m_values)
		{
			std::cout << key << "  =  " << typeVariantToString(value) << "\n";
		}
		std::cout << "\n";
	}

	String Stylesheet::typeVariantToString(const TypeVariant& v)
	{
		if (auto p = std::get_if<i32>(&v))
			return String("").add(*p);
		else if (auto p = std::get_if<f32>(&v))
			return String("").add(*p);
		else if (auto p = std::get_if<bool>(&v))
			return STR_BOOL(*p);
		else if (auto p = std::get_if<String>(&v))
			return *p;
		else if (auto p = std::get_if<Color>(&v))
			return *p;
		else if (auto p = std::get_if<Rectangle>(&v))
			return *p;
		else if (auto p = std::get_if<Vec2>(&v))
			return *p;
		else if (auto p = std::get_if<ColorGradient>(&v))
			return *p;
		return "";
	}
}
