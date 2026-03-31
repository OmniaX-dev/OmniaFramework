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

#include "gui/Window.hpp"
#include <ogfx/render/FontUtils.hpp>
#include <ogfx/utils/Animation.hpp>
#include <ostd/math/Geometry.hpp>
#include <ogfx/resources/Image.hpp>

namespace ogfx
{
	class WindowCore;
	class BasicRenderer2D
	{
		public:
			BasicRenderer2D(void) = default;
			void init(WindowCore& window);

			inline TTFRenderer& getTTFRenderer(void) { return m_ttfr; }
			inline ostd::IPoint getStringSize(const ostd::String str, int32_t fontSize = 0) { return m_ttfr.getStringDimensions(str, fontSize); }
			inline WindowCore& getWindow(void) { return *m_window; }
			inline bool isInitialized(void) { return m_initialized; }
			void setFont(const ostd::String& fontFilePath);
			void setFontSize(int32_t fontSize);

            void drawImage(ogfx::Image& image, const ostd::Vec2& position, const ostd::Rectangle& rect = { 0, 0, 0, 0 });
            void drawAnimation(const Animation& anim, const ostd::Vec2& position);

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
			WindowCore* m_window { nullptr };
			bool m_initialized { false };
	};
}
