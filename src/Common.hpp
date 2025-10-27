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

#define sf_color(ostd_color) sf::Color { ostd_color.r, ostd_color.g, ostd_color.b, ostd_color.a }
#define color_to_glsl(ostd_color) sf::Glsl::Vec4(ostd_color.r / 255.0f, ostd_color.g / 255.0f, ostd_color.b / 255.0f, ostd_color.a / 255.0f)
#define sf_intRect(ostd_rect) sf::IntRect { { (int)ostd_rect.x, (int)ostd_rect.y }, { (int)ostd_rect.w, (int)ostd_rect.h } }

class Common
{
	public:
		static double getCurrentTIme_ns(void);

	public:
	    inline static float guiScaleX { 1.0f };
	    inline static float guiScaleY { 1.0f };
};
