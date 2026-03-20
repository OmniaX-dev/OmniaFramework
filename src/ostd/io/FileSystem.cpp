#include "FileSystem.hpp"
#include "Logger.hpp"
#include <fstream>

namespace ostd
{
	std::vector<std::filesystem::path> FileSystem::listFilesInDirectory(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::directory_iterator(directoryPath.cpp_str()))
		{
			if (!std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}

	std::vector<std::filesystem::path> FileSystem::listDirectoriesInDirectory(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::directory_iterator(directoryPath.cpp_str()))
		{
			if (std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}

	std::vector<std::filesystem::path> FileSystem::listDirectory(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::directory_iterator(directoryPath.cpp_str()))
			list.push_back(file.path());
		return list;
	}

	std::vector<std::filesystem::path> FileSystem::listFilesInDirectoryRecursive(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::recursive_directory_iterator(directoryPath.cpp_str()))
		{
			if (!std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}

	std::vector<std::filesystem::path> FileSystem::listDirectoriesInDirectoryRecursive(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::recursive_directory_iterator(directoryPath.cpp_str()))
		{
			if (std::filesystem::is_directory(file.path()))
				list.push_back(file.path());
		}
		return list;
	}

	std::vector<std::filesystem::path> FileSystem::listDirectoryRecursive(const String& directoryPath)
	{
		std::vector<std::filesystem::path> list;
		for (const auto& file : std::filesystem::recursive_directory_iterator(directoryPath.cpp_str()))
			list.push_back(file.path());
		return list;
	}


	std::filesystem::path FileSystem::getHomeDirPath(void)
	{
		String home_path = "";
		#ifdef WINDOWS_OS
			home_path = String(getenv("HOMEDRIVE")) + "\\" + String(getenv("HOMEPATH"));
		#elif defined(LINUX_OS) || defined(MAC_OS)
			home_path = String(getenv("HOME"));
		#else
			home_path = "NULL";
		#endif
		return home_path;
	}

	std::filesystem::path FileSystem::getWorkingDirPath(void)
	{
		return std::filesystem::current_path();
	}


	bool FileSystem::ensureDirectory(const String& directoryPath)
	{
		namespace fs = std::filesystem;

		try {
			fs::path dir(directoryPath);

			if (!fs::exists(dir))
			{
				if (!fs::create_directories(dir))
				{
					OX_ERROR("Failed to create directory: %s", directoryPath.c_str());
					return false;
				}
			}
			else if (!fs::is_directory(dir))
			{
				OX_ERROR("Path exists but is not a directory: %s", directoryPath.c_str());
				return false;
			}
		}
		catch (const fs::filesystem_error& e)
		{
			OX_ERROR("Filesystem error for path '%s': %s", directoryPath.c_str(), e.what());
			return false;
		}
		return true;
	}

	bool FileSystem::deleteDirectory(const String& directoryPath)
	{
		namespace fs = std::filesystem;
		try
		{
			if (fs::exists(directoryPath) && fs::is_directory(directoryPath))
			{
				fs::remove_all(directoryPath);
				return true;
			}
		} catch (const fs::filesystem_error& e)
		{
			OX_ERROR("Failed to delete folder '%s': %s", directoryPath.c_str(), e.what());
			return false;
		}
		return false;
	}

	bool FileSystem::ensureFile(const String& filePath, bool truncate)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::path p(filePath);
			if (p.empty())
			{
				OX_ERROR("Invalid file path: '%s'", filePath.c_str());
				return false;
			}
			fs::path parent = p.parent_path();
			if (parent.empty() || !fs::exists(parent) || !fs::is_directory(parent))
			{
				OX_ERROR("Parent directory does not exist for file: '%s'", filePath.c_str());
				return false;
			}
			if (fs::exists(p))
			{
				if (fs::is_directory(p))
				{
					OX_ERROR("Path exists but is a directory: '%s'", filePath.c_str());
					return false;
				}
				if (!truncate)
					return true;
				std::ofstream ofs(p, std::ios::trunc);
				if (!ofs)
				{
					OX_ERROR("Failed to truncate file: '%s'", filePath.c_str());
					return false;
				}
				return true;
			}
			else
			{
				std::ofstream ofs(p);
				if (!ofs)
				{
					OX_ERROR("Failed to create file: '%s'", filePath.c_str());
					return false;
				}
				return true;
			}
		}
		catch (const fs::filesystem_error& e)
		{
			OX_ERROR("Filesystem error for file '%s': %s", filePath.c_str(), e.what());
			return false;
		}
	}

	bool FileSystem::deleteFile(const String& filePath)
	{
		namespace fs = std::filesystem;
		try
		{
			if (fs::exists(filePath) && !fs::is_directory(filePath))
			{
				fs::remove(filePath);
				return true;
			}
		} catch (const fs::filesystem_error& e)
		{
			OX_ERROR("Failed to delete file '%s': %s", filePath.c_str(), e.what());
			return false;
		}
		return false;
	}


	bool FileSystem::directoryExists(const String& directoryPath)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::path p(directoryPath);
			return fs::exists(p) && fs::is_directory(p);
		}
		catch (const fs::filesystem_error&)
		{
			return false;
		}
	}

	bool FileSystem::fileExists(const String& filePath)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::path p(filePath);
			return fs::exists(p) && !fs::is_directory(p);
		}
		catch (const fs::filesystem_error&)
		{
			return false;
		}
	}

	bool FileSystem::pathExists(const String& path)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::path p(path);
			return fs::exists(p);
		}
		catch (const fs::filesystem_error&)
		{
			return false;
		}
	}

	bool FileSystem::isValidFileCreationPath(const ostd::String& filePath)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::path p(filePath);
			if (p.empty())
				return false;
			fs::path parent = p.parent_path();
			if (parent.empty())
				return false;
			if (!fs::exists(parent) || !fs::is_directory(parent))
				return false;
			if (fs::exists(p) && !fs::is_regular_file(p))
				return false;
			return true;
		}
		catch (const fs::filesystem_error&)
		{
			return false;
		}
	}

	bool FileSystem::isValidDirectoryCreationPath(const ostd::String& directoryPath)
	{
		namespace fs = std::filesystem;
		try {
			fs::path p(directoryPath);
			if (p.empty())
				return false;
			fs::path parent = p.parent_path();
			if (parent.empty())
				return false;
			if (!fs::exists(parent) || !fs::is_directory(parent))
				return false;
			if (fs::exists(p))
				return false;
			return true;
		}
		catch (const fs::filesystem_error&)
		{
			return false;
		}
	}

	FileSystem::ePathStatus FileSystem::getPathStatus(const ostd::String& path)
	{
		if (directoryExists(path))
			return ePathStatus::ExistingDirectory;
		if (fileExists(path))
			return ePathStatus::ExistingFile;
		if (isValidDirectoryCreationPath(path))
			return ePathStatus::ValidNewDirectory;
		if (isValidFileCreationPath(path))
			return ePathStatus::ValidNewFile;
		return ePathStatus::Invalid;
	}


	bool FileSystem::readTextFile(String fileName, std::vector<String>& outLines)
	{
		String line;
		std::ifstream file(fileName.cpp_str());
		if (file.fail()) return false;
		outLines.clear();
		while (std::getline(file, line.cpp_str_ref()))
			outLines.push_back(line);
		return true;
	}

	bool FileSystem::readTextFileRaw(String fileName, String& outString)
	{
		std::vector<ostd::String> lines;
		if (!readTextFile(fileName, lines))
			return false;
		outString.clr();
		for (const auto& line : lines)
			outString.add(line).add("\n");
		return true;
	}

	bool FileSystem::loadFileFromHppResource(String output_file_path, const char* resource_buffer, unsigned int size)
	{
		unsigned char ext_len = resource_buffer[0];
		String ext = "";
		for (unsigned char i = 0; i < ext_len; i++)
			ext += (char)(resource_buffer[i + 1]);
		if (String(output_file_path).trim().toLower().endsWith(ext))
			ext = "";
		std::fstream bin (output_file_path.cpp_str() + ext.cpp_str(), std::ios::out | std::ios::binary);
		if (!bin.is_open()) return false;
		bin.write(resource_buffer + ext_len + 1, size - ext_len - 1);
		bin.close();
		return true;
	}
}
