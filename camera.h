#pragma once
#include "hittable.h"
#include "ProjectUtil.h"
#include "material.h"

class camera
{
public:
	double aspect_ratio = 1.0; // Ratio of image width over height
	int image_width = 100; // Rendered image width in pixel count
	int samples_per_pixel = 10; // Count of random samples for each pixel
	int max_depth = 10; // Maximum number of ray bounces into scene


	void render(const hittable& world)
	{
		std::ofstream out(get_project_root_dir() + "\\output.ppm"); // ������ļ�

		initialize();

		out << "P3\n" << image_width << ' ' << image_height << "\n255\n";

		for (int j = 0; j < image_height; j++)
		{
			std::clog << "\rScanline's remaining: " << (image_height - j) << ' ' << std::flush;
			for (int i = 0; i < image_width; i++)
			{
				color pixel_color(0, 0, 0);
				for (int sample = 0; sample < samples_per_pixel; sample++)
				{
					ray r = get_ray(i, j);
					pixel_color += ray_color(r, max_depth, world);
				}
				write_color(out, pixel_samples_scale * pixel_color);
			}
		}

		std::clog << "\rDone.                 \n";
	}

private:
	int image_height = 0; // Rendered image height
	double pixel_samples_scale = 0; // Color scale factor for a sum of pixel samples
	point3 center; // Camera center
	point3 pixel00_loc; // Location of pixel 0, 0
	vec3 pixel_delta_u; // Offset to pixel to the right
	vec3 pixel_delta_v; // Offset to pixel below

	void initialize()
	{
		image_height = static_cast<int>(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		pixel_samples_scale = 1.0 / samples_per_pixel;

		center = point3(0, 0, 0);

		// Determine viewport dimensions.
		auto focal_length = 1.0;
		auto viewport_height = 2.0;
		auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		auto viewport_u = vec3(viewport_width, 0, 0);
		auto viewport_v = vec3(0, -viewport_height, 0);

		// Calculate the horizontal and vertical delta vectors from pixel to pixel.
		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;

		// Calculate the location of the upper left pixel.
		auto viewport_upper_left =
			center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
	}

	static color ray_color(const ray& r, int depth, const hittable& world)
	{
		if (depth <= 0) return {0, 0, 0};
		hit_record rec;

		if (world.hit(r, interval(0.001, infinity), rec))
		{

			ray scattered;
			color attenuation;
			if (rec.mat->scatter(r, rec, attenuation, scattered))
				return attenuation * ray_color(scattered, depth - 1, world);
			return {0, 0, 0};

			// // vec3 direction = random_on_hemisphere(rec.normal);    // ������
			// vec3 direction = rec.normal + random_unit_vector();      // �����ط��䣬����ʵ
			// return 0.7 * ray_color(ray(rec.p, direction), depth - 1, world);
		}

		vec3 unit_direction = unit_vector(r.direction());
		auto a = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
	}

	ray get_ray(const int i, const int j) const
	{
		// Construct a camera ray originating from the origin and directed at randomly sampled
		// point around the pixel location i, j.

		const auto offset = sample_square();
		const auto pixel_sample = pixel00_loc
			+ (i + offset.x()) * pixel_delta_u
			+ (j + offset.y()) * pixel_delta_v;

		const auto ray_origin = center;
		const auto ray_direction = pixel_sample - ray_origin;

		return ray(ray_origin, ray_direction);
	}

	static vec3 sample_square()
	{
		// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
		return {random_double() - 0.5, random_double() - 0.5, 0};
	}
};
