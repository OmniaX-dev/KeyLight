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

#pragma once

#include <ostd/String.hpp>
#include <ostd/Color.hpp>
#include "vendor/nlohmann/json.hpp" // IWYU pragma: keep

using json = nlohmann::json;

class ConfigManager
{
	public: struct Settings
	{
		inline static const ostd::String useDefaultFFMPEG = "useSystenFFMPEG";
		inline static const ostd::String ffmpegPath = "ffmpegPath";
	};
	public:
		static bool init(const ostd::String& filePath);

		template<class T> static T get(const ostd::String& name);
     	template<class T> static bool set(const ostd::String& name, T value);


        // --- GETTERS ---
       	static bool get_bool(const ostd::String& name);
        static int32_t get_int(const ostd::String& name);
        static double get_double(const ostd::String& name);
        static ostd::String get_string(const ostd::String& name);
        static ostd::Color get_color(const ostd::String& name);
        static std::vector<int32_t> get_int_array(const ostd::String& name);
        static std::vector<double> get_double_array(const ostd::String& name);
        static std::vector<ostd::String> get_string_array(const ostd::String& name);

        // --- SETTERS ---
       	static bool set_bool(const ostd::String& name, bool value);
        static bool set_int(const ostd::String& name, int32_t value);
        static bool set_double(const ostd::String& name, double value);
        static bool set_string(const ostd::String& name, const ostd::String& value);
        static bool set_color(const ostd::String& name, const ostd::Color& value);
        static bool set_int_array(const ostd::String& name, const std::vector<int32_t>& value);
        static bool set_double_array(const ostd::String& name, const std::vector<double>& value);
        static bool set_string_array(const ostd::String& name, const std::vector<ostd::String>& value);

	private:
		static void __validate_settings(void);
		static bool __write_to_file(const json* obj = nullptr);

	private:
		inline static ostd::String m_rawJson { "" };
		inline static ostd::String m_filePath { "" };
		inline static json m_settingsJSON;
		inline static bool m_loaded { false };
};
