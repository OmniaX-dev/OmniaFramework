#pragma once

#include <ostd/Utils.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/IOHandlers.hpp>

namespace ostd
{
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

			inline OutputHandlerBase& xy(IPoint position) override { m_cursorPosition = position; __set_cursor(); return *this; }
			inline OutputHandlerBase& xy(int32_t x, int32_t y) override { m_cursorPosition = { x, y }; __set_cursor(); return *this; }
			inline OutputHandlerBase& x(int32_t x) override { m_cursorPosition.x = x; __set_cursor(); return *this; }
			inline OutputHandlerBase& y(int32_t y) override { m_cursorPosition.y = y; __set_cursor(); return *this; }
			
			void getConsoleSize(int32_t& outColumns, int32_t& outRows) override;
			IPoint getConsoleSize(void) override;
			
			inline IPoint getCursorPosition(void) override { return m_cursorPosition; }
			inline void getCursorPosition(int32_t& outX, int32_t& outY) override { outX = m_cursorPosition.x; outY = m_cursorPosition.y; }
			inline int32_t getCursorX(void) override { return m_cursorPosition.x; }
			inline int32_t getCursorY(void) override { return m_cursorPosition.y; }

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
			std::vector<tRichChar> m_buffer;
			std::vector<tRichChar> m_secondBuffer;
			bool m_validBuffer { false };

			tRichChar m_emptyChar {
				{ { 0, 0, 0, 255 }, "Black", false },
				{ { 0, 0, 0, 255 }, "Black", true },
				' '
			};
			
	};
}