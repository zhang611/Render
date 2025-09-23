#pragma once

#include <limits>
#include <memory>
#include <random>


// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

constexpr double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(const double degrees)
{
	return degrees * pi / 180.0;
}

// inline double random_double() {
//     // Returns a random real in [0,1).
//     return std::rand() / (RAND_MAX + 1.0);
// }

inline double random_double()
{
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator; // NOLINT(cert-msc51-cpp)
	return distribution(generator);
}

inline double random_double(const double min, const double max)
{
	// Returns a random real in [min_,max_).
	return min + (max - min) * random_double();
}

inline int random_int(const int min, const int max)
{
	// Returns a random integer in [min_,max_].
	return static_cast<int>(random_double(min, max + 1));
}
