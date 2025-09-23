#pragma once
#include "utils/rtweekend.h"

class interval
{
public:
	double min_, max_;

	interval() : min_(+infinity), max_(-infinity)
	{
	} // Default interval is empty

	interval(const double min, const double max) : min_(min), max_(max)
	{
	}

	interval(const interval& a, const interval& b)
	{
		// Create the interval tightly enclosing the two input intervals.
		min_ = a.min_ <= b.min_ ? a.min_ : b.min_;
		max_ = a.max_ >= b.max_ ? a.max_ : b.max_;
	}

	double size() const
	{
		return max_ - min_;
	}

	bool contains(const double x) const
	{
		return min_ <= x && x <= max_;
	}

	bool surrounds(const double x) const
	{
		return min_ < x && x < max_;
	}

	double clamp(const double x) const
	{
		if (x < min_) return min_;
		if (x > max_) return max_;
		return x;
	}

	interval expand(double delta) const
	{
		const auto padding = delta / 2;
		return interval(min_ - padding, max_ + padding);
	}


	static const interval empty, universe;
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);


inline interval operator+(const interval& ival, double displacement)
{
	return interval(ival.min_ + displacement, ival.max_ + displacement);
}

inline interval operator+(double displacement, const interval& ival)
{
	return ival + displacement;
}
