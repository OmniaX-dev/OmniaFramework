#ifndef __FILE_HPP__
#define __FILE_HPP__

#include <ostd/BaseObject.hpp>
#include <ostd/Types.hpp>
#include <filesystem>
#include <vector>

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

#endif
