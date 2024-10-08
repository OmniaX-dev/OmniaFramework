#pragma once

#include <ogfx/FontUtils.hpp>
#include <ostd/Geometry.hpp>
#include <ogfx/Image.hpp>

namespace ogfx
{
	class WindowBase;
	class BasicRenderer2D
	{
		public:
			BasicRenderer2D(void) = default;
			void init(WindowBase& window);

			inline TTFRenderer& getTTFRenderer(void) { return m_ttfr; }
			inline ostd::IPoint getStringSize(const ostd::String str, int32_t fontSize = 0) { return m_ttfr.getStringDimensions(str, fontSize); }
			inline WindowBase& getWindow(void) { return *m_window; }
			inline bool isInitialized(void) { return m_initialized; }
			void setFont(const ostd::String& fontFilePath);
			void setFontSize(int32_t fontSize);

            void drawImage(const ogfx::Image& image, const ostd::Vec2& position);

			void drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, int32_t fontSize = 0);
			void drawCenteredString(const ostd::String& str, const ostd::Vec2& center, const ostd::Color& color, int32_t fontSize = 0);

			void drawLine(const ostd::FLine& line, const ostd::Color& color, int32_t thickness = 1);

			void drawRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t thickness = 1);
			void drawRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t radius, int32_t thickness = 1);
			void drawCircle(const ostd::Vec2& center, int32_t radius, const ostd::Color& color, int32_t thickness = 1);

			void fillRect(const ostd::Rectangle& rect, const ostd::Color& color);
			void fillRoundRect(const ostd::Rectangle& rect, const ostd::Color& color, int32_t radius);
			void fillCircle(const ostd::Vec2& center, int32_t radius, const ostd::Color& color);

			void outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = 1);
			void outlinedRoundRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t radius, int32_t outlineThickness = 1);
			void outlinedCircle(const ostd::Vec2& center, int32_t radius, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = 1);

		private:
			TTFRenderer m_ttfr;
			WindowBase* m_window { nullptr };
			bool m_initialized { false };
	};
}
