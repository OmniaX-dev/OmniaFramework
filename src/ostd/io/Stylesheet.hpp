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

namespace ostd
{
	class Stylesheet
	{
		public: using QualifierList = std::vector<std::pair<const ostd::String, bool>>;
		public: using TypeVariant = std::variant<int32_t, float, bool, ostd::String, ostd::Color, ostd::Rectangle, ostd::Vec2>;
		public:
			Stylesheet(void);
			Stylesheet& clear(void);
			Stylesheet& loadFromFile(const ostd::String& filePath, bool clearCurrentRules = true);
			Stylesheet& loadFromString(const ostd::String& content, const ostd::String& filePath = "memory://", bool clearCurrentRules = true);
			void set(const std::string& key, TypeVariant value, const ostd::String& themeID);
			void setFull(const ostd::String& fullKey, TypeVariant value);
			const TypeVariant* getVariant(const ostd::String& key, const std::vector<ostd::String>& themeIDList, const QualifierList& qualifierList) const;
			const TypeVariant* getFull(const ostd::String& fullKey) const;

			template<typename T>
			inline T get(const ostd::String& key, const T& fallback, const std::vector<ostd::String>& themeIDList, const QualifierList& qualifierList) const
			{
				if (auto v = getVariant(key, themeIDList, qualifierList))
				{
					if (auto p = std::get_if<T>(v))
						return *p;
				}
				return fallback;
			}

			void debugPrint(void);
			ostd::String typeVariantToString(const TypeVariant& v);

		private:
			bool parseThemeFileLine(const ostd::String& line);
			ostd::String parseGroupSelector(const ostd::String& rawSelector) const;
			std::vector<ostd::String> parseGroup(const ostd::String& selector, const std::vector<ostd::String>& group);

		private:
			std::unordered_map<ostd::String, TypeVariant> m_values;
			const ostd::String m_validNameRegex { "^[A-Za-z_][A-Za-z0-9_]*$" };
	};
}
