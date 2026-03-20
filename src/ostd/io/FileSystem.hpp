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

#include <vector>
#include <filesystem>
#include <ostd/string/String.hpp>

namespace ostd
{
	class FileSystem
	{
		public: enum class ePathStatus { Invalid = 0, ExistingDirectory, ValidNewDirectory, ExistingFile, ValidNewFile };

		public:
			static std::vector<std::filesystem::path> listFilesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listFilesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoryRecursive(const String& directoryPath);

			static std::filesystem::path getHomeDirPath(void);
			static std::filesystem::path getWorkingDirPath(void);

			static bool ensureDirectory(const String& directoryPath);
			static bool deleteDirectory(const String& directoryPath);
			static bool ensureFile(const String& filePath, bool truncate = true);
			static bool deleteFile(const String& filePath);

			static bool directoryExists(const String& directoryPath);
			static bool fileExists(const String& filePath);
			static bool pathExists(const String& path);
			static bool isValidFileCreationPath(const ostd::String& filePath);
			static bool isValidDirectoryCreationPath(const ostd::String& directoryPath);
			static ePathStatus getPathStatus(const ostd::String& path);

			static bool readTextFile(String fileName, std::vector<String>& outLines);
			static bool readTextFileRaw(String fileName, String& outString);
			static bool loadFileFromHppResource(String output_file_path, const char* resource_buffer, unsigned int size);
	};
}
