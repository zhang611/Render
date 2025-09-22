#pragma once

#include "hittable.h"
#include "material.h"
#include "texture.h"

class constant_medium : public hittable
{
public:
	constant_medium(shared_ptr<hittable> boundary, const double density, const shared_ptr<texture>& tex)
		: boundary(std::move(boundary)), neg_inv_density(-1 / density),
		  phase_function(make_shared<isotropic>(tex))
	{
	}

	constant_medium(shared_ptr<hittable> boundary, const double density, const color& albedo)
		: boundary(std::move(boundary)), neg_inv_density(-1 / density),
		  phase_function(make_shared<isotropic>(albedo))
	{
	}

	// TODO: ʵ�������ײ
	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		hit_record rec1, rec2;

		if (!boundary->hit(r, interval::universe, rec1))
			return false;

		if (!boundary->hit(r, interval(rec1.t + 0.0001, infinity), rec2))
			return false;

		if (rec1.t < ray_t.min_) rec1.t = ray_t.min_;
		if (rec2.t > ray_t.max_) rec2.t = ray_t.max_;

		if (rec1.t >= rec2.t)
			return false;

		if (rec1.t < 0) rec1.t = 0;

		auto ray_length = r.direction().length();
		auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
		auto hit_distance = neg_inv_density * std::log(random_double());

		if (hit_distance > distance_inside_boundary)
			return false;

		rec.t = rec1.t + hit_distance / ray_length;
		rec.p = r.at(rec.t);

		rec.normal = vec3(1, 0, 0); // arbitrary
		rec.front_face = true; // also arbitrary
		rec.mat = phase_function;

		return true;
	}

	aabb bounding_box() const override { return boundary->bounding_box(); }

private:
	shared_ptr<hittable> boundary;
	double neg_inv_density;
	shared_ptr<material> phase_function;
};
