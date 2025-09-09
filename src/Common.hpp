#pragma once

#define sf_color(ostd_color) sf::Color { ostd_color.r, ostd_color.g, ostd_color.b, ostd_color.a }

class Common
{
	public:
		static double getCurrentTIme_ns(void);
};