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

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics.hpp>

#include <ostd/Signals.hpp>
#include <ostd/String.hpp>
#include <ostd/Defines.hpp>

#define sf_color(ostd_color) sf::Color { ostd_color.r, ostd_color.g, ostd_color.b, ostd_color.a }
#define color_to_glsl(ostd_color) sf::Glsl::Vec4(ostd_color.r / 255.0f, ostd_color.g / 255.0f, ostd_color.b / 255.0f, ostd_color.a / 255.0f)
#define sf_intRect(ostd_rect) sf::IntRect { { (int)ostd_rect.x, (int)ostd_rect.y }, { (int)ostd_rect.w, (int)ostd_rect.h } }

class Common
{
	public:
		static double getCurrentTIme_ns(void);
		static void ensureDirectory(const ostd::String& path);
		static void deleteDirectory(const ostd::String& path);
		static double percentage(double n, double max);
		sf::VertexArray getMusicWaveForm(const ostd::String& filePath, int32_t windowHeight);
		static ostd::String secondsToFormattedString(int32_t totalSeconds);

		inline static const sf::Clock& getAppClock(void) { return s_appClock; }
		inline static bool wasSIGINTTriggered(void) { return s_sigint_triggered; }
		inline static float scaleX(float value) { return value * guiScaleX; }
		inline static float scaleY(float value) { return value * guiScaleY; }
		inline static float scaleXY(float value) { return value * 0.5f * (guiScaleX + guiScaleY); }

	public:
	    inline static float guiScaleX { 1.0f };
	    inline static float guiScaleY { 1.0f };

	#ifdef BUILD_CONFIG_DEBUG
		inline static constexpr bool IsDebug = true;
	#else
		inline static constexpr bool IsDebug = false;
	#endif

		inline static bool s_sigint_triggered { false };

	private:
		inline static sf::Clock s_appClock;

	public:
		inline static const uint32_t SigIntSignal = ostd::SignalHandler::newCustomSignal(0xBB00000);
};
