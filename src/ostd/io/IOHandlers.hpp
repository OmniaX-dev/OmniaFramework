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

#include <ostd/data/Types.hpp>
#include <ostd/math/Geometry.hpp>
#include <ostd/string/String.hpp>
#include <ostd/string/TextStyleParser.hpp>

namespace ogfx
{
	class WindowBase;
	class BasicRenderer2D;
};

namespace ostd
{
	class BaseObject;
	class ConsoleColors
	{
		public: struct tConsoleColor {
				String name;
				Color fullColor;
				bool background;

				inline const tConsoleColor asBackground(void) const { return { name, fullColor, true }; }
				inline const tConsoleColor asForeground(void) const { return { name, fullColor, false }; }

				private: inline tConsoleColor(void) = default;
				private: inline tConsoleColor(const String& _name, const Color& _color, bool _background) : name(_name), fullColor(_color), background(_background) {  }

				friend std::ostream& operator<<(std::ostream&, tConsoleColor const&);

				friend class ConsoleColors;
			};
		public:
			inline static const tConsoleColor Transparent { "transparent", { 0, 0, 0, 0 }, false};
			inline static const tConsoleColor Red { "red", { 255, 0, 0, 255 }, false};
			inline static const tConsoleColor BrightRed { "brightred", { 255, 70, 70, 255 }, false};
			inline static const tConsoleColor Green { "green", { 0, 255, 0, 255 }, false};
			inline static const tConsoleColor BrightGreen { "brightgreen", { 70, 255, 70, 255 }, false};
			inline static const tConsoleColor Blue { "blue", { 0, 0, 255, 255 }, false};
			inline static const tConsoleColor BrightBlue { "brightblue", { 70, 70, 255, 255 }, false};
			inline static const tConsoleColor Magenta { "magenta", { 255, 0, 255, 255 }, false};
			inline static const tConsoleColor BrightMagenta { "brightmagenta", { 255, 120, 255, 255 }, false};
			inline static const tConsoleColor Cyan { "cyan", { 0, 255, 255, 255 }, false};
			inline static const tConsoleColor BrightCyan { "brightcyan", { 170, 120, 255, 255 }, false};
			inline static const tConsoleColor Yellow { "yellow", { 255, 255, 0, 255 }, false};
			inline static const tConsoleColor BrightYellow { "brightyellow", { 255, 255, 170, 255 }, false};
			inline static const tConsoleColor Black { "black", { 0, 0, 0, 255 }, false};
			inline static const tConsoleColor Gray { "darkgray", { 50, 50, 50, 255 }, false};
			inline static const tConsoleColor BrightGray { "brightgray", { 150, 150, 150, 255 }, false};
			inline static const tConsoleColor White { "white", { 255, 255, 255, 255 }, false};

			inline static const tConsoleColor OnTransparent { "transparent", { 0, 0, 0, 0 }, true};
			inline static const tConsoleColor OnRed { "red", { 255, 0, 0, 255 }, true};
			inline static const tConsoleColor OnBrightRed { "brightred", { 255, 70, 70, 255 }, true};
			inline static const tConsoleColor OnGreen { "green", { 0, 255, 0, 255 }, true};
			inline static const tConsoleColor OnBrightGreen { "brightgreen", { 70, 255, 70, 255 }, true};
			inline static const tConsoleColor OnBlue { "blue", { 0, 255, 0, 255 }, true};
			inline static const tConsoleColor OnBrightBlue { "brightblue", { 70, 70, 255, 255 }, true};
			inline static const tConsoleColor OnMagenta { "magenta", { 255, 0, 255, 255 }, true};
			inline static const tConsoleColor OnBrightMagenta { "brightmagenta", { 255, 120, 255, 255 }, true};
			inline static const tConsoleColor OnCyan { "cyan", { 0, 255, 255, 255 }, true};
			inline static const tConsoleColor OnBrightCyan { "brightcyan", { 170, 120, 255, 255 }, true};
			inline static const tConsoleColor OnYellow { "yellow", { 255, 255, 0, 255 }, true};
			inline static const tConsoleColor OnBrightYellow { "brightyellow", { 255, 255, 170, 255 }, true};
			inline static const tConsoleColor OnBlack { "black", { 0, 0, 0, 255 }, true};
			inline static const tConsoleColor OnGray { "darkgray", { 50, 50, 50, 255 }, true};
			inline static const tConsoleColor OnBrightGray { "brightgray", { 150, 150, 150, 255 }, true};
			inline static const tConsoleColor OnWhite { "white", { 255, 255, 255, 255 }, true};

			inline static const tConsoleColor getFromName(const String& name, bool background = false) {
				String edit = name;
				edit.toLower().trim();
				if (edit == Red.name) return (background ? OnRed : Red);
				if (edit == BrightRed.name) return (background ? OnBrightRed : BrightRed);
				if (edit == Green.name) return (background ? OnGreen : Green);
				if (edit == BrightGreen.name) return (background ? OnBrightGreen : BrightGreen);
				if (edit == Blue.name) return (background ? OnBlue : Blue);
				if (edit == BrightBlue.name) return (background ? OnBrightBlue : BrightBlue);
				if (edit == Magenta.name) return (background ? OnMagenta : Magenta);
				if (edit == BrightMagenta.name) return (background ? OnBrightMagenta : BrightMagenta);
				if (edit == Cyan.name) return (background ? OnCyan : Cyan);
				if (edit == BrightCyan.name) return (background ? OnBrightCyan : BrightCyan);
				if (edit == Yellow.name) return (background ? OnYellow : Yellow);
				if (edit == BrightYellow.name) return (background ? OnBrightYellow : BrightYellow);
				if (edit == Black.name) return (background ? OnBlack : Black);
				if (edit == Gray.name) return (background ? OnGray : Gray);
				if (edit == BrightGray.name) return (background ? OnBrightGray : BrightGray);
				if (edit == White.name) return (background ? OnWhite : White);
				return (background ? OnBlack : Black);
			}
			inline static bool isConsoleColor(const String& name) {
				String edit = name;
				edit.toLower().trim();
				if (edit == Red.name) return true;
				if (edit == BrightRed.name) return true;
				if (edit == Green.name) return true;
				if (edit == BrightGreen.name) return true;
				if (edit == Blue.name) return true;
				if (edit == BrightBlue.name) return true;
				if (edit == Magenta.name) return true;
				if (edit == BrightMagenta.name) return true;
				if (edit == Cyan.name) return true;
				if (edit == BrightCyan.name) return true;
				if (edit == Yellow.name) return true;
				if (edit == BrightYellow.name) return true;
				if (edit == Black.name) return true;
				if (edit == Gray.name) return true;
				if (edit == BrightGray.name) return true;
				if (edit == White.name) return true;
				return false;
			}
	};

	class OutputHandlerBase
	{
		public:
			virtual ~OutputHandlerBase(void) = default;

			inline virtual OutputHandlerBase& bg(const Color& color) { return *this; }
			inline virtual OutputHandlerBase& bg(const ConsoleColors::tConsoleColor& color) { return *this; }
			inline virtual OutputHandlerBase& bg(const String& color) { return *this; }
			inline virtual OutputHandlerBase& fg(const Color& color) { return *this; }
			inline virtual OutputHandlerBase& fg(const ConsoleColors::tConsoleColor& color) { return *this; }
			inline virtual OutputHandlerBase& fg(const String& color) { return *this; }

			inline virtual OutputHandlerBase& pChar(char c) { return *this; }
			inline virtual OutputHandlerBase& pStyled(const String& styled) { return *this; }
			inline virtual OutputHandlerBase& pStyled(const TextStyleParser::tStyledString& styled) { return *this; }
			inline virtual OutputHandlerBase& pStyled(TextStyleBuilder::IRichStringBase& styled) { return *this; }
			inline virtual OutputHandlerBase& pObject(const BaseObject& bo) { return *this; }

			inline virtual OutputHandlerBase& p(const String& se) { return *this; }
			inline virtual OutputHandlerBase& p(uint8_t i) { return *this; }
			inline virtual OutputHandlerBase& p(int8_t i) { return *this; }
			inline virtual OutputHandlerBase& p(uint16_t i) { return *this; }
			inline virtual OutputHandlerBase& p(int16_t i) { return *this; }
			inline virtual OutputHandlerBase& p(uint32_t i) { return *this; }
			inline virtual OutputHandlerBase& p(int32_t i) { return *this; }
			inline virtual OutputHandlerBase& p(uint64_t i) { return *this; }
			inline virtual OutputHandlerBase& p(int64_t i) { return *this; }
			inline virtual OutputHandlerBase& p(float f, uint8_t precision = 0) { return *this; }
			inline virtual OutputHandlerBase& p(double f, uint8_t precision = 0) { return *this; }

			inline virtual OutputHandlerBase& nl(void) { return *this; }
			inline virtual OutputHandlerBase& tab(void) { return *this; }
			inline virtual OutputHandlerBase& flush(void) { return *this; }
			inline virtual OutputHandlerBase& reset(void) { return *this; }
			inline virtual OutputHandlerBase& clear(void) { return *this; }

			inline virtual OutputHandlerBase& xy(IPoint position) { return *this; }
			inline virtual OutputHandlerBase& xy(int32_t x, int32_t y) { return *this; }
			inline virtual OutputHandlerBase& x(int32_t x) { return *this; }
			inline virtual OutputHandlerBase& y(int32_t y) { return *this; }

			inline virtual IPoint getCursorPosition(void) { return { 0, 0 }; }
			inline virtual void getCursorPosition(int32_t& outX, int32_t& outY) {  }
			inline virtual int32_t getCursorX(void) { return 0; }
			inline virtual int32_t getCursorY(void) { return 0; }

			inline virtual void getConsoleSize(int32_t& outColumns, int32_t& outRows) {  }
			inline virtual IPoint getConsoleSize(void) { return { 0, 0 }; }
	};

	class ConsoleOutputHandler : public OutputHandlerBase
	{
		public:
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
			OutputHandlerBase& p(uint8_t i) override;
			OutputHandlerBase& p(int8_t i) override;
			OutputHandlerBase& p(uint16_t i) override;
			OutputHandlerBase& p(int16_t i) override;
			OutputHandlerBase& p(uint32_t i) override;
			OutputHandlerBase& p(int32_t i) override;
			OutputHandlerBase& p(uint64_t i) override;
			OutputHandlerBase& p(int64_t i) override;
			OutputHandlerBase& p(float f, uint8_t precision = 0) override;
			OutputHandlerBase& p(double f, uint8_t precision = 0) override;

			OutputHandlerBase& nl(void) override;
			OutputHandlerBase& flush(void) override;
			OutputHandlerBase& reset(void) override;
			OutputHandlerBase& clear(void) override;
	};
}
