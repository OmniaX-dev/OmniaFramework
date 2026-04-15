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
#include <ostd/string/String.hpp>
#include <ostd/data/Color.hpp>
#include <ostd/vendor/nlohmann/json.hpp> // IWYU pragma: keep

namespace ostd
{
	using json = nlohmann::json;

	class JsonFile
	{
		public:
			inline JsonFile(void) { m_loaded = false; }
			inline JsonFile(const String& filePath) { init(filePath); }
			bool init(const String& filePath, bool writeable = true, const json* obj = nullptr);
			inline bool isLoaded(void) const { return m_loaded; }

			template<class T> T get(const String& name);
	     	template<class T> bool set(const String& name, T value);


	        // --- GETTERS ---
	       	bool get_bool(const String& name);
	        i32 get_int(const String& name);
	        f64 get_double(const String& name);
	        String get_string(const String& name);
	        Color get_color(const String& name);
	        Vec2 get_vec2(const String& name);
	        Rectangle get_rect(const String& name);
	        stdvec<i32> get_int_array(const String& name);
	        stdvec<f64> get_double_array(const String& name);
	        stdvec<String> get_string_array(const String& name);
	        stdvec<Color> get_color_array(const String& name);
	        stdvec<Rectangle> get_rect_array(const String& name);

	        // --- SETTERS ---
	       	bool set_bool(const String& name, bool value);
	        bool set_int(const String& name, i32 value);
	        bool set_double(const String& name, f64 value);
	        bool set_string(const String& name, const String& value);
	        bool set_color(const String& name, const Color& value);
	        bool set_vec2(const String& name, const Vec2& value);
	        bool set_rect(const String& name, const Rectangle& value);
	        bool set_int_array(const String& name, const stdvec<i32>& value);
	        bool set_double_array(const String& name, const stdvec<f64>& value);
	        bool set_string_array(const String& name, const stdvec<String>& value);
	        bool set_color_array(const String& name, const stdvec<Color>& value);
	        bool set_rect_array(const String& name, const stdvec<Rectangle>& value);

	        // --- WRAPPERS ---
	        inline f32 get_float(const String& name) { return cast<f32>(get_double(name)); }

		private:
			void __validate_settings(void);
			bool __write_to_file(const json* obj = nullptr);

		private:
			String m_rawJson { "" };
			String m_filePath { "" };
			json m_json;
			bool m_loaded { false };
			bool m_writeable { true };
	};
}
