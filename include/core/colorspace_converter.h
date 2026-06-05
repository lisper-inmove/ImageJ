#ifndef IMAGEJ_CORE_COLORSPACE_CONVERTER_H_
#define IMAGEJ_CORE_COLORSPACE_CONVERTER_H_

#include "image_data.h"

enum class ColorSpace { kRGB, kHSV, kLAB, kGray, kBinary };

// Converts src to target color space.
// - kRGB: returns copy of src
// - kHSV/kLAB: returns kRGB24 with H/L in R, S/A in G, V/B in B
// - kGray: returns kGray8 with luminance values
// - kBinary: returns kGray8 with 0 or 255
ImageData convertColorSpace(const ImageData& src, ColorSpace target);

#endif  // IMAGEJ_CORE_COLORSPACE_CONVERTER_H_
