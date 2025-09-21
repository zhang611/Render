#pragma once
#include <algorithm>

#include "interval.h"
#include "ray.h"
#include "vec3.h"

class aabb
{
public:
	interval x, y, z;

	aabb() = default; // The default AABB is empty, since intervals are empty by default.

	aabb(const interval& x, const interval& y, const interval& z)
		: x(x), y(y), z(z)
	{
		pad_to_minimums();  
	}

	aabb(const point3& a, const point3& b)
	{
		// Treat the two points a and b as extrema for the bounding box, so we don't require a
		// particular minimum/maximum coordinate order.

		x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
		y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
		z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);

		pad_to_minimums();

	}

	aabb(const aabb& box0, const aabb& box1)
	{
		x = interval(box0.x, box1.x);
		y = interval(box0.y, box1.y);
		z = interval(box0.z, box1.z);
	}

	const interval& axis_interval(int n) const
	{
		if (n == 1) return y;
		if (n == 2) return z;
		return x;
	}

	bool hit(const ray& r, interval ray_t) const
	{
		const point3& ray_orig = r.origin();
		const vec3& ray_dir = r.direction();

		for (int axis = 0; axis < 3; axis++)
		{
			const interval& ax = axis_interval(axis);
			const double adinv = 1.0 / ray_dir[axis];

			auto t0 = (ax.min_ - ray_orig[axis]) * adinv; // 进入时间
			auto t1 = (ax.max_ - ray_orig[axis]) * adinv; // 出去时间

			if (t0 < t1)
			{
				if (t0 > ray_t.min_) ray_t.min_ = t0; // 进入时间取最大值
				if (t1 < ray_t.max_) ray_t.max_ = t1; // 出去时间取最小值
			}
			else // 光纤在反方向
			{
				if (t1 > ray_t.min_) ray_t.min_ = t1;
				if (t0 < ray_t.max_) ray_t.max_ = t0;
			}

			if (ray_t.max_ <= ray_t.min_) // 没有交集
				return false;
		}
		return true;
	}

	int longest_axis() const
	{
		// Returns the index of the longest axis of the bounding box.
		if (x.size() > y.size()) return x.size() > z.size() ? 0 : 2;
		return y.size() > z.size() ? 1 : 2;
	}

	static const aabb empty, universe;


private:

	/// <summary>
	/// 保证 aabb 的体积不为0
	/// </summary>
	void pad_to_minimums() {
		// Adjust the AABB so that no side is narrower than some delta, padding if necessary.
		constexpr double delta = 0.0001;
		if (x.size() < delta) x = x.expand(delta);
		if (y.size() < delta) y = y.expand(delta);
		if (z.size() < delta) z = z.expand(delta);
	}
};


const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);
