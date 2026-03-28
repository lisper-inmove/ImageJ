#ifndef IMAGEJ_CORE_IMAGE_DATA_H_
#define IMAGEJ_CORE_IMAGE_DATA_H_

#include <cstddef>
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
  explicit ImageData(int width, int height, PixelFormat format);
  ~ImageData();

  // Copy and move operations (Rule of Five)
  ImageData(const ImageData&);
  ImageData& operator=(const ImageData&);
  ImageData(ImageData&&);
  ImageData& operator=(ImageData&&);

  // Basic information
  int width() const noexcept;
  int height() const noexcept;
  PixelFormat format() const noexcept;
  int channels() const noexcept;
  size_t byte_count() const noexcept;
  bool is_valid() const noexcept;

  // Pixel data access
  const uint8_t* data() const noexcept;
  uint8_t* data() noexcept;
  const uint8_t* pixel(int x, int y) const;
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

#endif  // IMAGEJ_CORE_IMAGE_DATA_H_