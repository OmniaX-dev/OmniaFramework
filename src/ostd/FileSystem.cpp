#include "Utils.hpp"
#include "Logger.hpp"

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

	std::filesystem::path Utils::getHomeDirPath(void)
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

	std::filesystem::path Utils::getWorkingDirPath(void)
	{
		return std::filesystem::current_path();
	}

	bool Utils::ensureDirectory(const String& path)
	{
	    namespace fs = std::filesystem;

	    try {
	        fs::path dir(path);

	        if (!fs::exists(dir))
	        {
	            if (!fs::create_directories(dir))
	            {
	                OX_ERROR("Failed to create directory: %s", path.c_str());
					return false;
	            }
	        }
	        else if (!fs::is_directory(dir))
	        {
	            OX_ERROR("Path exists but is not a directory: %s", path.c_str());
				return false;
	        }
	    }
	    catch (const fs::filesystem_error& e)
	    {
	        OX_ERROR("Filesystem error for path '%s': %s", path.c_str(), e.what());
			return false;
	    }
		return true;
	}

	bool Utils::deleteDirectory(const String& path)
	{
	    namespace fs = std::filesystem;
	    try
	    {
	        if (fs::exists(path) && fs::is_directory(path))
	        {
	            fs::remove_all(path);
				return true;
	        }
	    } catch (const fs::filesystem_error& e)
	    {
	        OX_ERROR("Failed to delete tmp folder '%s': %s", path.c_str(), e.what());
			return false;
	    }
		return false;
	}
}
