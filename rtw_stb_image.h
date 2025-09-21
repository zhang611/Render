#pragma once


// Disable strict warnings for this header from the Microsoft Visual C++ compiler.
#ifdef _MSC_VER
#pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#include <cstdlib>
#include <iostream>

class rtw_image
{
public:
	rtw_image() = default;

	explicit rtw_image(const char* image_filename)
	{
		const auto filename = std::string(image_filename);
		if (load(filename)) return;
		std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
	}

	~rtw_image()
	{
		delete[] bdata_;
		STBI_FREE(fdata_);
	}

	bool load(const std::string& filename)
	{
		// Loads the linear (gamma=1) image data from the given file name. Returns true if the
		// load succeeded. The resulting data buffer contains the three [0.0, 1.0]
		// floating-point values for the first pixel (red, then green, then blue). Pixels are
		// contiguous, going left to right for the width of the image, followed by the next row
		// below, for the full height of the image.

		auto n = bytes_per_pixel; // Dummy out parameter: original components per pixel
		fdata_ = stbi_loadf(filename.c_str(), &image_width_, &image_height_, &n, bytes_per_pixel);
		if (fdata_ == nullptr) return false;

		bytes_per_scanline_ = image_width_ * bytes_per_pixel;
		convert_to_bytes();
		return true;
	}

	int width() const { return (fdata_ == nullptr) ? 0 : image_width_; }
	int height() const { return (fdata_ == nullptr) ? 0 : image_height_; }

	const unsigned char* pixel_data(int x, int y) const
	{
		// Return the address of the three RGB bytes of the pixel at x,y. If there is no image
		// data, returns magenta.
		static unsigned char magenta[] = {255, 0, 255};
		if (bdata_ == nullptr) return magenta;

		x = clamp(x, 0, image_width_);
		y = clamp(y, 0, image_height_);

		return bdata_ + y * bytes_per_scanline_ + x * bytes_per_pixel;
	}

private:
	const int bytes_per_pixel = 3;
	float* fdata_ = nullptr; // Linear floating point pixel data
	unsigned char* bdata_ = nullptr; // Linear 8-bit pixel data
	int image_width_ = 0; // Loaded image width
	int image_height_ = 0; // Loaded image height
	int bytes_per_scanline_ = 0;

	static int clamp(const int x, const int low, const int high)
	{
		// Return the value clamped to the range [low, high).
		if (x < low) return low;
		if (x < high) return x;
		return high - 1;
	}

	static unsigned char float_to_byte(const float value)
	{
		if (value <= 0.0) return 0;
		if (1.0 <= value) return 255;
		return static_cast<unsigned char>(256.0 * value);
	}

	void convert_to_bytes()
	{
		// Convert the linear floating point pixel data to bytes, storing the resulting byte
		// data in the `bdata` member.

		const int total_bytes = image_width_ * image_height_ * bytes_per_pixel;
		bdata_ = new unsigned char[total_bytes];

		// Iterate through all pixel components, converting from [0.0, 1.0] float values to
		// unsigned [0, 255] byte values.

		auto* bptr = bdata_;
		auto* fptr = fdata_;
		for (auto i = 0; i < total_bytes; i++, fptr++, bptr++)
			*bptr = float_to_byte(*fptr);
	}
};

// Restore MSVC compiler warnings
#ifdef _MSC_VER
#pragma warning (pop)
#endif
