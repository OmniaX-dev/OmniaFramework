#pragma once

#include <ostd/IOHandlers.hpp>
#include <ostd/TextStyleParser.hpp>
#include <ogfx/BasicRenderer.hpp>

namespace ogfx
{
	class WindowBaseOutputHandler : public ostd::OutputHandlerBase
	{
		public: enum class eWrapMode { TripleDots, NewLine };
		public:
			WindowBaseOutputHandler(void);
			void attachWindow(WindowBase& window);
			void setMonospaceFont(const ostd::String& filePath);
			ostd::Vec2 getStringSize(const ostd::String& str);
			bool isReady(void);
			void resetCursorPosition(void);
			void resetColors(void);
			void beginFrame(void);

			void setFontSize(int32_t fontSize);
			void setConsoleMaxCharacters(const ostd::IPoint& size);
			void setConsolePosition(const ostd::Vec2& pos);
			void setWrapMode(eWrapMode wrapMode);
			void setPadding(const ostd::Rectangle& rect);
			void setDefaultForegroundColor(const ostd::Color& color);
			void setDefaultBackgorundColor(const ostd::Color& color);
			void setTabWidth(uint8_t tw);

			int32_t getFontSize(void);
			ostd::Vec2 getCharacterSize(int32_t fontSize = 0);
			ostd::Vec2 getConsolePosition(void);
			eWrapMode getWrapMode(void);
			ostd::Rectangle getPadding(void);
			ostd::Color getDefaultBackgroundColor(void);
			ostd::Color getDefaultForegroundColor(void);
			uint8_t getTabWidth(void);
			ostd::Rectangle getConsoleBounds(void);

			WindowBaseOutputHandler& bg(const ostd::Color& color) override;
			WindowBaseOutputHandler& bg(const ostd::ConsoleColors::tConsoleColor& color) override;
			WindowBaseOutputHandler& bg(const ostd::String& color) override;
			WindowBaseOutputHandler& fg(const ostd::Color& color) override;
			WindowBaseOutputHandler& fg(const ostd::ConsoleColors::tConsoleColor& color) override;
			WindowBaseOutputHandler& fg(const ostd::String& color) override;

			WindowBaseOutputHandler& pChar(char c) override;
			WindowBaseOutputHandler& pStyled(const ostd::String& styled) override;
			WindowBaseOutputHandler& pStyled(const ostd::TextStyleParser::tStyledString& styled) override;
			WindowBaseOutputHandler& pStyled(ostd::TextStyleBuilder::IRichStringBase& styled) override;
			WindowBaseOutputHandler& pObject(const ostd::BaseObject& bo) override;

			WindowBaseOutputHandler& p(const ostd::String& se) override;
			WindowBaseOutputHandler& p(uint8_t i) override;
			WindowBaseOutputHandler& p(int8_t i) override;
			WindowBaseOutputHandler& p(uint16_t i) override;
			WindowBaseOutputHandler& p(int16_t i) override;
			WindowBaseOutputHandler& p(uint32_t i) override;
			WindowBaseOutputHandler& p(int32_t i) override;
			WindowBaseOutputHandler& p(uint64_t i) override;
			WindowBaseOutputHandler& p(int64_t i) override;
			WindowBaseOutputHandler& p(float f, uint8_t precision = 0) override;
			WindowBaseOutputHandler& p(double f, uint8_t precision = 0) override;

			WindowBaseOutputHandler& nl(void) override;
			WindowBaseOutputHandler& tab(void) override;
			WindowBaseOutputHandler& flush(void) override;
			WindowBaseOutputHandler& clear(void) override;
			WindowBaseOutputHandler& reset(void) override;

			WindowBaseOutputHandler& xy(ostd::IPoint position) override;
			WindowBaseOutputHandler& xy(int32_t x, int32_t y) override;
			WindowBaseOutputHandler& x(int32_t x) override;
			WindowBaseOutputHandler& y(int32_t y) override;

			ostd::IPoint getCursorPosition(void) override;
			void getCursorPosition(int32_t& outX, int32_t& outY) override;
			int32_t getCursorX(void) override;
			int32_t getCursorY(void) override;

			void getConsoleSize(int32_t& outColumns, int32_t& outRows) override;
			ostd::IPoint getConsoleSize(void) override;

		private:
			void __update_char_size(void);
			void __print_string(const ostd::String& str);

		private:
			ostd::Color m_backgroundColor;
			ostd::Color m_foregroundColor;
			ostd::Color m_defaultBackgroundColor { 0, 0, 0, 0 };
			ostd::Color m_defaultForegroundColor { 0, 220, 0, 255 };
			ostd::Vec2 m_curosrPosition;
			BasicRenderer2D m_renderer;
			WindowBase* m_window { nullptr };
			int32_t m_fontSize { 20 };
			ostd::Vec2 m_charSize;
			ostd::IPoint m_consoleSize { 0, 0 };
			bool m_ready { false };
			ostd::Vec2 m_consolePosition { 0.0f, 0.0f };
			eWrapMode m_wrapMode { eWrapMode::TripleDots };
			ostd::Rectangle m_padding { 10, 10, 10, 10 };
			uint8_t m_tabWidth { 8 };
	};
}
