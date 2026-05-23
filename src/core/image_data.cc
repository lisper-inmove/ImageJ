#include "core/image_data.h"

ImageData::ImageData()
    : width_(0), height_(0), format_(PixelFormat::kUnknown) {}
ImageData::ImageData(int width, int height, PixelFormat format)
    : width_(width), height_(height), format_(format) {}
ImageData::~ImageData() = default;

ImageData::ImageData(const ImageData &) = default;
ImageData &ImageData::operator=(const ImageData &) = default;
ImageData::ImageData(ImageData &&) = default;
ImageData &ImageData::operator=(ImageData &&) = default;

int ImageData::width() const noexcept { return width_; }
int ImageData::height() const noexcept { return height_; }
ImageData::PixelFormat ImageData::format() const noexcept { return format_; }
int ImageData::channels() const noexcept { return 0; }
size_t ImageData::byte_count() const noexcept { return 0; }
bool ImageData::is_valid() const noexcept { return false; }

const uint8_t *ImageData::data() const noexcept { return nullptr; }
uint8_t *ImageData::data() noexcept { return nullptr; }
const uint8_t *ImageData::pixel(int x, int y) const { return nullptr; }
uint8_t *ImageData::pixel(int x, int y) { return nullptr; }

bool ImageData::create(int width, int height, PixelFormat format) {
  return false;
}
void ImageData::clear() {}
bool ImageData::copy_from(const ImageData &other) { return false; }

int ImageData::calculate_stride(int width, PixelFormat format) { return 0; }
int ImageData::calculate_byte_count(int width, int height, PixelFormat format) {
  return 0;
}
