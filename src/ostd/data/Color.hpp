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
		public: struct Channel
		{
			uint8_t value;
			Color& parent;

			inline Channel(Color& _parent) : parent(_parent) { value = 0; }
			inline operator uint8_t() const { return value; }
			inline Channel& operator=(uint8_t v)
			{
			    value = v;
			    parent.m_dirty = true;
			    return *this;
			}
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
			Channel r;
			Channel g;
			Channel b;
			Channel a;

		private:
			mutable bool m_dirty { true };
    		mutable FloatCol m_cachedFloat { 0, 0, 0, 0 };

		public:
	};
	struct Colors
	{
		inline static const Color Transparent   {   0,   0,   0,   0 };
		inline static const Color Black         {   0,   0,   0, 255 };
		inline static const Color White         { 255, 255, 255, 255 };
		inline static const Color Gray          { 128, 128, 128, 255 };
		inline static const Color LightGray     { 192, 192, 192, 255 };
		inline static const Color DarkGray      {  64,  64,  64, 255 };

		inline static const Color Red           { 255,   0,   0, 255 };
		inline static const Color Green         {   0, 255,   0, 255 };
		inline static const Color Blue          {   0,   0, 255, 255 };

		inline static const Color Yellow        { 255, 255,   0, 255 };
		inline static const Color Cyan          {   0, 255, 255, 255 };
		inline static const Color Magenta       { 255,   0, 255, 255 };

		inline static const Color Orange        { 255, 165,   0, 255 };
		inline static const Color Purple        { 128,   0, 128, 255 };
		inline static const Color Brown         { 139,  69,  19, 255 };
		inline static const Color Pink          { 255, 192, 203, 255 };
		inline static const Color Lime          {  50, 205,  50, 255 };
		inline static const Color Olive         { 128, 128,   0, 255 };
		inline static const Color Teal          {   0, 128, 128, 255 };
		inline static const Color Navy          {   0,   0, 128, 255 };
		inline static const Color Maroon        { 128,   0,   0, 255 };
		inline static const Color Indigo        {  75,   0, 130, 255 };
		inline static const Color Gold          { 255, 215,   0, 255 };
		inline static const Color Silver        { 192, 192, 192, 255 };
		inline static const Color Beige         { 245, 245, 220, 255 };
		inline static const Color Coral         { 255, 127,  80, 255 };
		inline static const Color Salmon        { 250, 128, 114, 255 };
		inline static const Color Chocolate     { 210, 105,  30, 255 };
		inline static const Color Khaki         { 240, 230, 140, 255 };
		inline static const Color Lavender      { 230, 230, 250, 255 };
		inline static const Color Mint          { 189, 252, 201, 255 };
		inline static const Color SkyBlue       { 135, 206, 235, 255 };
		inline static const Color RoyalBlue     {  65, 105, 225, 255 };
		inline static const Color DeepSkyBlue   {   0, 191, 255, 255 };
		inline static const Color Turquoise     {  64, 224, 208, 255 };
		inline static const Color Aquamarine    { 127, 255, 212, 255 };
		inline static const Color ForestGreen   {  34, 139,  34, 255 };
		inline static const Color SeaGreen      {  46, 139,  87, 255 };
		inline static const Color SpringGreen   {   0, 255, 127, 255 };
		inline static const Color Firebrick     { 178,  34,  34, 255 };
		inline static const Color Crimson       { 220,  20,  60, 255 };
		inline static const Color Tomato        { 255,  99,  71, 255 };
		inline static const Color DarkOrange    { 255, 140,   0, 255 };
		inline static const Color DarkRed       { 139,   0,   0, 255 };
		inline static const Color DarkBlue      {   0,   0, 139, 255 };
		inline static const Color DarkGreen     {   0, 100,   0, 255 };
		inline static const Color DarkCyan      {   0, 139, 139, 255 };
		inline static const Color DarkMagenta   { 139,   0, 139, 255 };
		inline static const Color DarkYellow    { 204, 204,   0, 255 };
	};
}
