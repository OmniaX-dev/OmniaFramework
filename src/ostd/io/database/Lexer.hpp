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

#pragma once

#include <ostd/string/String.hpp>

namespace ostd
{
	namespace db
	{
		class Lexer
		{
			public: enum class eKeyword { GET, IN, FOR, CROSS, IF, ORDER, UNIQUE, AS, OR, AND, NOT, MATCHES, BETWEEN, INCL, EXCL, INCL_L, INCL_R, MATCH, LEFT, RIGHT, FULL, ASC, DESC };
			public: enum class eTokenType {
				Keyword,
				Identifier,
				StringLit,
				NumberLit,
				Operator,     // == != < <= > >=
				Symbol,       // ( ) [ ] { } , . * ;
				Comment,
				EndOfInput,

				Error         // Used for error reporting
			};
			public: struct Token {
				eTokenType type;
				String value;
				u32 line;
				u32 col;
			};
			public: struct ErrorCodes {
				inline static constexpr u8 NoError { 0x00 };
				inline static constexpr u8 InvalidCharacter { 0x01 };
			};

			public:
				stdvec<Token> run(const String& rawQuery) const;
				inline bool hasError(void) const { return m_error != ErrorCodes::NoError; }
				inline u32 getErrorCode(void) const { return m_error; }

			private:
				inline void set_error(u8 errcode) const { m_error = errcode; }
				String get_error_string(void) const;

				Token parse_keyword_or_identifier(const String& rawQuery, u32& index, u32& line, u32& col) const;
				Token parse_string_literal(const String& rawQuery, u32& index, u32& line, u32& col) const;
				Token parse_number_literal(const String& rawQuery, u32& index, u32& line, u32& col) const;
				Token parse_operator(const String& rawQuery, u32& index, u32& line, u32& col) const;
				Token parse_symbol(const String& rawQuery, u32& index, u32& line, u32& col) const;
				Token parse_comment(const String& rawQuery, u32& index, u32& line, u32& col) const;

			private:
				mutable u32 m_error { ErrorCodes::NoError };
				inline static constexpr u32 InitialTokenCount { 200 };
				inline static const stdumap<String, eKeyword> Keywords {
					{ "GET", eKeyword::GET },
					{ "IN", eKeyword::IN },
					{ "FOR", eKeyword::FOR },
					{ "CROSS", eKeyword::CROSS },
					{ "IF", eKeyword::IF },
					{ "ORDER", eKeyword::ORDER },
					{ "UNIQUE", eKeyword::UNIQUE },
					{ "AS", eKeyword::AS },
					{ "OR", eKeyword::OR },
					{ "AND", eKeyword::AND },
					{ "NOT", eKeyword::NOT },
					{ "MATCHES", eKeyword::MATCHES },
					{ "BETWEEN", eKeyword::BETWEEN },
					{ "INCL", eKeyword::INCL },
					{ "EXCL", eKeyword::EXCL },
					{ "INCL_L", eKeyword::INCL_L },
					{ "INCL_R", eKeyword::INCL_R },
					{ "MATCH", eKeyword::MATCH },
					{ "LEFT", eKeyword::LEFT },
					{ "RIGHT", eKeyword::RIGHT },
					{ "FULL", eKeyword::FULL },
					{ "ASC", eKeyword::ASC },
					{ "DESC", eKeyword::DESC },
				};
		};
	}
}
