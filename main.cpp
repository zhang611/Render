#include "rtweekend.h"
#include <fstream>
#include <Windows.h>

#include "camera.h"
#include "hittable_list.h"
#include "ProjectUtil.h"
#include "sphere.h"


int main()
{
	// basic setting
	OutputDebugString(L"hello world");
	OutputDebugStringA(get_project_root_dir().c_str());

	// World
	hittable_list world;
	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));


	// Camera
	camera cam;
	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 400;

	cam.samples_per_pixel = 100;


	cam.render(world);
}
