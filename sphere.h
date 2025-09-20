#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable
{
public:
	sphere(const point3& center, double radius) : center_(center), radius_(std::fmax(0, radius))
	{
	}

	sphere(const point3& center, double radius, shared_ptr<material> mat)
		: center_(center), radius_(std::fmax(0, radius)), mat_(mat)
	{
	}


	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
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
		if (!ray_t.surrounds(root))
		{
			root = (h + sqrt_d) / a;
			if (!ray_t.surrounds(root)) return false;
		}

		rec.t = root;
		rec.p = r.at(rec.t);

		vec3 outward_normal = (rec.p - center_) / radius_;
		rec.set_face_normal(r, outward_normal);
		// rec.normal = (rec.p - center_) / radius_;    // ���̶ֹ��������ʽ
		rec.mat = mat_;

		return true;
	}

private:
	point3 center_;
	double radius_;
	shared_ptr<material> mat_;
};
