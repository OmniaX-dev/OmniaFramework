/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

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

#include "String.hpp"

namespace ostd
{
	stdvec<u32> String::utf8::decode(const cpp_string& s)
	{
		stdvec<u32> out;
		out.reserve(s.size());  // Upper bound: 1 codepoint per byte (pure ASCII).

		const u8* p = reinterpret_cast<const u8*>(s.data());
		const u8* end = p + s.size();

		while (p < end)
		{
			u8 c = *p;

			// ASCII fast path — handles the vast majority of typical input.
			if (c < 0x80) {
				out.push_back(c);
				p++;
				continue;
			}

			// Decode multibyte sequence. Determine length from lead byte,
			// then read continuation bytes. Any malformed input emits U+FFFD
			// and advances by one byte to recover.
			u32 codepoint = 0;
			u32 expected = 0;

			if      ((c >> 5) == 0x6)  { expected = 1; codepoint = c & 0x1F; }
			else if ((c >> 4) == 0xE)  { expected = 2; codepoint = c & 0x0F; }
			else if ((c >> 3) == 0x1E) { expected = 3; codepoint = c & 0x07; }
			else {
				// Continuation byte where a lead was expected, or invalid lead.
				out.push_back(0xFFFD);
				p++;
				continue;
			}

			p++;
			bool ok = true;
			for (u32 i = 0; i < expected; i++) {
				if (p >= end || (*p & 0xC0) != 0x80) {
					ok = false;
					break;
				}
				codepoint = (codepoint << 6) | (*p & 0x3F);
				p++;
			}

			out.push_back(ok ? codepoint : 0xFFFD);
		}

		return out;
	}

	String String::utf8::encode(const stdvec<u32>& codepoints)
	{
		// Pre-compute exact byte size to allocate once. This walks the vector
		// twice but avoids any reallocation in the underlying string buffer.
		u32 total_bytes = 0;
		for (u32 cp : codepoints)
			total_bytes += encode_length(cp);  // Returns 0 for invalid → skipped.

		String out;
		out.cpp_str_ref().reserve(total_bytes);

		// Append-by-byte. Mirrors the single-codepoint utf8_encode logic but
		// writes directly into the output buffer with no intermediate Strings.
		for (u32 cp : codepoints)
		{
			if (cp < 0x80) {
				out.addChar(cast<char>(cp));
			}
			else if (cp < 0x800) {
				out.addChar(cast<char>(0xC0 | (cp >> 6)));
				out.addChar(cast<char>(0x80 | (cp & 0x3F)));
			}
			else if (cp < 0x10000) {
				// Reject UTF-16 surrogates: they are not valid standalone codepoints.
				if (cp >= 0xD800 && cp <= 0xDFFF) continue;
				out.addChar(cast<char>(0xE0 |  (cp >> 12)));
				out.addChar(cast<char>(0x80 | ((cp >> 6) & 0x3F)));
				out.addChar(cast<char>(0x80 |  (cp & 0x3F)));
			}
			else if (cp <= 0x10FFFF) {
				out.addChar(cast<char>(0xF0 |  (cp >> 18)));
				out.addChar(cast<char>(0x80 | ((cp >> 12) & 0x3F)));
				out.addChar(cast<char>(0x80 | ((cp >> 6)  & 0x3F)));
				out.addChar(cast<char>(0x80 |  (cp & 0x3F)));
			}
			// else: codepoint beyond Unicode range — skipped silently.
		}

		return out;
	}

	u32 String::utf8::align_to_codepoint(const cpp_string& s, u32 byte_pos)
	{
		if (byte_pos >= s.size()) return cast<u32>(s.size());
		// Already on a lead byte (or ASCII): nothing to do.
		if ((cast<u8>(s[byte_pos]) & 0xC0) != 0x80) return byte_pos;
		// We're on a continuation byte. Walk backward to the lead byte.
		u32 i = byte_pos;
		while (i > 0 && (cast<u8>(s[i]) & 0xC0) == 0x80)
			i--;
		return i;
	}

	u32 String::utf8::prev_codepoint_start(const cpp_string& s, u32 byte_pos)
	{
		if (byte_pos == 0 || s.empty()) return 0;
		// Clamp to a valid index. If we're past end, start from the last byte.
		u32 i = (byte_pos > s.size()) ? cast<u32>(s.size()) : byte_pos;
		if (i == 0) return 0;
		i--;
		// A continuation byte has the bit pattern 10xxxxxx (top two bits == 10).
		// Walk backward over continuation bytes until we find a lead byte
		// (anything that is NOT a continuation byte: ASCII or multibyte lead).
		while (i > 0 && (cast<u8>(s[i]) & 0xC0) == 0x80)
			i--;
		return i;
	}

	u32 String::utf8::next_codepoint_start(const cpp_string& s, u32 byte_pos)
	{
		if (byte_pos >= s.size()) return cast<u32>(s.size());
		u8 c = cast<u8>(s[byte_pos]);

		// ASCII fast path: advance by 1.
		if (c < 0x80) return byte_pos + 1;

		// Determine sequence length from the lead byte.
		u32 advance = 1;
		if      ((c >> 5) == 0x6)  advance = 2;  // 110xxxxx
		else if ((c >> 4) == 0xE)  advance = 3;  // 1110xxxx
		else if ((c >> 3) == 0x1E) advance = 4;  // 11110xxx
		// If c is a continuation byte (10xxxxxx) or invalid lead, fall back
		// to advancing by 1 — gracefully recover from malformed UTF-8.

		u32 result = byte_pos + advance;
		if (result > s.size()) result = cast<u32>(s.size());
		return result;
	}

	u8 String::utf8::codepoint_length(const cpp_string& s, u32 byte_pos)
	{
		if (byte_pos >= s.size()) return 0;
		u8 c = cast<u8>(s[byte_pos]);
		if (c < 0x80)              return 1;  // ASCII
		if ((c >> 5) == 0x6)       return 2;  // 110xxxxx
		if ((c >> 4) == 0xE)       return 3;  // 1110xxxx
		if ((c >> 3) == 0x1E)      return 4;  // 11110xxx
		// Continuation byte or invalid lead — caller is misaligned.
		// Return 1 so the caller still makes forward progress.
		return 1;
	}

	u8 String::utf8::encode_length(u32 codepoint)
	{
		if (codepoint < 0x80)        return 1;
		if (codepoint < 0x800)       return 2;
		if (codepoint < 0x10000)     return 3;
		if (codepoint <= 0x10FFFF)   return 4;
		return 0;  // Beyond Unicode's defined range.
	}

	String String::utf8::encode(u32 codepoint)
	{
		String out;
		if (codepoint < 0x80) {
			out.addChar(cast<char>(codepoint));
		}
		else if (codepoint < 0x800) {
			out.addChar(cast<char>(0xC0 | (codepoint >> 6)));
			out.addChar(cast<char>(0x80 | (codepoint & 0x3F)));
		}
		else if (codepoint < 0x10000) {
			out.addChar(cast<char>(0xE0 |  (codepoint >> 12)));
			out.addChar(cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
			out.addChar(cast<char>(0x80 |  (codepoint & 0x3F)));
		}
		else if (codepoint <= 0x10FFFF) {
			out.addChar(cast<char>(0xF0 |  (codepoint >> 18)));
			out.addChar(cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
			out.addChar(cast<char>(0x80 | ((codepoint >> 6)  & 0x3F)));
			out.addChar(cast<char>(0x80 |  (codepoint & 0x3F)));
		}
		// else: invalid codepoint — return empty String.
		return out;
	}

	u32 String::utf8::count_codepoints(const cpp_string& s)
	{
		u32 count = 0;
		const char* p = s.data();
		const char* end = p + s.size();
		while (p < end) {
			// Every byte that is NOT a continuation byte starts a new codepoint.
			if ((cast<u8>(*p) & 0xC0) != 0x80)
				count++;
			p++;
		}
		return count;
	}

	bool String::utf8::is_valid(const cpp_string& s)
	{
		const u8* p = reinterpret_cast<const u8*>(s.data());
		const u8* end = p + s.size();

		while (p < end)
		{
			u8 c = *p;
			u32 expected_continuations = 0;
			u32 codepoint = 0;
			u32 min_codepoint = 0;  // Used to reject overlong encodings.

			if (c < 0x80) {
				p++;
				continue;
			}
			else if ((c >> 5) == 0x6) {
				expected_continuations = 1;
				codepoint = c & 0x1F;
				min_codepoint = 0x80;
			}
			else if ((c >> 4) == 0xE) {
				expected_continuations = 2;
				codepoint = c & 0x0F;
				min_codepoint = 0x800;
			}
			else if ((c >> 3) == 0x1E) {
				expected_continuations = 3;
				codepoint = c & 0x07;
				min_codepoint = 0x10000;
			}
			else {
				// Continuation byte where a lead byte was expected, or a 5/6-byte
				// sequence (which is illegal in modern UTF-8).
				return false;
			}

			p++;
			for (u32 i = 0; i < expected_continuations; i++) {
				if (p >= end) return false;
				if ((*p & 0xC0) != 0x80) return false;  // Not a continuation byte.
				codepoint = (codepoint << 6) | (*p & 0x3F);
				p++;
			}

			// Reject overlong encodings (e.g., encoding U+0041 in 2 bytes).
			if (codepoint < min_codepoint) return false;
			// Reject codepoints outside Unicode's defined range.
			if (codepoint > 0x10FFFF) return false;
			// Reject UTF-16 surrogates (they're not valid standalone codepoints).
			if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return false;
		}

		return true;
	}

	String String::utf8::truncate(const String& s, u32 maxCps)
	{
		const cpp_string& bytes = s.cpp_str();
		u32 i = 0;
		u32 cpCount = 0;
		while (i < bytes.size() && cpCount < maxCps) {
			i = String::utf8::next_codepoint_start(bytes, i);
			cpCount++;
		}
		return s.new_substr(0, cast<i32>(i));
	}
}
