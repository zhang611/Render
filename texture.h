#pragma once
#include <cmath>
#include <memory>

#include "color.h"
#include "perlin.h"
#include "rtw_stb_image.h"

class texture
{
public:
	virtual ~texture() = default;

	virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color final : public texture
{
public:
	explicit solid_color(const color& albedo) : albedo_(albedo)
	{
	}

	solid_color(const double red, const double green, const double blue) : solid_color(color(red, green, blue))
	{
	}

	color value(double u, double v, const point3& p) const override
	{
		return albedo_;
	}

private:
	color albedo_;
};


/// <summary>
/// Íø¸ñÎÆÀí
/// </summary>
class checker_texture : public texture
{
public:
	checker_texture(const double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
		: inv_scale_(1.0 / scale), even_(std::move(even)), odd_(std::move(odd))
	{
	}

	checker_texture(const double scale, const color& c1, const color& c2)
		: checker_texture(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2))
	{
	}

	color value(const double u, const double v, const point3& p) const override
	{
		const auto xInteger = static_cast<int>(std::floor(inv_scale_ * p.x()));
		const auto yInteger = static_cast<int>(std::floor(inv_scale_ * p.y()));
		const auto zInteger = static_cast<int>(std::floor(inv_scale_ * p.z()));

		const bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

		return isEven ? even_->value(u, v, p) : odd_->value(u, v, p);
	}

private:
	double inv_scale_;
	std::shared_ptr<texture> even_;
	std::shared_ptr<texture> odd_;
};


class image_texture : public texture
{
public:
	image_texture(const char* filename) : image_(filename)
	{
	}

	color value(double u, double v, const point3& p) const override
	{
		// If we have no texture data, then return solid cyan as a debugging aid.
		if (image_.height() <= 0) return {0, 1, 1};

		// Clamp input texture coordinates to [0,1] x [1,0]
		u = interval(0, 1).clamp(u);
		v = 1.0 - interval(0, 1).clamp(v); // Flip V to image coordinates

		const auto i = static_cast<int>(u * image_.width());
		const auto j = static_cast<int>(v * image_.height());
		const auto pixel = image_.pixel_data(i, j);

		constexpr auto color_scale = 1.0 / 255.0;
		return {color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]};
	}

private:
	rtw_image image_;
};


class noise_texture final : public texture
{
public:
	noise_texture(double scale) : scale_(scale)
	{
	}

	color value(double u, double v, const point3& p) const override
	{
		// return color(1, 1, 1) * noise_.turb(p, 7);
		return color(.5, .5, .5) * (1 + std::sin(scale_ * p.z() + 10 * noise_.turb(p, 7)));

	}

private:
	perlin noise_;
	double scale_;
};
