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

#include "Lexer.hpp"

namespace ostd
{
	namespace db
	{
		stdvec<Lexer::Token> Lexer::run(const ostd::String& rawQuery) const
		{
			auto l_isLetter = [](char c) -> bool { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
			auto l_isNumber = [](char c) -> bool { return (c >= '0' && c <= '9'); };
			auto l_isOperatorStart = [](char c) -> bool { return c == '=' || c == '<' || c == '>' || c == '!'; };
			auto l_isSymbol = [](char c) -> bool { return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == '*' || c == ';'; };

			set_error(ErrorCodes::NoError);
			stdvec<Token> tokens;
			tokens.reserve(InitialTokenCount);

			u32 line = 1, col = 1;
			for (u32 index = 0; index < rawQuery.len(); index++)
			{
				char current = rawQuery[index];
				Token token;
				if (current == '\r') continue;
				else if (current == ' ' || current == '\t')
				{
					col++;
					continue;
				}
				else if (current == '\n')
				{
					col = 1;
					line++;
					continue;
				}
				else if (l_isLetter(current) || current == '_')
				{
					token = parse_keyword_or_identifier(rawQuery, index, line, col);
				}
				else if (current == '\"')
				{
					token = parse_string_literal(rawQuery, index, line, col);
				}
				else if (l_isNumber(current))
				{
					token = parse_number_literal(rawQuery, index, line, col);
				}
				else if (l_isOperatorStart(current))
				{
					token = parse_operator(rawQuery, index, line, col);
				}
				else if (l_isSymbol(current))
				{
					token = parse_symbol(rawQuery, index, line, col);
				}
				else if (current == '#')
				{
					token = parse_comment(rawQuery, index, line, col);
				}
				else
				{
					token = { eTokenType::Error, "", line, col };
					set_error(ErrorCodes::InvalidCharacter);
				}
				if (token.type == eTokenType::Error)
				{
					token.value = get_error_string();
					return { token };
				}
				tokens.push_back(token);
			}
			tokens.push_back({ eTokenType::EndOfInput, "", line, col });

			return tokens;
		}

		String Lexer::get_error_string(void) const
		{
			String err_str { "Error " + String::getHexStr(m_error) + ": " };
			switch (m_error)
			{
				case ErrorCodes::InvalidCharacter: err_str += "Invalid character.";
				case ErrorCodes::NoError:
				default:
					err_str = "";
			}
			return err_str;
		}

		Lexer::Token Lexer::parse_keyword_or_identifier(const String& rawQuery, u32& index, u32& line, u32& col) const
		{
			Token token { eTokenType::Error, "", line, col };
			bool identifierOnly { rawQuery[index] == '_' };
			// TODO
			return token;
		}

		Lexer::Token Lexer::parse_string_literal(const String& rawQuery, u32& index, u32& line, u32& col) const
		{
			Token token { eTokenType::Error, "", line, col };
			// TODO
			return token;
		}

		Lexer::Token Lexer::parse_number_literal(const String& rawQuery, u32& index, u32& line, u32& col) const
		{
			Token token { eTokenType::Error, "", line, col };
			// TODO
			return token;
		}

		Lexer::Token Lexer::parse_operator(const String& rawQuery, u32& index, u32& line, u32& col) const
		{
			Token token { eTokenType::Error, "", line, col };
			// TODO
			return token;
		}

		Lexer::Token Lexer::parse_symbol(const String& rawQuery, u32& index, u32& line, u32& col) const
		{
			Token token { eTokenType::Error, "", line, col };
			// TODO
			return token;
		}

		Lexer::Token Lexer::parse_comment(const String& rawQuery, u32& index, u32& line, u32& col) const
		{
			Token token { eTokenType::Error, "", line, col };
			// TODO
			return token;
		}
	}
}
