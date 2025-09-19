#include <fstream>
#include <iostream>
#include <Windows.h>
#include "color.h"
#include "vec3.h"
#include "ProjectUtil.h"
#include "ray.h"


static double hit_sphere(const point3& center, const double radius, const ray& r)
{
	vec3 oc = center - r.origin();
	auto a = r.direction().length_squared();
	auto h = dot(r.direction(), oc);
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = h * h - a * c;

	if (discriminant < 0) return -1.0;

	return (h - std::sqrt(discriminant)) / a;
}


/// <summary>
/// 默认返回黑色
/// </summary>
/// <param name="r"></param>
/// <returns></returns>
static color ray_color(const ray& r)
{
	auto t = hit_sphere(point3(0, 0, -1), 0.5, r);
	if (t > 0.0)
	{
		vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
		return 0.5 * color(N.x() + 1, N.y() + 1, N.z() + 1);
	}


	const vec3 unit_direction = unit_vector(r.direction());
	auto a = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}


int main()
{
	// basic setting
	OutputDebugString(L"hello world");
	OutputDebugStringA(get_project_root_dir().c_str());
	std::ofstream out(get_project_root_dir() + "\\output.ppm"); // 输出到文件


	// Image
	constexpr int image_width = 400; // 图片像素
	constexpr auto aspect_ratio = 16.0 / 9.0;

	// Calculate the image height, and ensure that it's at least 1.
	int image_height = static_cast<int>(image_width / aspect_ratio);
	image_height = (image_height < 1) ? 1 : image_height;

	// Camera

	constexpr auto focal_length = 1.0;
	constexpr auto viewport_height = 2.0;
	const auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);
	const auto camera_center = point3(0, 0, 0);

	// Calculate the vectors across the horizontal and down the vertical viewport edges.
	const auto viewport_u = vec3(viewport_width, 0, 0);
	const auto viewport_v = vec3(0, -viewport_height, 0);

	// Calculate the horizontal and vertical delta vectors from pixel to pixel.
	auto pixel_delta_u = viewport_u / image_width;
	auto pixel_delta_v = viewport_v / image_height;

	// Calculate the location of the upper left pixel.
	const auto viewport_upper_left = camera_center
		- vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
	auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);


	// Render
	out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	for (int j = 0; j < image_height; j++)
	{
		std::clog << "\rScanline's remaining: " << (image_height - j) << ' ' << std::flush;
		for (int i = 0; i < image_width; i++)
		{
			auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
			auto ray_direction = pixel_center - camera_center;
			ray r(camera_center, ray_direction);

			color pixel_color = ray_color(r);
			write_color(out, pixel_color);
		}
	}
	std::clog << "\rDone.                 \n";
}
