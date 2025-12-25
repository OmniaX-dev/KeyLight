/*
    KeyLight - A MIDI Piano Visualizer
    Copyright (C) 2025  OmniaX-Dev

    This file is part of KeyLight.

    KeyLight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KeyLight is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KeyLight.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "JSONManager.hpp"
#include <fstream>
#include <ostd/File.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Logger.hpp>
#include <ostd/String.hpp>
#include <ostd/Utils.hpp>



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
	// ----- int ----------------------------------------------------------
	template<> struct Getter<int>
	{
	    static int exec_impl(const std::string& p, const json& root)
	    {
	        try { return root.at(p).get<int>(); }
	        catch (const json::exception&) { return 0; }
	    }
	};
	// ----- double -------------------------------------------------------
	template<> struct Getter<double>
	{
	    static double exec_impl(const std::string& p, const json& root)
	    {
	        try { return root.at(p).get<double>(); }
	        catch (const json::exception&) { return 0.0; }
	    }
	};
	// ----- ostd::String -------------------------------------------------
	template<> struct Getter<ostd::String>
	{
	    static ostd::String exec_impl(const std::string& p, const json& root)
	    {
	        try { return ostd::String(root.at(p).get<std::string>().c_str()); }
	        catch (const json::exception&) { return ostd::String(""); }
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
	                std::vector<int> vals;
	                for (const auto& v : node)
	                {
	                    if (!v.is_number_integer()) return ostd::Color();
	                    int iv = v.get<int>();
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
	                std::vector<float> vals;
	                vals.reserve(4);
	                for (const auto& v : node)
	                {
	                    if (!v.is_number_float() && !v.is_number_integer())
	                        return ostd::Rectangle();
	                    vals.push_back(v.get<float>());
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
	// ----- std::vector<int32_t> ----------------------------------------
	template<> struct Getter<std::vector<int32_t>>
	{
	    static std::vector<int32_t> exec_impl(const std::string& p, const json& root)
	    {
	        std::vector<int32_t> result;
	        try
			{
	            const json& arr = root.at(p);
	            if (!arr.is_array()) return result;
	            for (const auto& v : arr)
				{
	                if (v.is_number_integer())
	                    result.push_back(v.get<int32_t>());
	            }
	        } catch (...) {}
	        return result;
	    }
	};
	// ----- std::vector<double> -----------------------------------------
	template<> struct Getter<std::vector<double>>
	{
	    static std::vector<double> exec_impl(const std::string& p, const json& root)
	    {
	        std::vector<double> result;
	        try
			{
	            const json& arr = root.at(p);
	            if (!arr.is_array()) return result;
	            for (const auto& v : arr)
				{
	                if (v.is_number())
	                    result.push_back(v.get<double>());
	            }
	        } catch (...) {}
	        return result;
	    }
	};
	// ----- std::vector<ostd::String> -----------------------------------
	template<> struct Getter<std::vector<ostd::String>>
	{
	    static std::vector<ostd::String> exec_impl(const std::string& p, const json& root)
	    {
	        std::vector<ostd::String> result;
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
	// ----- std::vector<ostd::Color> -----------------------------------
	template<> struct Getter<std::vector<ostd::Color>>
	{
	    static std::vector<ostd::Color> exec_impl(const std::string& p, const json& root)
	    {
	        std::vector<ostd::Color> result;
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
	// ----- std::vector<ostd::Vec2> -----------------------------------
	template<> struct Getter<ostd::Vec2>
	{
	    static ostd::Vec2 exec_impl(const std::string& p, const json& root)
	    {
	        try
			{
	            const json& arr = root.at(p);
	            if (!arr.is_array() || arr.size() != 2)
	                return ostd::Vec2();
	            return ostd::Vec2(arr[0].get<float>(), arr[1].get<float>());
	        }
	        catch (...)
			{
	            return ostd::Vec2();
	        }
	    }
	};
	// ----- std::vector<ostd::Rectangle> -----------------------------------
	template<> struct Getter<std::vector<ostd::Rectangle>>
	{
	    static std::vector<ostd::Rectangle> exec_impl(const std::string& p, const json& root)
	    {
	        std::vector<ostd::Rectangle> result;
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

	// ----- int ----------------------------------------------------------
	template<> struct Setter<int>
	{
	    static bool exec_impl(const std::string& p, json& root, int value)
	    {
	        try
			{
				root[p] = value;
	            return true;
	        }
			catch (...) { return false; }
	    }
	};

	// ----- double -------------------------------------------------------
	template<> struct Setter<double>
	{
	    static bool exec_impl(const std::string& p, json& root, double value)
	    {
	        try
			{
				root[p] = value;
	            return true;
	        }
			catch (...) { return false; }
	    }
	};

	// ----- ostd::String -------------------------------------------------
	template<> struct Setter<ostd::String>
	{
	    static bool exec_impl(const std::string& p, json& root, const ostd::String& value)
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
	            arr.push_back(value.r);
	            arr.push_back(value.g);
	            arr.push_back(value.b);
	            if (value.a != 255) arr.push_back(value.a);
	            root[p] = arr;
	            return true;
	        } catch (...) { return false; }
	    }
	};
	// ----- std::vector<ostd::Vec2> -----------------------------------
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
	// ----- std::vector<int32_t> ----------------------------------------
	template<> struct Setter<std::vector<int32_t>>
	{
	    static bool exec_impl(const std::string& p, json& root, const std::vector<int32_t>& value)
	    {
	        try
			{
	            json arr = json::array();
	            for (int32_t v : value) arr.push_back(v);
	            root[p] = arr;
	            return true;
	        } catch (...) { return false; }
	    }
	};
	// ----- std::vector<double> -----------------------------------------
	template<> struct Setter<std::vector<double>>
	{
	    static bool exec_impl(const std::string& p, json& root, const std::vector<double>& value)
	    {
	        try
			{
	            json arr = json::array();
	            for (double v : value) arr.push_back(v);
	            root[p] = arr;
	            return true;
	        } catch (...) { return false; }
	    }
	};
	// ----- std::vector<ostd::String> -----------------------------------
	template<> struct Setter<std::vector<ostd::String>>
	{
	    static bool exec_impl(const std::string& p, json& root, const std::vector<ostd::String>& value)
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
	// ----- std::vector<ostd::Color> -----------------------------------
	template<> struct Setter<std::vector<ostd::Color>>
	{
	    static bool exec_impl(const std::string& p, json& root, const std::vector<ostd::Color>& value)
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
	// ----- std::vector<ostd::Rectangle> -----------------------------------
	template<> struct Setter<std::vector<ostd::Rectangle>>
	{
	    static bool exec_impl(const std::string& p, json& root, const std::vector<ostd::Rectangle>& value)
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
} // namespace detail


static inline bool __navigate_json_path(const ostd::String& path, json*& outSource, ostd::String& outName)
{
    auto tokens = path.tokenize(".");
    if (tokens.count() <= 1)
    {
	    if (!outSource->contains(path))
	        return false; // path broken
   		outName = path;
        return true;
    }

    ostd::String key = "";
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
T JSONManager::get(const ostd::String& name)
{
	json* source = &m_json;
	ostd::String settingName = "";
	if (__navigate_json_path(name, source, settingName))
		return detail::Getter<T>::exec_impl(settingName, *source);
	else
		OX_ERROR("Setting not found: %s", name.c_str());
    return detail::Getter<T>::exec_impl(settingName, *source);
}
template<class T>
bool JSONManager::set(const ostd::String& name, T value)
{
	if (!m_loaded) return false;
	if (!m_writeable) return false;

    auto tokens = name.tokenize(".");
    if (tokens.count() == 0) return false;

    json* current = &m_json;
    ostd::String key;

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

bool JSONManager::get_bool(const ostd::String& name) { return get<bool>(name); }
int JSONManager::get_int(const ostd::String& name) { return get<int32_t>(name); }
double JSONManager::get_double(const ostd::String& name) { return get<double>(name); }
ostd::String JSONManager::get_string(const ostd::String& name) { return get<ostd::String>(name); }
ostd::Color JSONManager::get_color(const ostd::String& name) { return get<ostd::Color>(name); }
ostd::Vec2 JSONManager::get_vec2(const ostd::String& name) { return get<ostd::Vec2>(name); }
ostd::Rectangle JSONManager::get_rect(const ostd::String& name) { return get<ostd::Rectangle>(name); }
std::vector<int32_t> JSONManager::get_int_array(const ostd::String& name) { return get<std::vector<int32_t>>(name); }
std::vector<double> JSONManager::get_double_array(const ostd::String& name) { return get<std::vector<double>>(name); }
std::vector<ostd::String> JSONManager::get_string_array(const ostd::String& name) { return get<std::vector<ostd::String>>(name); }
std::vector<ostd::Color> JSONManager::get_color_array(const ostd::String& name) { return get<std::vector<ostd::Color>>(name); }
std::vector<ostd::Rectangle> JSONManager::get_rect_array(const ostd::String& name) { return get<std::vector<ostd::Rectangle>>(name); }

bool JSONManager::set_bool(const ostd::String& name, bool value) { return set<bool>(name, value); }
bool JSONManager::set_int(const ostd::String& name, int32_t value) { return set<int32_t>(name, value); }
bool JSONManager::set_double(const ostd::String& name, double value) { return set<double>(name, value); }
bool JSONManager::set_string(const ostd::String& name, const ostd::String& value) { return set<ostd::String>(name, value); }
bool JSONManager::set_color(const ostd::String& name, const ostd::Color& value) { return set<ostd::Color>(name, value); }
bool JSONManager::set_vec2(const ostd::String& name, const ostd::Vec2& value) { return set<ostd::Vec2>(name, value); }
bool JSONManager::set_rect(const ostd::String& name, const ostd::Rectangle& value) { return set<ostd::Rectangle>(name, value); }
bool JSONManager::set_int_array(const ostd::String& name, const std::vector<int32_t>& value) { return set<std::vector<int32_t>>(name, value); }
bool JSONManager::set_double_array(const ostd::String& name, const std::vector<double>& value) { return set<std::vector<double>>(name, value); }
bool JSONManager::set_string_array(const ostd::String& name, const std::vector<ostd::String>& value) { return set<std::vector<ostd::String>>(name, value); }
bool JSONManager::set_color_array(const ostd::String& name, const std::vector<ostd::Color>& value) { return set<std::vector<ostd::Color>>(name, value); }
bool JSONManager::set_rect_array(const ostd::String& name, const std::vector<ostd::Rectangle>& value) { return set<std::vector<ostd::Rectangle>>(name, value); }


bool JSONManager::init(const ostd::String& filePath, bool writeable, const json* obj)
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

bool JSONManager::__write_to_file(const json* obj)
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
