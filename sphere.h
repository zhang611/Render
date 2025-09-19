#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable
{
public:
	sphere(const point3& center, double radius) : center_(center), radius_(std::fmax(0, radius))
	{
	}

	bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const override
	{
		vec3 oc = center_ - r.origin();
		auto a = r.direction().length_squared();
		auto h = dot(r.direction(), oc);
		auto c = oc.length_squared() - radius_ * radius_;

		auto discriminant = h * h - a * c;
		if (discriminant < 0) return false;

		const auto sqrt_d = std::sqrt(discriminant);

		// Find the nearest root that lies in the acceptable range.
		auto root = (h - sqrt_d) / a;
		if (root <= ray_tmin || ray_tmax <= root)
		{
			root = (h + sqrt_d) / a;
			if (root <= ray_tmin || ray_tmax <= root)
				return false;
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		rec.normal = (rec.p - center_) / radius_;

		return true;
	}

private:
	point3 center_;
	double radius_;
};
