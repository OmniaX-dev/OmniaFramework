#include "File.hpp"
#include "Utils.hpp"
#include "String.hpp"

#include <fstream>
#include <ios>
#include <iterator>

namespace ostd
{
	TextFileBuffer& TextFileBuffer::open(const std::filesystem::path &filePath)
	{
		if (exists())
		{
			m_extension = "";
			m_fullPath = "";
			m_name = "";
			m_fullName = "";
			m_directoryPath = "";
			m_directoryName = "";
			m_buffer = "";
			invalidate();
		}
		if (!std::filesystem::is_regular_file(filePath)) return *this;
		std::ifstream file(filePath, std::ios::in | std::ios::binary);
		if (!file.is_open()) return *this;
		m_buffer.set({ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() });

		m_extension = filePath.extension().string();
		m_fullPath = filePath.string();
		m_fullName = filePath.filename().string();
		m_name = filePath.stem().string();
		String pathEditor(m_fullPath);
		if (pathEditor.contains('/'))
			m_directoryPath = pathEditor.new_substr(0, pathEditor.lastIndexOf('/'));
		pathEditor = m_directoryPath;
		if (pathEditor.contains('/'))
			m_directoryName = pathEditor.new_substr(pathEditor.lastIndexOf('/') + 1);
		validate();
		return *this;
	}
	
	std::vector<String> TextFileBuffer::getLines(const String& separator, bool trim_tokens, bool allow_white_space_only_tokens)
	{
		if (!exists()) return { };
		auto st = String(m_buffer);
		return st.tokenize(separator, trim_tokens, allow_white_space_only_tokens).getRawData();
	}
}
