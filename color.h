#pragma once
#include "interval.h"
#include "vec3.h"

using color = vec3;

inline void write_color(std::ostream& out, const color& pixel_color)
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	// Translate the [0,1] component values to the byte range [0,255].
	static const interval intensity(0.000, 0.999);
	const int r_byte = static_cast<int>(256 * intensity.clamp(r));
	const int g_byte = static_cast<int>(256 * intensity.clamp(g));
	const int b_byte = static_cast<int>(256 * intensity.clamp(b));


	// Write out the pixel color components.
	out << r_byte << ' ' << g_byte << ' ' << b_byte << '\n';
}
