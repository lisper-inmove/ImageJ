#ifndef IMAGE_DATA_H_
#define IMAGE_DATA_H_

#include <cstdint>
#include <vector>

class ImageData {
 public:
  // Pixel format enumeration
  enum class PixelFormat {
    kUnknown = 0,
    kGray8,      // 8-bit grayscale
    kRGB24,      // 24-bit RGB
    kRGBA32,     // 32-bit RGBA
  };

  // Construction and destruction
  ImageData();
  ImageData(int width, int height, PixelFormat format);
  ~ImageData();

  // Basic information
  int width() const;
  int height() const;
  PixelFormat format() const;
  int channels() const;
  size_t byte_count() const;
  bool is_valid() const;

  // Pixel data access
  const uint8_t* data() const;
  uint8_t* data();
  uint8_t* pixel(int x, int y);

  // Image operations
  bool create(int width, int height, PixelFormat format);
  void clear();
  bool copy_from(const ImageData& other);

  // Static utility methods
  static int calculate_stride(int width, PixelFormat format);
  static int calculate_byte_count(int width, int height, PixelFormat format);

 private:
  int width_;
  int height_;
  PixelFormat format_;
  std::vector<uint8_t> data_;
};

#endif  // IMAGE_DATA_H_