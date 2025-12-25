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

#include <ostd/String.hpp>
#include <ostd/Utils.hpp>
#include <ostd/Logger.hpp>

#include <SFML/Audio.hpp>

#include <math.h>

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

double Common::percentage(double n, double max)
{
    if (max == 0.0) return 0.0;
    return (n / max) * 100.0;
}

sf::VertexArray Common::getMusicWaveForm(const ostd::String& filePath, int32_t windowHeight)
{
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(filePath.cpp_str()))
	{
		OX_ERROR("Error while trying to load audio file: %s", filePath.c_str());
		return {  };
	}
	const int16_t* samples = buffer.getSamples();
	std::size_t sampleCount = buffer.getSampleCount();
	unsigned int channels = buffer.getChannelCount();
	std::vector<float> amplitudes;
	amplitudes.reserve(1000);
	std::size_t samplesPerPixel = sampleCount / channels / 1000;
	for (std::size_t i = 0; i < sampleCount; i += samplesPerPixel * channels)
	{
		long sum = 0;
		for (unsigned int c = 0; c < channels; ++c)
			sum += std::abs(samples[i + c]);
		amplitudes.push_back(static_cast<float>(sum) / channels / 32768.f);
	}
	sf::VertexArray waveform(sf::PrimitiveType::LineStrip, amplitudes.size());
	for (std::size_t x = 0; x < amplitudes.size(); ++x) {
		float y = (1.f - amplitudes[x]) * windowHeight / 2.f;
		waveform[x].position = sf::Vector2f(static_cast<float>(x), y);
		waveform[x].color = sf::Color::White;
	}
	return waveform;
}

ostd::String Common::secondsToFormattedString(int32_t totalSeconds)
{
	int32_t hours   = totalSeconds / 3600;
	int32_t minutes = (totalSeconds % 3600) / 60;
	int32_t seconds = totalSeconds % 60;
	ostd::String fmtstr = "";
	fmtstr.add(ostd::String("").add(hours).addLeftPadding(2, '0')).add(":");
	fmtstr.add(ostd::String("").add(minutes).addLeftPadding(2, '0')).add(":");
	fmtstr.add(ostd::String("").add(seconds).addLeftPadding(2, '0'));
	return fmtstr;
}

void Common::RGBtoHSV(float r, float g, float b, float& h, float& s, float& v)
{
    float max = std::max({r, g, b});
    float min = std::min({r, g, b});
    float d = max - min;

    v = max;

    if (max == 0)
    {
        s = 0;
        h = 0;
        return;
    }

    s = d / max;

    if (max == r)      h = std::fmod((g - b) / d + 6.0f, 6.0f);
    else if (max == g) h = (b - r) / d + 2.0f;
    else               h = (r - g) / d + 4.0f;

    h /= 6.0f;
}

void Common::HSVtoRGB(float h, float s, float v, float& r, float& g, float& b)
{
    int i = static_cast<int>(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6)
    {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
}

std::vector<Common::tLocaleInfo> Common::getAvailableLocales(const std::string& localeDir)
{
	std::vector<tLocaleInfo> locales;
    static const std::unordered_map<std::string, ostd::String> nameMap = {
        {"en", "English"},
        {"es", "Español"},
        {"fr", "Français"},
        {"it", "Italiano"},
        {"de", "Deutsch"},
        {"ja", "日本語"},
        {"zh", "中文"},
        {"ru", "Русский"},
        {"pt", "Português"},
        {"ko", "한국어"}
    };

    if (!std::filesystem::exists(localeDir))
    {
        locales.push_back({0, "English", "en_US.UTF-8"});
        return locales;
    }

    uint32_t id = 0;
    for (const auto& entry : std::filesystem::directory_iterator(localeDir))
    {
        if (!entry.is_directory()) continue;
        std::string code = entry.path().filename().string();
        std::filesystem::path moPath = entry.path() / "LC_MESSAGES" / "KeyLight.mo";
        if (!std::filesystem::exists(moPath)) continue;

        auto it = nameMap.find(code);
        ostd::String displayName = (it != nameMap.end()) ? it->second : ostd::String(code.c_str());

        locales.push_back({
            id++,
            displayName,
            getFullLocale(code)
        });
    }

    return locales;
}

ostd::String Common::getFullLocale(const std::string& shortCode)
{
    static const std::unordered_map<std::string, std::string> localeMap = {
        {"en", "en_US.UTF-8"},
        {"es", "es_ES.UTF-8"},
        {"fr", "fr_FR.UTF-8"},
        {"it", "it_IT.UTF-8"},
        {"de", "de_DE.UTF-8"},
        {"ja", "ja_JP.UTF-8"},
        {"zh", "zh_CN.UTF-8"},
        {"ru", "ru_RU.UTF-8"},
        {"pt", "pt_BR.UTF-8"},
        {"ko", "ko_KR.UTF-8"},
    };

    auto it = localeMap.find(shortCode);
    if (it != localeMap.end())
        return ostd::String(it->second.c_str());

    // Fallback: try shortCode + _US.UTF-8 or just shortCode
    return ostd::String((shortCode + "_US.UTF-8").c_str());
}
