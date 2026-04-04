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

#include "io/FileSystem.hpp"
#include "io/Logger.hpp"
#include "string/String.hpp"
#include <ostd/data/Color.hpp>
#include <ostd/math/Geometry.hpp>
#include <variant>
#include <unordered_map>

namespace ogfx
{
	namespace gui
	{
		struct Theme
		{
			public: using ThemeValue = std::variant<int32_t, float, bool, ostd::String, ostd::Color, ostd::Rectangle, ostd::Vec2>;
			public:
				inline Theme(bool blank = false)
				{
					if (blank) return;

					// Loading default theme
					set("window.backgroundColor", ostd::Colors::Black);

					set("label.textColor", ostd::Colors::White);
					set("label.backgroundColor", ostd::Colors::Red);
					set("label.borderColor", ostd::Colors::White);
					set("label.fontSize", 20);
					set("label.borderRadius", 20);
					set("label.borderWidth", 2);
					set("label.showBackground", true);
					set("label.showBorder", false);
					set("label.padding", ostd::Rectangle { 5, 5, 5, 5 });
				}

				inline Theme& blank(void)
				{
					m_values.clear();
					return *this;
				}

				inline Theme& loadFromFile(const ostd::String& filePath)
				{
					std::vector<ostd::String> lines;
					ostd::FileSystem::readTextFile(filePath, lines);
					uint32_t lineNumber = 0;
					ostd::String lineCopy = "";
					for (auto& line : lines)
					{
						lineNumber++;
						lineCopy = line;
						line.trim();
						if (line.startsWith("%"))
							continue;
						if (line.contains("%"))
							line.substr(0, line.indexOf("%")).trim();
						if (line == "")
							continue;
						if (!parseThemeFileLine(line))
							OX_WARN("Invalid theme line in file <%s:%d>:\n\t%s", filePath.c_str(), lineNumber, lineCopy.c_str());
					}
					return *this;
				}

				inline void set(const std::string& key, ThemeValue value)
				{
					m_values[key] = std::move(value);
				}

				inline const ThemeValue* __get(const ostd::String& key) const
				{
					auto it = m_values.find(key);
					return it != m_values.end() ? &it->second : nullptr;
				}

				template<typename T>
				inline T get(const ostd::String& key, const T& fallback) const
				{
					if (auto v = __get(key))
					{
						if (auto p = std::get_if<T>(v))
							return *p;
					}
					return fallback;
				}

			private:
				inline bool parseThemeFileLine(const ostd::String& line)
				{
					if (!line.contains("="))
						return false;
					ostd::String key = line.new_substr(0, line.indexOf("=")).trim();
					ostd::String value = line.new_substr(line.indexOf("=") + 1).trim().toLower();
					if (key == "")
						return false;
					if (value.isInt())
						set(key, static_cast<int32_t>(value.toInt()));
					else if (value.isNumeric(true))
						set(key, value.toFloat());
					else if (value == "true" || value == "false")
						set(key, value == "true");
					else if (value.startsWith("\"") && value.endsWith("\""))
						set(key, value);
					else if (value.startsWith("color(") && value.endsWith(")"))
					{
						value.substr(6, value.len() - 1).trim();
						set(key, ostd::Color(value));
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
						set(key, ostd::Vec2(vec[0], vec[1]));
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
						set(key, ostd::Rectangle(vec[0], vec[1], vec[2], vec[3]));
					}
					else
						return false;
					return true;
				}

			private:
				std::unordered_map<ostd::String, ThemeValue> m_values;
		};
	}
}
