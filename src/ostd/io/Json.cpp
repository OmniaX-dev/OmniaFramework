#include "Json.hpp"
#include <fstream>
#include "../io/File.hpp"
#include "../io/Logger.hpp"
#include "../math/Geometry.hpp"
#include "../string/String.hpp"

namespace ostd
{
	namespace detail
	{
		template<class T> struct Getter;
		// ----- bool ---------------------------------------------------------
		template<> struct Getter<bool>
		{
		    static bool exec_impl(const std::string& p, const json& root)
		    {
		        try { return root.at(p).get<bool>(); }
		        catch (const json::exception&) { return false; }
		    }
		};
		// ----- i32 ----------------------------------------------------------
		template<> struct Getter<i32>
		{
		    static i32 exec_impl(const std::string& p, const json& root)
		    {
		        try { return root.at(p).get<i32>(); }
		        catch (const json::exception&) { return 0; }
		    }
		};
		// ----- f64 -------------------------------------------------------
		template<> struct Getter<f64>
		{
		    static f64 exec_impl(const std::string& p, const json& root)
		    {
		        try { return root.at(p).get<f64>(); }
		        catch (const json::exception&) { return 0.0; }
		    }
		};
		// ----- String -------------------------------------------------
		template<> struct Getter<String>
		{
		    static String exec_impl(const std::string& p, const json& root)
		    {
		        try { return String(root.at(p).get<std::string>().c_str()); }
		        catch (const json::exception&) { return String(""); }
		    }
		};
		// ----- ostd::Color -------------------------------------------------
		template<> struct Getter<ostd::Color>
		{
		    static ostd::Color exec_impl(const std::string& p, const json& root)
		    {
		        try {
					const json& node = root.at(p);

		            // Case 1: Hex string ("#RRGGBBAA")
		            if (node.is_string())
		            {
		                const std::string& hex = node.get<std::string>();
		                if (hex.starts_with("#") && hex.size() == 9)  // 9 for #RRGGBBAA
			                return ostd::Color(hex);
		                return ostd::Color();  // Not a valid hex format
		            }

					// Case 2: Array of integers
		            if (node.is_array() && !node.empty())
		            {
		                stdvec<i32> vals;
		                for (const auto& v : node)
		                {
		                    if (!v.is_number_integer()) return ostd::Color();
		                    i32 iv = v.get<i32>();
		                    if (iv < 0 || iv > 255) return ostd::Color();
		                    vals.push_back(iv);
		                }

		                switch (vals.size())
		                {
		                    case 1: return ostd::Color(vals[0], vals[0], vals[0]);
		                    case 3: return ostd::Color(vals[0], vals[1], vals[2]);
		                    case 4: return ostd::Color(vals[0], vals[1], vals[2], vals[3]);
		                    default: return ostd::Color();
		                }
		            }

		            // Anything else → default color
		            return ostd::Color();
		        }
		        catch (...) { return ostd::Color(); }
		    }
		};
		// ----- ostd::Rectangle -------------------------------------------------
		template<> struct Getter<ostd::Rectangle>
		{
		    static ostd::Rectangle exec_impl(const std::string& p, const json& root)
		    {
		        try
		        {
		            const json& node = root.at(p);
		            if (node.is_array() && node.size() == 4)
		            {
		                stdvec<f32> vals;
		                vals.reserve(4);
		                for (const auto& v : node)
		                {
		                    if (!v.is_number_float() && !v.is_number_integer())
		                        return ostd::Rectangle();
		                    vals.push_back(v.get<f32>());
		                }
		                return ostd::Rectangle(vals[0], vals[1], vals[2], vals[3]);
		            }
		            return ostd::Rectangle();
		        }
		        catch (...)
		        {
		            return ostd::Rectangle();
		        }
		    }
		};
		// ----- stdvec<i32> ----------------------------------------
		template<> struct Getter<stdvec<i32>>
		{
		    static stdvec<i32> exec_impl(const std::string& p, const json& root)
		    {
		        stdvec<i32> result;
		        try
				{
		            const json& arr = root.at(p);
		            if (!arr.is_array()) return result;
		            for (const auto& v : arr)
					{
		                if (v.is_number_integer())
		                    result.push_back(v.get<i32>());
		            }
		        } catch (...) {}
		        return result;
		    }
		};
		// ----- stdvec<f64> -----------------------------------------
		template<> struct Getter<stdvec<f64>>
		{
		    static stdvec<f64> exec_impl(const std::string& p, const json& root)
		    {
		        stdvec<f64> result;
		        try
				{
		            const json& arr = root.at(p);
		            if (!arr.is_array()) return result;
		            for (const auto& v : arr)
					{
		                if (v.is_number())
		                    result.push_back(v.get<f64>());
		            }
		        } catch (...) {}
		        return result;
		    }
		};
		// ----- stdvec<String> -----------------------------------
		template<> struct Getter<stdvec<String>>
		{
		    static stdvec<String> exec_impl(const std::string& p, const json& root)
		    {
		        stdvec<String> result;
		        try
				{
		            const json& arr = root.at(p);
		            if (!arr.is_array()) return result;
		            for (const auto& v : arr)
					{
		                if (v.is_string())
		                    result.emplace_back(v.get<std::string>().c_str());
		            }
		        } catch (...) {}
		        return result;
		    }
		};
		// ----- stdvec<ostd::Color> -----------------------------------
		template<> struct Getter<stdvec<ostd::Color>>
		{
		    static stdvec<ostd::Color> exec_impl(const std::string& p, const json& root)
		    {
		        stdvec<ostd::Color> result;
		        try
		        {
		            const json& node = root.at(p);
		            if (!node.is_array() || node.empty())
		                return result;
		            result.reserve(node.size());
		            for (const auto& element : node)
		            {
		                // We temporarily create a fake "sub-root" that only contains one key "color"
		                // This lets us call the existing Getter<ostd::Color> without modifying it
		                json fake_root = { { "color", element } };
		                ostd::Color color = Getter<ostd::Color>::exec_impl("color", fake_root);
		                result.push_back(color);
		            }
		            return result;
		        }
		        catch (...)
		        {
		            return {};
		        }
		    }
		};
		// ----- stdvec<ostd::Vec2> -----------------------------------
		template<> struct Getter<ostd::Vec2>
		{
		    static ostd::Vec2 exec_impl(const std::string& p, const json& root)
		    {
		        try
				{
		            const json& arr = root.at(p);
		            if (!arr.is_array() || arr.size() != 2)
		                return ostd::Vec2();
		            return ostd::Vec2(arr[0].get<f32>(), arr[1].get<f32>());
		        }
		        catch (...)
				{
		            return ostd::Vec2();
		        }
		    }
		};
		// ----- stdvec<ostd::Rectangle> -----------------------------------
		template<> struct Getter<stdvec<ostd::Rectangle>>
		{
		    static stdvec<ostd::Rectangle> exec_impl(const std::string& p, const json& root)
		    {
		        stdvec<ostd::Rectangle> result;
		        try
		        {
		            const json& node = root.at(p);
		            if (!node.is_array() || node.empty())
		                return result;
		            result.reserve(node.size());
		            for (const auto& element : node)
		            {
		                json fake_root = { { "rect", element } };
		                ostd::Rectangle rect = Getter<ostd::Rectangle>::exec_impl("rect", fake_root);
		                result.push_back(rect);
		            }
		            return result;
		        }
		        catch (...)
		        {
		            return {};
		        }
		    }
		};







		template<class T> struct Setter;
		// ----- bool ---------------------------------------------------------
		template<> struct Setter<bool>
		{
		    static bool exec_impl(const std::string& p, json& root, bool value)
		    {
		        try
				{
					root[p] = value;
		            return true;
		        }
				catch (...) { return false; }
		    }
		};

		// ----- i32 ----------------------------------------------------------
		template<> struct Setter<i32>
		{
		    static bool exec_impl(const std::string& p, json& root, i32 value)
		    {
		        try
				{
					root[p] = value;
		            return true;
		        }
				catch (...) { return false; }
		    }
		};

		// ----- f64 -------------------------------------------------------
		template<> struct Setter<f64>
		{
		    static bool exec_impl(const std::string& p, json& root, f64 value)
		    {
		        try
				{
					root[p] = value;
		            return true;
		        }
				catch (...) { return false; }
		    }
		};

		// ----- String -------------------------------------------------
		template<> struct Setter<String>
		{
		    static bool exec_impl(const std::string& p, json& root, const String& value)
		    {
		        try
				{
					root[p] = value;
		            return true;
		        }
				catch (...) { return false; }
		    }
		};
		// ----- ostd::Color -------------------------------------------------
		template<> struct Setter<ostd::Color>
		{
		    static bool exec_impl(const std::string& p, json& root, const ostd::Color& value)
		    {
		        try
				{
		            json arr = json::array();
		            arr.push_back(cast<u8>(value.r));
		            arr.push_back(cast<u8>(value.g));
		            arr.push_back(cast<u8>(value.b));
		            if (value.a != 255) arr.push_back(cast<u8>(value.a));
		            root[p] = arr;
		            return true;
		        } catch (...) { return false; }
		    }
		};
		// ----- stdvec<ostd::Vec2> -----------------------------------
		template<> struct Setter<ostd::Vec2>
		{
		    static bool exec_impl(const std::string& p, json& root, const ostd::Vec2& value)
		    {
		        try
				{
		            json arr = json::array();
		            arr.push_back(value.x);
		            arr.push_back(value.y);
		            root[p] = arr;
		            return true;
		        }
		        catch (...)
				{
		            return false;
		        }
		    }
		};
		// ----- ostd::Rectangle -------------------------------------------------
		template<> struct Setter<ostd::Rectangle>
		{
		    static bool exec_impl(const std::string& p, json& root, const ostd::Rectangle& value)
		    {
		        try
		        {
		            json arr = json::array();
		            arr.push_back(value.x);
		            arr.push_back(value.y);
		            arr.push_back(value.w);
		            arr.push_back(value.h);
		            root[p] = arr;
		            return true;
		        }
		        catch (...)
		        {
		            return false;
		        }
		    }
		};
		// ----- stdvec<i32> ----------------------------------------
		template<> struct Setter<stdvec<i32>>
		{
		    static bool exec_impl(const std::string& p, json& root, const stdvec<i32>& value)
		    {
		        try
				{
		            json arr = json::array();
		            for (i32 v : value) arr.push_back(v);
		            root[p] = arr;
		            return true;
		        } catch (...) { return false; }
		    }
		};
		// ----- stdvec<f64> -----------------------------------------
		template<> struct Setter<stdvec<f64>>
		{
		    static bool exec_impl(const std::string& p, json& root, const stdvec<f64>& value)
		    {
		        try
				{
		            json arr = json::array();
		            for (f64 v : value) arr.push_back(v);
		            root[p] = arr;
		            return true;
		        } catch (...) { return false; }
		    }
		};
		// ----- stdvec<String> -----------------------------------
		template<> struct Setter<stdvec<String>>
		{
		    static bool exec_impl(const std::string& p, json& root, const stdvec<String>& value)
		    {
		        try
				{
		            json arr = json::array();
		            for (const auto& s : value) arr.push_back(s.cpp_str());
		            root[p] = arr;
		            return true;
		        } catch (...) { return false; }
		    }
		};
		// ----- stdvec<ostd::Color> -----------------------------------
		template<> struct Setter<stdvec<ostd::Color>>
		{
		    static bool exec_impl(const std::string& p, json& root, const stdvec<ostd::Color>& value)
		    {
		        try
		        {
		            json arr = json::array();
		            for (const auto& color : value)
		                arr.push_back(color.hexString(true, "#").cpp_str());
		            root[p] = arr;
		            return true;
		        }
		        catch (...)
		        {
		            return false;
		        }
		    }
		};
		// ----- stdvec<ostd::Rectangle> -----------------------------------
		template<> struct Setter<stdvec<ostd::Rectangle>>
		{
		    static bool exec_impl(const std::string& p, json& root, const stdvec<ostd::Rectangle>& value)
		    {
		        try
		        {
		            json arr = json::array();
		            for (const auto& rect : value)
		            {
		                json rect_arr = json::array();
		                rect_arr.push_back(rect.x);
		                rect_arr.push_back(rect.y);
		                rect_arr.push_back(rect.w);
		                rect_arr.push_back(rect.h);
		                arr.push_back(rect_arr);
		            }
		            root[p] = arr;
		            return true;
		        }
		        catch (...)
		        {
		            return false;
		        }
		    }
		};
	}


	static inline bool __navigate_json_path(const String& path, json*& outSource, String& outName)
	{
	    auto tokens = path.tokenize(".");
	    if (tokens.count() <= 1)
	    {
		    if (!outSource->contains(path))
		        return false; // path broken
	   		outName = path;
	        return true;
	    }

	    String key = "";
	    while (tokens.hasNext())
	    {
	        key = tokens.next();
	        if (!outSource->contains(key))
	            return false; // path broken

	        if (!tokens.hasNext())
	        {
	       		outName = key;
	            return true;
	        }

	        const json& child = (*outSource)[key];
	        if (!child.is_object())
	            return false; // path broken
	        outSource = const_cast<json*>(&child);
	    }
	    return false;
	}

	template<class T>
	T JsonFile::get(const String& name)
	{
		json* source = &m_json;
		String settingName = "";
		if (__navigate_json_path(name, source, settingName))
			return detail::Getter<T>::exec_impl(settingName, *source);
		else
			OX_ERROR("Setting not found: %s", name.c_str());
	    return detail::Getter<T>::exec_impl(settingName, *source);
	}
	template<class T>
	bool JsonFile::set(const String& name, T value)
	{
		if (!m_loaded) return false;
		if (!m_writeable) return false;

	    auto tokens = name.tokenize(".");
	    if (tokens.count() == 0) return false;

	    json* current = &m_json;
	    String key;

	    while (tokens.hasNext())
	    {
	        key = tokens.next();
	        if (!tokens.hasNext()) break;

	        if (!current->contains(key.cpp_str()) || !(*current)[key.cpp_str()].is_object())
	            (*current)[key.cpp_str()] = json::object();
	        current = &(*current)[key.cpp_str()];
	    }

	    // Set the leaf value
	    if (detail::Setter<T>::exec_impl(key.cpp_str(), *current, value))
	        return __write_to_file();

	    OX_ERROR("Failed to set value for: %s", name.c_str());
	    return false;
	}

	bool JsonFile::get_bool(const String& name) { return get<bool>(name); }
	i32 JsonFile::get_int(const String& name) { return get<i32>(name); }
	f64 JsonFile::get_double(const String& name) { return get<f64>(name); }
	String JsonFile::get_string(const String& name) { return get<String>(name); }
	ostd::Color JsonFile::get_color(const String& name) { return get<ostd::Color>(name); }
	ostd::Vec2 JsonFile::get_vec2(const String& name) { return get<ostd::Vec2>(name); }
	ostd::Rectangle JsonFile::get_rect(const String& name) { return get<ostd::Rectangle>(name); }
	stdvec<i32> JsonFile::get_int_array(const String& name) { return get<stdvec<i32>>(name); }
	stdvec<f64> JsonFile::get_double_array(const String& name) { return get<stdvec<f64>>(name); }
	stdvec<String> JsonFile::get_string_array(const String& name) { return get<stdvec<String>>(name); }
	stdvec<ostd::Color> JsonFile::get_color_array(const String& name) { return get<stdvec<ostd::Color>>(name); }
	stdvec<ostd::Rectangle> JsonFile::get_rect_array(const String& name) { return get<stdvec<ostd::Rectangle>>(name); }

	bool JsonFile::set_bool(const String& name, bool value) { return set<bool>(name, value); }
	bool JsonFile::set_int(const String& name, i32 value) { return set<i32>(name, value); }
	bool JsonFile::set_double(const String& name, f64 value) { return set<f64>(name, value); }
	bool JsonFile::set_string(const String& name, const String& value) { return set<String>(name, value); }
	bool JsonFile::set_color(const String& name, const ostd::Color& value) { return set<ostd::Color>(name, value); }
	bool JsonFile::set_vec2(const String& name, const ostd::Vec2& value) { return set<ostd::Vec2>(name, value); }
	bool JsonFile::set_rect(const String& name, const ostd::Rectangle& value) { return set<ostd::Rectangle>(name, value); }
	bool JsonFile::set_int_array(const String& name, const stdvec<i32>& value) { return set<stdvec<i32>>(name, value); }
	bool JsonFile::set_double_array(const String& name, const stdvec<f64>& value) { return set<stdvec<f64>>(name, value); }
	bool JsonFile::set_string_array(const String& name, const stdvec<String>& value) { return set<stdvec<String>>(name, value); }
	bool JsonFile::set_color_array(const String& name, const stdvec<ostd::Color>& value) { return set<stdvec<ostd::Color>>(name, value); }
	bool JsonFile::set_rect_array(const String& name, const stdvec<ostd::Rectangle>& value) { return set<stdvec<ostd::Rectangle>>(name, value); }


	bool JsonFile::init(const String& filePath, bool writeable, const json* obj)
	{
		m_filePath = filePath;
		m_rawJson = "";
		m_loaded = false;
		m_writeable = writeable;
		if (obj != nullptr)
			m_json = *obj;
		ostd::TextFileBuffer jsonFile(filePath);
		if (!jsonFile.exists())
		{
			if (!writeable)
			{
			   OX_ERROR("JSON read-only file doesn't exist: %s", filePath.c_str());
			   return false;
			}
			m_loaded = true;
			__write_to_file();
			m_loaded = false;
		}
		jsonFile = ostd::TextFileBuffer(filePath);
		try
		{
			m_json = json::parse(jsonFile.rawContent());
		}
		catch (const json::parse_error& e)
		{
		    OX_ERROR("JSON parse failed: %s", e.what());
		    return false;
		}
		m_rawJson = jsonFile.rawContent();
		m_loaded = true;
		OX_DEBUG("Loaded JSON file: <%s>", filePath.c_str());
		return true;
	}

	bool JsonFile::__write_to_file(const json* obj)
	{
		if (!m_loaded) return false;
		if (obj == nullptr)
			obj = &m_json;
		try
		{
			std::ofstream file(m_filePath.cpp_str(), std::ios::trunc);
			if (!file) throw std::runtime_error("Failed to open file.");
			file << obj->dump(4);
			if (file.fail()) throw std::runtime_error("Failed to write to file");
			file.flush();
	    	if (file.fail()) throw std::runtime_error("Failed to flush");
		}
		catch (const std::exception& e)
		{
			OX_ERROR("Failed to write JSON file: %s", e.what());
			return false;
		}
		return true;
	}
}
