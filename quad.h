#pragma once
#include <utility>

#include "hittable.h"

class quad : public hittable
{
public:
	quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
		: q_(Q), u_(u), v_(v), mat_(std::move(mat))
	{
		auto n = cross(u, v);
		normal_ = unit_vector(n);
		d_ = dot(normal_, Q);
		w_ = n / dot(n, n);

		quad::set_bounding_box();
	}

	virtual void set_bounding_box()
	{
		// Compute the bounding box of all four vertices.
		auto bbox_diagonal1 = aabb(q_, q_ + u_ + v_);
		auto bbox_diagonal2 = aabb(q_ + u_, q_ + v_);
		bbox_ = aabb(bbox_diagonal1, bbox_diagonal2);
	}

	aabb bounding_box() const override { return bbox_; }

	bool hit(const ray& r, const interval ray_t, hit_record& rec) const override
	{
		const auto denom = dot(normal_, r.direction()); // 算 t 的分母

		// No hit if the ray is parallel to the plane.  光和平面平行，没有交点
		if (std::fabs(denom) < 1e-8) return false;

		// Return false if the hit point parameter t is outside the ray interval.
		auto t = (d_ - dot(normal_, r.origin())) / denom;
		if (!ray_t.contains(t))
			return false;

		const auto intersection = r.at(t); // 交点


		const vec3 planar_hitpt_vector = intersection - q_;
		const auto alpha = dot(w_, cross(planar_hitpt_vector, v_));
		const auto beta = dot(w_, cross(u_, planar_hitpt_vector));

		if (!is_interior(alpha, beta, rec)) return false;


		rec.t = t;
		rec.p = intersection;
		rec.mat = mat_;
		rec.set_face_normal(r, normal_);

		return true;
	}


	virtual bool is_interior(const double a, const double b, hit_record& rec) const
	{
		const auto unit_interval = interval(0, 1);
		// Given the hit point in plane coordinates, return false if it is outside the
		// primitive, otherwise set the hit record UV coordinates and return true.

		if (!unit_interval.contains(a) || !unit_interval.contains(b)) return false;

		rec.u = a;
		rec.v = b;
		return true;
	}

private:
	point3 q_; // 四边形原点
	vec3 u_, v_; // 四边形两个方向向量
	vec3 w_; // TODO 自己算一下，求解二维线性方程组的系数，向量叉乘自己为0

	shared_ptr<material> mat_;
	aabb bbox_;

	vec3 normal_;
	double d_;
};
