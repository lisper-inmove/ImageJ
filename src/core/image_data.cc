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

int ImageData::channels() const noexcept {
  switch (format_) {
    case PixelFormat::kGray8:
      return 1;
    case PixelFormat::kRGB24:
      return 3;
    case PixelFormat::kRGBA32:
      return 4;
    default:
      return 0;
  }
}

size_t ImageData::byte_count() const noexcept {
  return data_.size();
}

bool ImageData::is_valid() const noexcept {
  return width_ > 0 && height_ > 0 &&
         format_ != PixelFormat::kUnknown &&
         !data_.empty();
}

const uint8_t *ImageData::data() const noexcept {
  return data_.empty() ? nullptr : data_.data();
}

uint8_t *ImageData::data() noexcept {
  return data_.empty() ? nullptr : data_.data();
}

const uint8_t *ImageData::pixel(int x, int y) const {
  int stride = calculate_stride(width_, format_);
  int offset = y * stride + x * channels();
  return data_.data() + offset;
}

uint8_t *ImageData::pixel(int x, int y) {
  int stride = calculate_stride(width_, format_);
  int offset = y * stride + x * channels();
  return data_.data() + offset;
}

bool ImageData::create(int width, int height, PixelFormat format) {
  if (width <= 0 || height <= 0 || format == PixelFormat::kUnknown) {
    return false;
  }
  size_t bytes = calculate_byte_count(width, height, format);
  data_.resize(bytes);
  width_ = width;
  height_ = height;
  format_ = format;
  return true;
}

void ImageData::clear() {
  data_.clear();
  width_ = 0;
  height_ = 0;
  format_ = PixelFormat::kUnknown;
}

bool ImageData::copy_from(const ImageData &other) {
  if (!other.is_valid()) {
    return false;
  }
  if (!create(other.width_, other.height_, other.format_)) {
    return false;
  }
  std::copy(other.data_.begin(), other.data_.end(), data_.begin());
  return true;
}

int ImageData::calculate_stride(int width, PixelFormat format) {
  switch (format) {
    case PixelFormat::kGray8:
      return width;
    case PixelFormat::kRGB24:
      return width * 3;
    case PixelFormat::kRGBA32:
      return width * 4;
    default:
      return 0;
  }
}

int ImageData::calculate_byte_count(int width, int height, PixelFormat format) {
  if (width <= 0 || height <= 0) {
    return 0;
  }
  return calculate_stride(width, format) * height;
}
