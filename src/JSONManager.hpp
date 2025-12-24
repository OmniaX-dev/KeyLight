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

class JSONManager
{
	// public: struct Settings
	// {
	// 	inline static const ostd::String useDefaultFFMPEG = "useSystenFFMPEG";
	// 	inline static const ostd::String ffmpegPath = "ffmpegPath";
	// };
	public:
		inline JSONManager(void) { m_loaded = false; }
		inline JSONManager(const ostd::String& filePath) { init(filePath); }
		bool init(const ostd::String& filePath, const json* obj = nullptr);

		template<class T> T get(const ostd::String& name);
     	template<class T> bool set(const ostd::String& name, T value);


        // --- GETTERS ---
       	bool get_bool(const ostd::String& name);
        int32_t get_int(const ostd::String& name);
        double get_double(const ostd::String& name);
        ostd::String get_string(const ostd::String& name);
        ostd::Color get_color(const ostd::String& name);
        std::vector<int32_t> get_int_array(const ostd::String& name);
        std::vector<double> get_double_array(const ostd::String& name);
        std::vector<ostd::String> get_string_array(const ostd::String& name);

        // --- SETTERS ---
       	bool set_bool(const ostd::String& name, bool value);
        bool set_int(const ostd::String& name, int32_t value);
        bool set_double(const ostd::String& name, double value);
        bool set_string(const ostd::String& name, const ostd::String& value);
        bool set_color(const ostd::String& name, const ostd::Color& value);
        bool set_int_array(const ostd::String& name, const std::vector<int32_t>& value);
        bool set_double_array(const ostd::String& name, const std::vector<double>& value);
        bool set_string_array(const ostd::String& name, const std::vector<ostd::String>& value);

	private:
		void __validate_settings(void);
		bool __write_to_file(const json* obj = nullptr);

	private:
		ostd::String m_rawJson { "" };
		ostd::String m_filePath { "" };
		json m_json;
		bool m_loaded { false };
};
