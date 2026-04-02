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
#include <ostd/math/Geometry.hpp>
#include <variant>
#include <unordered_map>

namespace ogfx
{
	namespace gui
	{
		struct Theme
		{
			public: using ThemeValue = std::variant<int, float, bool, ostd::String, ostd::Color, ostd::Rectangle, ostd::Vec2>;
			public:
				inline Theme(bool blank = false)
				{
					if (blank) return;

					set("window.backgroundColor", ostd::Colors::Black);

					set("label.textColor", ostd::Colors::White);
					set("label.backgroundColor", ostd::Colors::Transparent);
					set("label.borderColor", ostd::Colors::White);
					set("label.fontSize", 20);
					set("label.borderRadius", 20);
					set("label.borderWidth", 2);
					set("label.showBackground", false);
					set("label.showBorder", false);
					set("label.padding", ostd::Rectangle { 5, 5, 5, 5 });
				}

				inline Theme& blank(void)
				{
					m_values.clear();
					return *this;
				}

				inline Theme& loadFromJsonFile(const ostd::String& jsonFilePath)
				{
					return *this; //TODO: Implement
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
				std::unordered_map<ostd::String, ThemeValue> m_values;
		};
	}
}
