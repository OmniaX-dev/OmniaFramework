#pragma once

#include <vector>
#include <filesystem>
#include <ostd/string/String.hpp>

namespace ostd
{
	class FileSystem
	{
		public:
			static std::vector<std::filesystem::path> listFilesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectory(const String& directoryPath);
			static std::vector<std::filesystem::path> listFilesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoriesInDirectoryRecursive(const String& directoryPath);
			static std::vector<std::filesystem::path> listDirectoryRecursive(const String& directoryPath);

			static std::filesystem::path getHomeDirPath(void);
			static std::filesystem::path getWorkingDirPath(void);

			static bool ensureDirectory(const String& path);
			static bool deleteDirectory(const String& path);

			static bool directoryExists(const String& directoryPath);
			static bool fileExists(const String& filePath);
			static bool pathExists(const String& path);
	};
}
