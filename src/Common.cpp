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

#include "Common.hpp"

#include <ostd/Utils.hpp>
#include <ostd/Logger.hpp>

double Common::getCurrentTIme_ns(void)
{
	return std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void Common::ensureDirectory(const ostd::String& path)
{
    namespace fs = std::filesystem;

    try {
        fs::path dir(path);

        if (!fs::exists(dir))
        {
            if (!fs::create_directories(dir))
            {
                OX_ERROR("Failed to create directory: %s", path.c_str());
            }
        }
        else if (!fs::is_directory(dir))
        {
            OX_ERROR("Path exists but is not a directory: %s", path.c_str());
        }
    }
    catch (const fs::filesystem_error& e)
    {
        OX_ERROR("Filesystem error for path '%s': %s", path.c_str(), e.what());
    }
}

double Common::percentage(double n, double max)
{
    if (max == 0.0) return 0.0;
    return (n / max) * 100.0;
}

void Common::deleteDirectory(const ostd::String& path)
{
    namespace fs = std::filesystem;

    try
    {
        if (fs::exists(path) && fs::is_directory(path))
        {
            fs::remove_all(path);
        }
    } catch (const fs::filesystem_error& e)
    {
        OX_ERROR("Failed to delete tmp folder '%s': %s", path.c_str(), e.what());
    }
}
