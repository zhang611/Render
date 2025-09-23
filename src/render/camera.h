#pragma once
#include <fstream>

#include "utils/ProjectUtil.h"


class camera
{
public:
	double aspect_ratio = 1.0; // Ratio of image width over height
	int image_width = 100; // Rendered image width in pixel count
	int samples_per_pixel = 10; // Count of random samples for each pixel
	int max_depth = 10; // Maximum number of ray bounces into scene
	color background = color(0.70, 0.80, 1.00); // Scene background color


	double vfov = 90; // Vertical view angle (field of view)
	point3 lookfrom = point3(0, 0, 0); // Point camera is looking from
	point3 lookat = point3(0, 0, -1); // Point camera is looking at
	vec3 vup = vec3(0, 1, 0); // Camera-relative "up" direction

	// ģ�⾰���Բ�̷����
	double defocus_angle = 0; // Variation angle of rays through each pixel
	double focus_dist = 10; // Distance from camera lookfrom point to plane of perfect focus

	void render(const hittable& world, const std::string& name)
	{
		initialize();

		// 帧缓冲：按行主序存储每个像素最终颜色
		std::vector<color> framebuffer(image_width * image_height, color(0, 0, 0));

		// 并行策略：按扫描行分块。块大小可调（cache 友好 + 减少竞争）。
		const int hardware_threads = max(1u, std::thread::hardware_concurrency());
		constexpr int block_lines = 4; // 每个任务处理的行数，可根据场景/CPU 调整

		std::atomic<int> next_line{0};
		std::atomic<int> lines_done{0};

		auto worker = [&]()
		{
			while (true)
			{
				int start = next_line.fetch_add(block_lines);
				if (start >= image_height) break;
				int end = min(start + block_lines, image_height);
				for (int j = start; j < end; ++j)
				{
					for (int i = 0; i < image_width; ++i)
					{
						color pixel_color(0, 0, 0);
						// 分层采样
						for (int s_j = 0; s_j < sqrt_spp; ++s_j)
						{
							for (int s_i = 0; s_i < sqrt_spp; ++s_i)
							{
								ray r = get_ray(i, j, s_i, s_j);
								pixel_color += ray_color(r, max_depth, world);
							}
						}
						framebuffer[j * image_width + i] = pixel_samples_scale * pixel_color;
					}
					lines_done.fetch_add(1);
				}
			}
		};

		std::vector<std::thread> threads;
		threads.reserve(hardware_threads);
		for (int t = 0; t < hardware_threads; ++t)
		{
			threads.emplace_back(worker);
		}

		// 简单进度输出
		while (lines_done.load() < image_height)
		{
			std::clog << "\rScanlines remaining: " << (image_height - lines_done.load()) << ' ' << std::flush;
			std::this_thread::sleep_for(std::chrono::milliseconds(250));  // 主线程每 250ms 才查询一次进度，把资源给工作线程
		}

		for (auto& th : threads) th.join();
		std::clog << "\rScanlines remaining: 0            \n";

		// 输出到文件
		const std::string filename = get_project_root_dir() + "\\output_" + name + ".ppm";
		std::ofstream out(filename);
		out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
		for (int j = 0; j < image_height; ++j)
		{
			for (int i = 0; i < image_width; ++i)
			{
				write_color(out, framebuffer[j * image_width + i]);
			}
		}

		std::clog << "Done (multithread). Output: " << filename << "\n";
	}

private:
	int image_height = 0; // Rendered image height
	double pixel_samples_scale = 0; // Color scale_ factor for a sum of pixel samples
	int sqrt_spp; // Square root of number of samples per pixel
	double recip_sqrt_spp; // 1 / sqrt_spp
	point3 center; // Camera center
	point3 pixel00_loc; // Location of pixel 0, 0
	vec3 pixel_delta_u; // Offset to pixel to the right
	vec3 pixel_delta_v; // Offset to pixel below

	/// <summary>
	/// v ���ϣ�w�����߷����෴��u ����
	/// </summary>
	vec3 u, v, w; // Camera frame basis vectors
	vec3 defocus_disk_u; // Defocus disk horizontal radius
	vec3 defocus_disk_v; // Defocus disk vertical radius

	void initialize()
	{
		image_height = static_cast<int>(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		sqrt_spp = static_cast<int>(std::sqrt(samples_per_pixel));
		pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
		recip_sqrt_spp = 1.0 / sqrt_spp;

		pixel_samples_scale = 1.0 / samples_per_pixel;

		center = lookfrom;

		// Determine viewport dimensions.
		// auto focal_length = (lookfrom - lookat).length();

		auto theta = degrees_to_radians(vfov);
		auto h = std::tan(theta / 2);
		// auto viewport_height = 2 * h * focal_length;
		auto viewport_height = 2 * h * focus_dist;

		auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

		// Calculate the u,v,w unit basis vectors for the camera coordinate frame.
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		vec3 viewport_u = viewport_width * u; // Vector across viewport horizontal edge
		vec3 viewport_v = viewport_height * -v; // Vector down viewport vertical edge

		// Calculate the horizontal and vertical delta vectors from pixel to pixel.
		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;

		// Calculate the location of the upper left pixel.
		// auto viewport_upper_left = center - (focal_length * w) - viewport_u / 2 - viewport_v / 2;
		auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;

		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

		// Calculate the camera defocus disk basis vectors.
		auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
		defocus_disk_u = u * defocus_radius;
		defocus_disk_v = v * defocus_radius;
	}

	color ray_color(const ray& r, const int depth, const hittable& world)
	{
		if (depth <= 0) return {0, 0, 0};
		hit_record rec;

		// If the ray hits nothing, return the background color.
		if (!world.hit(r, interval(0.001, infinity), rec)) return background;

		ray scattered;
		color attenuation;
		color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

		if (!rec.mat->scatter(r, rec, attenuation, scattered)) return color_from_emission;
		color color_from_scatter = attenuation * ray_color(scattered, depth - 1, world);
		return color_from_emission + color_from_scatter;
	}

	ray get_ray(const int i, const int j) const
	{
		// Construct a camera ray originating from the origin and directed at randomly sampled
		// point around the pixel location i, j.

		// Construct a camera ray originating from the defocus disk and directed at a randomly
		// sampled point around the pixel location i, j.

		const auto offset = sample_square();
		const auto pixel_sample = pixel00_loc
			+ (i + offset.x()) * pixel_delta_u
			+ (j + offset.y()) * pixel_delta_v;

		// const auto ray_origin = center;
		auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();

		const auto ray_direction = pixel_sample - ray_origin;

		auto ray_time = random_double();
		return ray(ray_origin, ray_direction, ray_time);
	}

	ray get_ray(int i, int j, int s_i, int s_j) const
	{
		// Construct a camera ray originating from the defocus disk and directed at a randomly
		// sampled point around the pixel location i, j for stratified sample square s_i, s_j.

		auto offset = sample_square_stratified(s_i, s_j);
		auto pixel_sample = pixel00_loc
			+ ((i + offset.x()) * pixel_delta_u)
			+ ((j + offset.y()) * pixel_delta_v);

		auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		auto ray_time = random_double();

		return ray(ray_origin, ray_direction, ray_time);
	}


	// �����㷨����ƽ��ֳ� sqrt_spp x sqrt_spp ��С���ӣ���ÿ��С�������������
	// �����˴��������ʱ���ܳ��ֵĲ�������ڼ��л���ڷ�ɢ�����⣬Ч������
	vec3 sample_square_stratified(const int s_i, const int s_j) const
	{
		// Returns the vector to a random point in the square sub-pixel specified by grid
		// indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

		auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
		auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

		return vec3(px, py, 0);
	}

	static vec3 sample_square()
	{
		// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
		return {random_double() - 0.5, random_double() - 0.5, 0};
	}

	point3 defocus_disk_sample() const
	{
		// Returns a random point in the camera defocus disk.
		auto p = random_in_unit_disk();
		return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
	}
};
