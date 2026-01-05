#pragma once

#include <ostd/Types.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/String.hpp>
#include <ostd/TextStyleParser.hpp>

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
			inline virtual OutputHandlerBase& pStyled(const TextStyleBuilder::IRichStringBase& styled) { return *this; }
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
			OutputHandlerBase& pStyled(const TextStyleBuilder::IRichStringBase& styled) override;
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


	namespace legacy
	{
		struct ConsoleCol
		{
			inline static constexpr const char* Red = "red";
			inline static constexpr const char* BrightRed = "b-red";
			inline static constexpr const char* OnRed = "o-red";
			inline static constexpr const char* OnBrightRed = "ob-red";

			inline static constexpr const char* Green = "green";
			inline static constexpr const char* BrightGreen = "b-green";
			inline static constexpr const char* OnGreen = "o-green";
			inline static constexpr const char* OnBrightGreen = "ob-green";

			inline static constexpr const char* Blue = "blue";
			inline static constexpr const char* BrightBlue = "b-blue";
			inline static constexpr const char* OnBlue = "o-blue";
			inline static constexpr const char* OnBrightBlue = "ob-blue";

			inline static constexpr const char* Magenta = "magenta";
			inline static constexpr const char* BrightMagenta = "b-magenta";
			inline static constexpr const char* OnMagenta = "o-magenta";
			inline static constexpr const char* OnBrightMagenta = "ob-magenta";

			inline static constexpr const char* Cyan = "cyan";
			inline static constexpr const char* BrightCyan = "b-cyan";
			inline static constexpr const char* OnCyan = "o-cyan";
			inline static constexpr const char* OnBrightCyan = "ob-cyan";

			inline static constexpr const char* Yellow = "yellow";
			inline static constexpr const char* BrightYellow = "b-yellow";
			inline static constexpr const char* OnYellow = "o-yellow";
			inline static constexpr const char* OnBrightYellow = "ob-yellow";

			inline static constexpr const char* Black = "gray";
			inline static constexpr const char* BrightGray = "b-gray";
			inline static constexpr const char* OnBlack = "o-gray";
			inline static constexpr const char* OnBrightGray = "ob-gray";

			inline static constexpr const char* Gray = "lgray";
			inline static constexpr const char* White = "white";
			inline static constexpr const char* onGray = "o-lgray";
			inline static constexpr const char* OnWhite = "o-white";
		};

		class IOutputHandler
		{
			public:
				virtual ~IOutputHandler(void) = default;
				virtual IOutputHandler& col(String color) = 0;
				virtual IOutputHandler& col(const Color& color) = 0;
				//virtual IOutputHandler& p(String str) = 0;
				virtual IOutputHandler& p(char c) = 0;
				virtual IOutputHandler& p(const StringEditor& se) = 0;
				//virtual IOutputHandler& p(const BaseObject& bo) = 0;
				virtual IOutputHandler& pi(uint8_t i) = 0;
				virtual IOutputHandler& pi(int8_t i) = 0;
				virtual IOutputHandler& pi(uint16_t i) = 0;
				virtual IOutputHandler& pi(int16_t i) = 0;
				virtual IOutputHandler& pi(uint32_t i) = 0;
				virtual IOutputHandler& pi(int32_t i) = 0;
				virtual IOutputHandler& pi(uint64_t i) = 0;
				virtual IOutputHandler& pi(int64_t i) = 0;
				virtual IOutputHandler& pf(float f, uint8_t precision = 0) = 0;
				virtual IOutputHandler& pf(double f, uint8_t precision = 0) = 0;
				virtual IOutputHandler& pStyled(const StringEditor& styled) = 0;
				virtual IOutputHandler& pStyled(const TextStyleParser::tStyledString& styled) = 0;
				virtual IOutputHandler& pStyled(const TextStyleBuilder::IRichStringBase& styled) = 0;
				virtual IOutputHandler& nl(void) = 0;
				virtual IOutputHandler& flush(void) = 0;
				virtual IOutputHandler& reset(void) = 0;

				virtual IOutputHandler& bgcol(const Color& color) = 0;
				virtual IOutputHandler& bgcol(String color) = 0;
				virtual IOutputHandler& resetColors(void) = 0;
				virtual IOutputHandler& clear(void) = 0;
		};

		class ConsoleOutputHandler : public IOutputHandler
		{
			public:
				IOutputHandler& col(String color) override;
				IOutputHandler& col(const Color& color) override;
				inline IOutputHandler& bgcol(const Color& color) override { return *this; }
				inline IOutputHandler& bgcol(String color) override { return *this; }
				inline IOutputHandler& resetColors(void) override { return *this; }
				//IOutputHandler& p(String str);
				IOutputHandler& p(char c) override;
				IOutputHandler& p(const StringEditor& se) override;
				//IOutputHandler& p(const BaseObject& bo);
				IOutputHandler& pi(uint8_t i) override;
				IOutputHandler& pi(int8_t i) override;
				IOutputHandler& pi(uint16_t i) override;
				IOutputHandler& pi(int16_t i) override;
				IOutputHandler& pi(uint32_t i) override;
				IOutputHandler& pi(int32_t i) override;
				IOutputHandler& pi(uint64_t i) override;
				IOutputHandler& pi(int64_t i) override;
				IOutputHandler& pf(float f, uint8_t precision = 0) override;
				IOutputHandler& pf(double f, uint8_t precision = 0) override;
				IOutputHandler& pStyled(const StringEditor& styled) override;
				IOutputHandler& pStyled(const TextStyleParser::tStyledString& styled) override;
				IOutputHandler& pStyled(const TextStyleBuilder::IRichStringBase& styled) override;
				IOutputHandler& nl(void) override;
				IOutputHandler& flush(void) override;
				IOutputHandler& reset(void) override;
				IOutputHandler& clear(void) override;
		};

		class BufferedOutputHandler : public IOutputHandler
		{
			public:
				inline IOutputHandler& col(String color) override { return *this; }
				inline IOutputHandler& col(const Color& color) override { return *this; }
				inline IOutputHandler& bgcol(const Color& color) override { return *this; }
				inline IOutputHandler& bgcol(String color) override { return *this; }
				inline IOutputHandler& resetColors(void) override { return *this; }
				IOutputHandler& p(char c) override;
				IOutputHandler& p(const StringEditor& se) override;
				IOutputHandler& pi(uint8_t i) override;
				IOutputHandler& pi(int8_t i) override;
				IOutputHandler& pi(uint16_t i) override;
				IOutputHandler& pi(int16_t i) override;
				IOutputHandler& pi(uint32_t i) override;
				IOutputHandler& pi(int32_t i) override;
				IOutputHandler& pi(uint64_t i) override;
				IOutputHandler& pi(int64_t i) override;
				IOutputHandler& pf(float f, uint8_t precision = 0) override;
				IOutputHandler& pf(double f, uint8_t precision = 0) override;
				IOutputHandler& pStyled(const StringEditor& styled) override { return *this; };
				inline IOutputHandler& pStyled(const TextStyleParser::tStyledString& styled) override { return *this; }
				inline IOutputHandler& pStyled(const TextStyleBuilder::IRichStringBase& styled) override { return *this; };
				IOutputHandler& nl(void) override;
				inline IOutputHandler& flush(void) override { return *this; }
				inline IOutputHandler& reset(void) override { return *this; }

				inline IOutputHandler& clear(void) override { m_buffer = ""; return *this; }
				inline const StringEditor& getBuffer(void) { return m_buffer; }

			private:
				StringEditor m_buffer;
		};
	}
}
