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

#include <filesystem>
#include <vector>
#include <ostd/data_types/BaseObject.hpp>
#include <ostd/data_types/Types.hpp>

namespace ostd
{
	class TextFileBuffer : public BaseObject
	{
		public:
			inline TextFileBuffer(void) { invalidate(); setTypeName("ostd::TextFileBuffer"); }
			inline TextFileBuffer(const std::filesystem::path& filePath) { open(filePath); }
			TextFileBuffer& open(const std::filesystem::path& filePath);

			inline const String& ext(void) const { return m_extension; }
			inline const String& fullPath(void) const { return m_fullPath; }
			inline const String& dirPath(void) const { return m_directoryPath; }
			inline const String& dir(void) const { return m_directoryName; }
			inline const String& fullName(void) const { return m_fullName; }
			inline const String& name(void) const { return m_name; }

			inline const String& rawContent(void) { return m_buffer; }
			std::vector<String> getLines(const String& separator = "\n", bool trim_tokens = true, bool allow_white_space_only_tokens = false);

			inline bool exists(void) { return isValid(); }

		private:
			String m_extension { "" };
			String m_fullPath { "" };
			String m_name { "" };
			String m_fullName { "" };
			String m_directoryPath { "" };
			String m_directoryName { "" };

			String m_buffer { "" };
	};
}
