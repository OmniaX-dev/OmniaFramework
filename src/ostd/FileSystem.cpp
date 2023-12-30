#include "Utils.hpp"

namespace ostd
{
	std::vector<std::filesystem::path> Utils::listFilesInDirectory(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::directory_iterator(directoryPath.cpp_str()))
		{
			if (!std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}

	std::vector<std::filesystem::path> Utils::listDirectoriesInDirectory(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::directory_iterator(directoryPath.cpp_str()))
		{
			if (std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}
	
	std::vector<std::filesystem::path> Utils::listDirectory(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::directory_iterator(directoryPath.cpp_str()))
			list.push_back(file.path());
		return list;
	}

	std::vector<std::filesystem::path> Utils::listFilesInDirectoryRecursive(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::recursive_directory_iterator(directoryPath.cpp_str()))
		{
			if (!std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}

	std::vector<std::filesystem::path> Utils::listDirectoriesInDirectoryRecursive(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::recursive_directory_iterator(directoryPath.cpp_str()))
		{
			if (std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}
	
	std::vector<std::filesystem::path> Utils::listDirectoryRecursive(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::recursive_directory_iterator(directoryPath.cpp_str()))
			list.push_back(file.path());
		return list;
	}
	
}