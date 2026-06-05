#include "core/colorspace_converter.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

// Clamp value to [0, 255] range
inline uint8_t clamp255(int v) {
  return static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
}

inline uint8_t clamp255f(float v) {
  return static_cast<uint8_t>(v < 0.0f ? 0 : (v > 255.0f ? 255 : v));
}

// Otsu thresholding on luminance values
uint8_t computeOtsuThreshold(const ImageData& src) {
  const int w = src.width();
  const int h = src.height();
  const int total = w * h;
  int histogram[256] = {};

  // Build histogram from luminance
  if (src.format() == ImageData::PixelFormat::kGray8) {
    for (int y = 0; y < h; ++y) {
      const uint8_t* row = src.pixel(0, y);
      for (int x = 0; x < w; ++x) {
        histogram[row[x]]++;
      }
    }
  } else {
    const int ch = src.channels();
    for (int y = 0; y < h; ++y) {
      const uint8_t* row = src.pixel(0, y);
      for (int x = 0; x < w; ++x) {
        const uint8_t* p = row + x * ch;
        // luminance: 0.299*R + 0.587*G + 0.114*B
        uint8_t lum = clamp255f(0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
        histogram[lum]++;
      }
    }
  }

  // Otsu's method: find threshold that minimizes intra-class variance
  float sum = 0;
  for (int i = 0; i < 256; ++i) {
    sum += i * histogram[i];
  }

  float sum_b = 0;
  int w_b = 0;
  int w_f = 0;
  float max_var = 0;
  uint8_t threshold = 128;

  for (int t = 0; t < 256; ++t) {
    w_b += histogram[t];
    if (w_b == 0) continue;
    w_f = total - w_b;
    if (w_f == 0) break;

    sum_b += t * histogram[t];
    float m_b = sum_b / w_b;
    float m_f = (sum - sum_b) / w_f;
    float var_between = static_cast<float>(w_b) * w_f * (m_b - m_f) * (m_b - m_f);

    if (var_between > max_var) {
      max_var = var_between;
      threshold = static_cast<uint8_t>(t);
    }
  }

  return threshold;
}

// RGB to HSV per pixel
void rgbToHsv(uint8_t r, uint8_t g, uint8_t b, uint8_t& h, uint8_t& s, uint8_t& v) {
  float rf = r / 255.0f;
  float gf = g / 255.0f;
  float bf = b / 255.0f;

  float max_val = std::max({rf, gf, bf});
  float min_val = std::min({rf, gf, bf});
  float delta = max_val - min_val;

  // V = max (0-255)
  v = static_cast<uint8_t>(max_val * 255.0f);

  // S = delta / max (0-255)
  if (max_val < 0.001f) {
    s = 0;
    h = 0;
  } else {
    s = static_cast<uint8_t>((delta / max_val) * 255.0f);
    float hue = 0;
    if (delta > 0.001f) {
      if (max_val == rf) {
        hue = 60.0f * (std::fmod((gf - bf) / delta, 6.0f));
      } else if (max_val == gf) {
        hue = 60.0f * (((bf - rf) / delta) + 2.0f);
      } else {
        hue = 60.0f * (((rf - gf) / delta) + 4.0f);
      }
      if (hue < 0) hue += 360.0f;
    }
    // H in [0,360] scaled to [0,180] (OpenCV convention for 8-bit)
    h = static_cast<uint8_t>(hue / 2.0f);
  }
}

// RGB to CIELAB via XYZ (D65 illuminant)
void rgbToLab(uint8_t r, uint8_t g, uint8_t b, uint8_t& l, uint8_t& a, uint8_t& bb) {
  // Normalize to [0,1]
  float rf = r / 255.0f;
  float gf = g / 255.0f;
  float bf_ = b / 255.0f;

  // sRGB linearization (gamma correction removal)
  auto linearize = [](float c) -> float {
    if (c <= 0.04045f) return c / 12.92f;
    return std::pow((c + 0.055f) / 1.055f, 2.4f);
  };

  float r_lin = linearize(rf);
  float g_lin = linearize(gf);
  float b_lin = linearize(bf_);

  // RGB → XYZ (D65, sRGB matrix)
  float x = 0.4124564f * r_lin + 0.3575761f * g_lin + 0.1804375f * b_lin;
  float y = 0.2126729f * r_lin + 0.7151522f * g_lin + 0.0721750f * b_lin;
  float z = 0.0193339f * r_lin + 0.1191920f * g_lin + 0.9503041f * b_lin;

  // D65 reference white point
  const float xn = 0.95047f;
  const float yn = 1.00000f;
  const float zn = 1.08883f;

  auto f = [](float t) -> float {
    const float delta = 6.0f / 29.0f;
    if (t > delta * delta * delta) return std::cbrt(t);
    return t / (3.0f * delta * delta) + 4.0f / 29.0f;
  };

  float fy = f(y / yn);
  float fx = f(x / xn);
  float fz = f(z / zn);

  // L* in [0,100], scale to [0,255]
  l = clamp255f((116.0f * fy - 16.0f) * 2.55f);
  // a* and b* range roughly [-128, 128], offset by 128
  a  = clamp255f(500.0f * (fx - fy) + 128.0f);
  bb = clamp255f(200.0f * (fy - fz) + 128.0f);
}

ImageData convertToHSV(const ImageData& src) {
  ImageData dst;
  if (!dst.create(src.width(), src.height(), ImageData::PixelFormat::kRGB24)) {
    return dst;
  }
  const int ch = src.channels();
  for (int y = 0; y < src.height(); ++y) {
    const uint8_t* src_row = src.pixel(0, y);
    uint8_t* dst_row = dst.pixel(0, y);
    for (int x = 0; x < src.width(); ++x) {
      const uint8_t* p = src_row + x * ch;
      uint8_t* out = dst_row + x * 3;
      rgbToHsv(p[0], p[1], p[2], out[0], out[1], out[2]);
    }
  }
  return dst;
}

ImageData convertToLAB(const ImageData& src) {
  ImageData dst;
  if (!dst.create(src.width(), src.height(), ImageData::PixelFormat::kRGB24)) {
    return dst;
  }
  const int ch = src.channels();
  for (int y = 0; y < src.height(); ++y) {
    const uint8_t* src_row = src.pixel(0, y);
    uint8_t* dst_row = dst.pixel(0, y);
    for (int x = 0; x < src.width(); ++x) {
      const uint8_t* p = src_row + x * ch;
      uint8_t* out = dst_row + x * 3;
      rgbToLab(p[0], p[1], p[2], out[0], out[1], out[2]);
    }
  }
  return dst;
}

ImageData convertToGray(const ImageData& src) {
  ImageData dst;
  if (!dst.create(src.width(), src.height(), ImageData::PixelFormat::kGray8)) {
    return dst;
  }
  if (src.format() == ImageData::PixelFormat::kGray8) {
    // Already grayscale, just copy
    for (int y = 0; y < src.height(); ++y) {
      const uint8_t* src_row = src.pixel(0, y);
      uint8_t* dst_row = dst.pixel(0, y);
      std::copy(src_row, src_row + src.width(), dst_row);
    }
  } else {
    const int ch = src.channels();
    for (int y = 0; y < src.height(); ++y) {
      const uint8_t* src_row = src.pixel(0, y);
      uint8_t* dst_row = dst.pixel(0, y);
      for (int x = 0; x < src.width(); ++x) {
        const uint8_t* p = src_row + x * ch;
        dst_row[x] = clamp255f(0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
      }
    }
  }
  return dst;
}

ImageData convertToBinary(const ImageData& src) {
  ImageData dst;
  if (!dst.create(src.width(), src.height(), ImageData::PixelFormat::kGray8)) {
    return dst;
  }
  uint8_t threshold = computeOtsuThreshold(src);
  if (src.format() == ImageData::PixelFormat::kGray8) {
    for (int y = 0; y < src.height(); ++y) {
      const uint8_t* src_row = src.pixel(0, y);
      uint8_t* dst_row = dst.pixel(0, y);
      for (int x = 0; x < src.width(); ++x) {
        dst_row[x] = (src_row[x] >= threshold) ? 255 : 0;
      }
    }
  } else {
    const int ch = src.channels();
    for (int y = 0; y < src.height(); ++y) {
      const uint8_t* src_row = src.pixel(0, y);
      uint8_t* dst_row = dst.pixel(0, y);
      for (int x = 0; x < src.width(); ++x) {
        const uint8_t* p = src_row + x * ch;
        uint8_t lum = clamp255f(0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
        dst_row[x] = (lum >= threshold) ? 255 : 0;
      }
    }
  }
  return dst;
}

}  // namespace

ImageData convertColorSpace(const ImageData& src, ColorSpace target) {
  if (!src.is_valid()) {
    return ImageData();
  }

  switch (target) {
    case ColorSpace::kRGB: {
      // Direct copy
      ImageData dst;
      dst.copy_from(src);
      return dst;
    }
    case ColorSpace::kHSV:
      return convertToHSV(src);
    case ColorSpace::kLAB:
      return convertToLAB(src);
    case ColorSpace::kGray:
      return convertToGray(src);
    case ColorSpace::kBinary:
      return convertToBinary(src);
  }
  return ImageData();
}
