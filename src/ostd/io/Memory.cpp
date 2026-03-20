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

#include "Memory.hpp"
#include <fstream>

namespace ostd
{
	void Memory::printByteStream(const ByteStream& data, StreamIndex start, uint8_t line_len, uint16_t n_rows, OutputHandlerBase& out, int32_t addrHighlight, uint32_t highlightRange, const String& title)
	{
		StreamIndex end = start + (n_rows * line_len);
		if (end > data.size()) end = data.size();
		String titleEdit(title);
		if (titleEdit.len() > 12)
			titleEdit = titleEdit.substr(0, 12);
		else if (titleEdit.len() < 12)
		{
			int32_t diff = 12 - titleEdit.len();
			for (int32_t i = 0; i < diff; i++)
				titleEdit.addChar(' ');
		}
		bool highlight = addrHighlight >= 0;
		uint8_t i = 1;
		ByteStream tmp;
		uint16_t linew = 1 + 1 + 6 + 1 + 1 + 2 + ((2 + 2) * line_len) + 1 + 4;
		out.fg(ConsoleColors::BrightBlue).p(String::duplicateChar('=', linew)).nl();
		if (line_len <= 0xFF)
		{
			out.fg(ConsoleColors::BrightBlue).p("|");
			out.fg(ConsoleColors::BrightMagenta).p(titleEdit);
			out.fg(ConsoleColors::BrightBlue).p("|  ");
			for (int32_t i = 0; i < line_len; i++)
				out.fg(ConsoleColors::Green).p(String::getHexStr(i, false, 1)).p("  ");
			out.fg(ConsoleColors::BrightBlue).p("|").nl();
			out.fg(ConsoleColors::BrightBlue).p(String::duplicateChar('=', linew)).nl();
		}
		out.fg(ConsoleColors::BrightBlue).p("| ");
		out.fg(ConsoleColors::BrightGray).p("0x");
		out.fg(ConsoleColors::BrightCyan).p(String::getHexStr(start, false, 4)).fg(ConsoleColors::BrightBlue).p(" |  ");
		for (StreamIndex addr = start; addr < end; addr++)
		{
			tmp.push_back(data[addr]);
			if (highlight && (addr >= (uint32_t)addrHighlight && addr < (uint32_t)(addrHighlight + highlightRange)))
				out.fg(ConsoleColors::Red);
			else if (data[addr] == 0)
				out.fg(ConsoleColors::BrightGray);
			else
				out.fg(ConsoleColors::White);
			out.p(String::getHexStr(data[addr], false)).p("  ");
			if (i++ % line_len == 0 || addr == end - 1)
			{
				i = 1;
				out.fg(ConsoleColors::BrightBlue).p("|");
				out.nl();
				out.fg(ConsoleColors::BrightBlue).p("|");
				out.fg(ConsoleColors::BrightGray).p("  --------  ").fg(ConsoleColors::BrightBlue).p("|").fg(ConsoleColors::BrightGray).p("  ");
				for (const auto& c : tmp)
				{
					if (isprint(c)) out.fg(ConsoleColors::BrightYellow).pChar((char)c).fg(ConsoleColors::BrightGray).p("   ");
					else out.fg(ConsoleColors::BrightGray).p(".   ");
				}
				out.fg(ConsoleColors::BrightBlue).p("| ");
				tmp.clear();
				out.reset();
				if (addr == end - 1) break;
				out.nl();
				out.fg(ConsoleColors::BrightBlue).p("| ");
				out.fg(ConsoleColors::BrightGray).p("0x");
				out.fg(ConsoleColors::BrightCyan).p(String::getHexStr(addr + 1, false, 4)).fg(ConsoleColors::BrightBlue).p(" |  ");
			}
		}
		out.nl().fg(ConsoleColors::BrightBlue).p(String::duplicateChar('=', linew)).nl().reset();
	}

	bool Memory::saveByteStreamToFile(const ByteStream& stream, const String& filePath)
	{
		std::ofstream writeFile;
		writeFile.open(filePath.cpp_str(), std::ios::out | std::ios::binary);
		writeFile.write((char*)(&stream[0]), stream.size());
		writeFile.close();
		return true;
	}

	bool Memory::loadByteStreamFromFile(const String& filePath, ByteStream& outStream)
	{
		std::ifstream rf(filePath.cpp_str(), std::ios::out | std::ios::binary);
		if(!rf) return false; //TODO: Error
		uint8_t cell = 0;
		while(rf.read((char*)&cell, sizeof(cell)))
			outStream.push_back(cell);
		if (outStream.size() == 0) return false; //TODO: Error
		return true;
	}

	ByteStream Memory::stringToByteStream(const String& data)
	{
		ByteStream bstream;
		for (auto& c : data)
			bstream.push_back((int8_t)c);
		return bstream;
	}

	String Memory::byteStreamToString(const ByteStream& data)
	{
		String out_string = "";
		for (int64_t i = 0; i < data.size(); i++)
		{
			if (data[i] == 0) break;
			out_string.addChar((char)data[i]);
		}
		return out_string;
	}
}
