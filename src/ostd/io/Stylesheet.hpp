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

#pragma once

#include <ostd/string/TextStyleParser.hpp>
#include <ostd/data/Color.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/data/AnimationData.hpp>
#include <ostd/io/StaticHashMap.hpp>
#include <variant>


namespace ostd
{
	class Stylesheet
	{
		public: using QualifierList = stdvec<std::pair<const String, bool>>;
		public: using VariableList = stdumap<String, std::pair<String, bool>>;
		public: using TypeVariant = std::variant<i32, f32, bool, String, Color, Rectangle, Vec2, ColorGradient, AnimationData>;
		private: struct Macro
		{
			String body { "" };
			stdvec<std::pair<String, String>> params;

			inline i32 argCount(void) const { return params.size(); }
		};
		public:
			Stylesheet(void);
			Stylesheet& clear(void);
			Stylesheet& loadFromFile(const String& filePath, bool clearCurrentRules = true, VariableList variables = {});
			Stylesheet& loadFromString(const String& content, const String& filePath = "memory://", bool clearCurrentRules = true, VariableList variables = {});
			void set(const std::string& key, TypeVariant value, const String& themeID);
			void removeRule(const String& fullKey);
			void setFull(const String& fullKey, TypeVariant value);
			const TypeVariant* getVariant(const String& key, const stdvec<String>& themeIDList, const QualifierList& qualifierList) const;
			const TypeVariant* getFull(const String& fullKey) const;

			template<typename T>
			inline T get(const String& key, const T& fallback, const stdvec<String>& themeIDList, const QualifierList& qualifierList) const
			{
				if (auto v = getVariant(key, themeIDList, qualifierList))
				{
					// Direct match: return as-is
					if (auto p = std::get_if<T>(v))
						return *p;

					// Cross-cast for numeric types
					if constexpr (std::is_same_v<T, i32>)
					{
						if (auto p = std::get_if<f32>(v))
							return cast<i32>(*p);
					}
					else if constexpr (std::is_same_v<T, f32>)
					{
						if (auto p = std::get_if<i32>(v))
							return cast<f32>(*p);
					}
				}
				return fallback;
			}

			void debugPrint(void);
			String typeVariantToString(const TypeVariant& v);

		private:
			bool parseThemeFileLine(const String& line, const VariableList& variables, bool exitCondition = false);
			String parseGroupSelector(const String& rawSelector) const;
			stdvec<String> parseGroup(const String& selector, const stdvec<String>& group);
			Color parseColor(const String& _value, const VariableList& variables);
			Vec2 parseVec2(const String& _value, const VariableList& variables);
			Rectangle parseRect(const String& _value, const VariableList& variables);
			ColorGradient parseColorGradient(const String& _value, const VariableList& variables);
			AnimationData parseAnim(const String& _value, bool& outError, const VariableList& variables);
			String replaceVariables(const String& line, const VariableList& variables, bool stop_at_first_match = true);
			bool parseMacro(const String& macroCode);
			stdvec<String> parseMacroCall(const String& call, const Macro& macro);

		private:
			stdumap<String, TypeVariant> m_values;
			stdumap<String, Macro> m_macros;
			const String m_validNameRegex { "^[A-Za-z_][A-Za-z0-9_]*$" };

			inline static const String MacroParamDefault { "NO_VAL" };
	};
}
