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

#include <cstdint>
#include <ostd/data/Types.hpp>
#include <ostd/data/BaseObject.hpp>
#include <ostd/string/String.hpp>

namespace ostd
{
	class Color : public BaseObject
	{
		public: struct FloatCol
		{
			float r;
			float g;
			float b;
			float a;

			FloatCol(void) : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {  }
			FloatCol(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {  }
		};

		public: enum class eColorFormat { RGBA = 0, ARGB };

		public:
			Color(void);
			Color(uint8_t rgb_single_value, uint8_t alpha = 255);
			Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t alpha = 255);
			Color(const String& color_string);
			Color(const FloatCol& normalized_color);
			Color(const Color& copy);

			bool operator==(const Color& col2);
			bool operator!=(const Color& col2);
			Color& operator=(const Color& copy);

			Color& set(void);
			Color& set(uint8_t rgb_single_value, uint8_t alpha = 255);
			Color& set(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t alpha = 255);
			Color& set(const String& color_string);
			Color& set(const FloatCol& normalized_color);

			String hexString(bool include_alpha = false, String prefix = "0x") const;
			String rgbString(bool include_parenthesis = true, bool include_alpha = false) const;
			uint32_t asInteger(eColorFormat format = eColorFormat::RGBA) const;
			FloatCol getNormalizedColor(void) const;

			String toString(void) const override;
			void print(bool newLine = true, OutputHandlerBase* __destination = nullptr) const override;
			inline void invalidate(void) override {  }
			inline void setValid(bool valid) override {  }

			static void RGBtoHSV(float r, float g, float b, float& h, float& s, float& v);
			static void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b);

		public:
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
	};
}
