#pragma once

#define sf_color(ostd_color) sf::Color { ostd_color.r, ostd_color.g, ostd_color.b, ostd_color.a }
#define color_to_glsl(ostd_color) sf::Glsl::Vec4(ostd_color.r / 255.0f, ostd_color.g / 255.0f, ostd_color.b / 255.0f, ostd_color.a / 255.0f)
#define sf_intRect(ostd_rect) sf::IntRect { { (int)ostd_rect.x, (int)ostd_rect.y }, { (int)ostd_rect.w, (int)ostd_rect.h } }

class Common
{
	public:
		static double getCurrentTIme_ns(void);
};