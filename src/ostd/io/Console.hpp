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

#pragma once

#include <ostd/math/Geometry.hpp>
#include <ostd/io/IOHandlers.hpp>
#include <ostd/string/String.hpp>
#include <ostd/utils/Defines.hpp>

#ifndef WINDOWS_OS
	#include <termios.h>
#else
	#include <conio.h>
#endif

namespace ostd
{
	class BasicConsole
	{
		public:
			static void clearConsole(void);
			static void getConsoleSize(i32& outColumns, i32& outRows);
			static void setConsoleCursorPosition(i32 x, i32 y);
			static IPoint getConsoleCursorPosition(void);
			static i32 getConsoleWidth(void);
			static i32 getConsoleHeight(void);
	};

	class InteractiveConsole : public OutputHandlerBase
	{
		public: enum class eMode { Direct = 0, Buffered = 1 };
		public: struct tRichChar {
			TextStyleParser::tColor foreground;
			TextStyleParser::tColor background;
			char ascii;
		};
		public:
			InteractiveConsole(void);
			InteractiveConsole(bool clearOnStart, eMode mode = eMode::Direct);

			OutputHandlerBase& bg(const ConsoleColors::tConsoleColor& color) override;
			OutputHandlerBase& bg(const String& color) override;
			OutputHandlerBase& fg(const ConsoleColors::tConsoleColor& color) override;
			OutputHandlerBase& fg(const String& color) override;

			OutputHandlerBase& pChar(char c) override;
			OutputHandlerBase& pStyled(const String& styled) override;
			OutputHandlerBase& pStyled(const TextStyleParser::tStyledString& styled) override;
			OutputHandlerBase& pStyled(TextStyleBuilder::IRichStringBase& styled) override;
			OutputHandlerBase& pObject(const BaseObject& bo) override;

			OutputHandlerBase& p(const String& se) override;
			OutputHandlerBase& p(u8 i) override;
			OutputHandlerBase& p(i8 i) override;
			OutputHandlerBase& p(u16 i) override;
			OutputHandlerBase& p(i16 i) override;
			OutputHandlerBase& p(u32 i) override;
			OutputHandlerBase& p(i32 i) override;
			OutputHandlerBase& p(u64 i) override;
			OutputHandlerBase& p(i64 i) override;
			OutputHandlerBase& p(f32 f, u8 precision = 0) override;
			OutputHandlerBase& p(f64 f, u8 precision = 0) override;

			OutputHandlerBase& nl(void) override;
			OutputHandlerBase& flush(void) override;
			OutputHandlerBase& reset(void) override;
			OutputHandlerBase& clear(void) override;

			inline OutputHandlerBase& xy(IPoint position) override { m_cursorPosition = position; __set_cursor(); return *this; }
			inline OutputHandlerBase& xy(i32 x, i32 y) override { m_cursorPosition = { x, y }; __set_cursor(); return *this; }
			inline OutputHandlerBase& x(i32 x) override { m_cursorPosition.x = x; __set_cursor(); return *this; }
			inline OutputHandlerBase& y(i32 y) override { m_cursorPosition.y = y; __set_cursor(); return *this; }

			void getConsoleSize(i32& outColumns, i32& outRows) override;
			IPoint getConsoleSize(void) override;

			inline IPoint getCursorPosition(void) override { return m_cursorPosition; }
			inline void getCursorPosition(i32& outX, i32& outY) override { outX = m_cursorPosition.x; outY = m_cursorPosition.y; }
			inline i32 getCursorX(void) override { return m_cursorPosition.x; }
			inline i32 getCursorY(void) override { return m_cursorPosition.y; }

			//InteractiveConsole
			inline void enableDirectMode(void) { m_mode = eMode::Direct; }
			inline void enableBufferedMode(void) { m_mode = eMode::Buffered; }
			inline void setMode(eMode mode) { m_mode = mode; }
			inline eMode getMode(void) { return m_mode; }

			void update(void);


		private:
			void __set_cursor(void);
			void __construct_buffer(void);
			void __swap_buffers(void);
			void __validate_and_clear_buffers(void);

		private:
			IPoint m_cursorPosition { 0, 0 };
			IPoint m_oldConsoleSize { 0, 0 };
			eMode m_mode { eMode::Direct };
			stdvec<tRichChar> m_buffer;
			stdvec<tRichChar> m_secondBuffer;
			bool m_validBuffer { false };

			tRichChar m_emptyChar {
				{ { 0, 0, 0, 255 }, "Black", false },
				{ { 0, 0, 0, 255 }, "Black", true },
				' '
			};

	};

	enum class eKeys
	{
		NoKeyPressed = 0,
		Backspace = 127,
		Enter = 10,
		Tab = 9,
		Up = 0x5B41,
		Down = 0x5B42,
		Left = 0x5B44,
		Right = 0x5B43,
		Escape = 27,
		Space = ' ',
		symExclamation = '!',
		symDoubleQuote = '\"',
		symPound = '#',
		symDollar = '$',
		symPercent = '%',
		symAmpersand = '&',
		symSingeQuote = '\'',
		symLeftParenthesis = '(',
		symRightParenthesis = ')',
		symAsterisk = '*',
		symPlus = '+',
		symComma = ',',
		symMinus = '-',
		symPeriod = '.',
		symSlash = '/',
		Num0 = '0', Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		symColon = ':',
		symSemicolon = ';',
		symLessThan = '<',
		symEquals = '=',
		symGreaterThan = '>',
		symQuestionMark = '?',
		symAtSign = '@',
		A = 'A',B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
		symLeftSquareBracket = '[',
		symBackslash = '\\',
		symRightSquareBracket = ']',
		symCaret = '^',
		symUnderscore = '_',
		symGraveAccent = 96,
		a = 'a',b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,
		symLeftCurlyBracket = '{',
		symVerticalBar = '|',
		symRightCurlyBracket = '}',
		symTilde = '~'
	};

	class KeyboardController
	{
		public:
			KeyboardController(void);
			~KeyboardController(void);

			eKeys getPressedKey(void);
			eKeys waitForKeyPress(void);

			inline String getInputString(void) { return m_cmd; }

			inline bool isOutputEnabled(void) { return m_output_enabled; }
			inline void enableOutput(bool __oe = true) { m_output_enabled = __oe; }
			inline void disableOutput(void) { enableOutput(false); }

			inline bool isCommandBufferEnabled(void) { return m_cmd_buffer_enabled; }
			inline void enableCommandBuffer(bool __cbe = true) { m_cmd_buffer_enabled = __cbe; }
			inline void disableCommandBuffer(void) { enableCommandBuffer(false); }

		private:
			String getKeyBuffer(void);
			String flushKeyBuffer(void);

#ifndef WINDOWS_OS
			i32 kbhit(void);
			i32 getch(void);
#endif


		private:
#ifndef WINDOWS_OS
			struct termios initial_settings, new_settings;
			i32 peek_character;
#endif
			stdvec<eKeys> m_key_buff;
			String m_cmd;
			bool m_output_enabled;
			bool m_cmd_buffer_enabled;
	};
}
