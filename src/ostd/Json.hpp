#pragma once

#include <ostd/Geometry.hpp>
#include <ostd/String.hpp>
#include <ostd/Color.hpp>
#include <ostd/vendor/nlohmann/json.hpp> // IWYU pragma: keep

namespace ostd
{
	using json = nlohmann::json;

	class JsonFile
	{
		public:
			inline JsonFile(void) { m_loaded = false; }
			inline JsonFile(const ostd::String& filePath) { init(filePath); }
			bool init(const ostd::String& filePath, bool writeable = true, const json* obj = nullptr);

			template<class T> T get(const ostd::String& name);
	     	template<class T> bool set(const ostd::String& name, T value);


	        // --- GETTERS ---
	       	bool get_bool(const ostd::String& name);
	        int32_t get_int(const ostd::String& name);
	        double get_double(const ostd::String& name);
	        ostd::String get_string(const ostd::String& name);
	        ostd::Color get_color(const ostd::String& name);
	        ostd::Vec2 get_vec2(const ostd::String& name);
	        ostd::Rectangle get_rect(const ostd::String& name);
	        std::vector<int32_t> get_int_array(const ostd::String& name);
	        std::vector<double> get_double_array(const ostd::String& name);
	        std::vector<ostd::String> get_string_array(const ostd::String& name);
	        std::vector<ostd::Color> get_color_array(const ostd::String& name);
	        std::vector<ostd::Rectangle> get_rect_array(const ostd::String& name);

	        // --- SETTERS ---
	       	bool set_bool(const ostd::String& name, bool value);
	        bool set_int(const ostd::String& name, int32_t value);
	        bool set_double(const ostd::String& name, double value);
	        bool set_string(const ostd::String& name, const ostd::String& value);
	        bool set_color(const ostd::String& name, const ostd::Color& value);
	        bool set_vec2(const ostd::String& name, const ostd::Vec2& value);
	        bool set_rect(const ostd::String& name, const ostd::Rectangle& value);
	        bool set_int_array(const ostd::String& name, const std::vector<int32_t>& value);
	        bool set_double_array(const ostd::String& name, const std::vector<double>& value);
	        bool set_string_array(const ostd::String& name, const std::vector<ostd::String>& value);
	        bool set_color_array(const ostd::String& name, const std::vector<ostd::Color>& value);
	        bool set_rect_array(const ostd::String& name, const std::vector<ostd::Rectangle>& value);

	        // --- WRAPPERS ---
	        inline float get_float(const ostd::String& name) { return static_cast<float>(get_double(name)); }

		private:
			void __validate_settings(void);
			bool __write_to_file(const json* obj = nullptr);

		private:
			ostd::String m_rawJson { "" };
			ostd::String m_filePath { "" };
			json m_json;
			bool m_loaded { false };
			bool m_writeable { true };
	};
}
