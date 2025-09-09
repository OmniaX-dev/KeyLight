#include "Common.hpp"

#include <ostd/Utils.hpp>

double Common::getCurrentTIme_ns(void)
{
	return std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}