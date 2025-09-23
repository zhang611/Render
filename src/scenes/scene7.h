#pragma once
#include <string>

#include "entity/hittable_list.h"
#include "entity/material.h"
#include "entity/sphere.h"
#include "entity/texture.h"
#include "render/camera.h"
#include "utils/ProjectUtil.h"


/// <summary>
/// µØÇò
/// </summary>
inline void scene()
{
	const std::string filename = get_project_root_dir() + "\\earthmap.jpg";

	auto earth_texture = make_shared<image_texture>(filename.c_str());
	auto earth_surface = make_shared<lambertian>(earth_texture);
	const auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

	camera cam;

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 400;
	cam.samples_per_pixel = 100;
	cam.max_depth = 50;

	cam.vfov = 20;
	cam.lookfrom = point3(0, 0, 12);
	cam.lookat = point3(0, 0, 0);
	cam.vup = vec3(0, 1, 0);

	cam.defocus_angle = 0;

	cam.render(hittable_list(globe), "7");
}
