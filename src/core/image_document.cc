#include "core/image_document.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <QImage>

#include "core/image_document_adapter.h"
#include "opencv2/core/mat.hpp"

ImageDocument::ImageDocument() = default;
ImageDocument::~ImageDocument() = default;

ImageDocument::ImageDocument(const ImageDocument &) = default;
ImageDocument &ImageDocument::operator=(const ImageDocument &) = default;
ImageDocument::ImageDocument(ImageDocument &&) noexcept = default;
ImageDocument &ImageDocument::operator=(ImageDocument &&) noexcept = default;

const ImageData &ImageDocument::image_data() const noexcept {
  return image_data_;
}
ImageData &ImageDocument::image_data() noexcept { return image_data_; }
bool ImageDocument::is_valid() const noexcept { return image_data_.is_valid(); }
bool ImageDocument::is_modified() const noexcept { return is_modified_; }
void ImageDocument::set_modified(bool modified) noexcept {
  is_modified_ = modified;
}

const ImageDocument::Metadata &ImageDocument::metadata() const noexcept {
  return metadata_;
}
void ImageDocument::set_metadata(const Metadata &metadata) {
  metadata_ = metadata;
}

bool ImageDocument::create_new(int width, int height,
                               ImageData::PixelFormat format) {
  return false;
}
namespace {

// Auto-detect best dimensions from total pixels (for RAW files).
// Returns {width, height} preferring standard aspect ratios.
std::pair<int, int> detectRawDimensions(int64_t total_pixels) {
  std::vector<std::pair<int, int>> candidates;
  double best_score = 1e9;
  int best_w = 0, best_h = 0;

  for (int w = 64; w <= total_pixels; ++w) {
    if (total_pixels % w != 0) continue;
    int h = static_cast<int>(total_pixels / w);
    if (h < 64) break;
    double ratio = static_cast<double>(w) / h;
    // Standard targets: 16:9 (1.778), 3:2 (1.5), 4:3 (1.333), 1:1 (1.0)
    double score = 1e9;
    for (double target : {16.0 / 9.0, 3.0 / 2.0, 4.0 / 3.0, 1.0}) {
      score = std::min(score, std::abs(ratio - target));
    }
    if (score < best_score) {
      best_score = score;
      best_w = w;
      best_h = h;
    }
  }
  return {best_w, best_h};
}

// Load grayscale RAW file. Detects bit format and normalizes to 8-bit.
// If width/height == 0, auto-detects dimensions. Otherwise validates size.
// is_8bit: file contains 8-bit grayscale data (1 byte/pixel).
bool loadRawFile(const std::string& file_path, ImageData& out_data,
                 int width = 0, int height = 0, bool is_8bit = false) {
  // Read entire file
  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) return false;

  int64_t file_size = file.tellg();
  if (file_size < 128) return false;  // too small
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> raw_bytes(file_size);
  file.read(reinterpret_cast<char*>(raw_bytes.data()), file_size);

  if (is_8bit) {
    // 8-bit RAW: file_size should equal width * height
    if (width <= 0 || height <= 0) return false;
    int64_t expected_bytes = static_cast<int64_t>(width) * height;
    if (file_size != expected_bytes) {
      return false;
    }

    if (!out_data.create(width, height, ImageData::PixelFormat::kGray8)) {
      return false;
    }

    const uint8_t* src = raw_bytes.data();
    for (int y = 0; y < height; ++y) {
      uint8_t* dst_row = out_data.pixel(0, y);
      memcpy(dst_row, src + static_cast<int64_t>(y) * width, width);
    }
    return true;
  }

  int64_t total_pixels = file_size / 2;

  if (width > 0 && height > 0) {
    // User-specified dimensions: validate against file size
    int64_t expected_bytes = static_cast<int64_t>(width) * height * 2;
    if (file_size != expected_bytes) {
      return false;
    }
  } else {
    // Auto-detect dimensions
    auto [w, h] = detectRawDimensions(total_pixels);
    width = w;
    height = h;
    if (width == 0 || height == 0) return false;
  }

  // Read as 16-bit LE, find max to detect bit format
  const uint16_t* src = reinterpret_cast<const uint16_t*>(raw_bytes.data());
  uint16_t max_val = 0;
  int64_t sample_count = std::min(total_pixels, int64_t(65536));
  for (int64_t i = 0; i < sample_count; ++i) {
    max_val = std::max(max_val, src[i]);
  }

  // Determine shift to normalize to 8-bit
  int shift;
  if (max_val <= 4095) {
    shift = 4;   // 12-bit direct: >>4 to get 8-bit
  } else if ((max_val >> 4) <= 4095) {
    shift = 8;   // 12-bit left-shifted to 16-bit: >>8 to get 8-bit
  } else if (max_val <= 16383) {
    shift = 6;   // 14-bit direct
  } else if ((max_val >> 2) <= 16383) {
    shift = 8;   // 14-bit left-shifted
  } else {
    shift = 8;   // 16-bit full range or unknown, use >>8
  }

  if (!out_data.create(width, height, ImageData::PixelFormat::kGray8)) {
    return false;
  }

  for (int y = 0; y < height; ++y) {
    uint8_t* dst_row = out_data.pixel(0, y);
    for (int x = 0; x < width; ++x) {
      int64_t idx = static_cast<int64_t>(y) * width + x;
      uint16_t val = src[idx];
      dst_row[x] = static_cast<uint8_t>((val >> shift) & 0xFF);
    }
  }

  return true;
}

}  // namespace

bool ImageDocument::load_from_file(const std::string &file_path) {
  // Detect RAW files by extension
  {
    size_t dot = file_path.rfind('.');
    std::string ext;
    if (dot != std::string::npos) {
      ext = file_path.substr(dot);
    }
    if (ext == ".raw") {
    if (!loadRawFile(file_path, image_data_)) {
      return false;
    }

    metadata_.file_path = file_path;
    metadata_.file_format = "raw";
    std::ifstream f(file_path, std::ios::binary | std::ios::ate);
    if (f.is_open()) {
      metadata_.file_size = f.tellg();
    }
    is_modified_ = false;
    return true;
  }
  }  // end RAW detection scope

  cv::Mat mat = cv::imread(file_path, cv::IMREAD_COLOR);
  if (mat.empty()) {
    return false;
  }

  // Convert BGR (OpenCV default) to RGB for ImageData storage
  cv::Mat rgb_mat;
  cv::cvtColor(mat, rgb_mat, cv::COLOR_BGR2RGB);

  QImage qimage(rgb_mat.data, rgb_mat.cols, rgb_mat.rows,
                static_cast<int>(rgb_mat.step), QImage::Format_RGB888);
  // Deep copy: QImage does not own the cv::Mat data
  QImage copied = qimage.copy();

  ImageDocumentAdapter adapter(this);
  if (!adapter.update_from_qimage(copied)) {
    return false;
  }

  // Set metadata
  metadata_.file_path = file_path;
  // Extract file format from extension
  size_t dot_pos = file_path.rfind('.');
  if (dot_pos != std::string::npos) {
    metadata_.file_format = file_path.substr(dot_pos + 1);
  }

  // Get file size
  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    metadata_.file_size = file.tellg();
  }

  is_modified_ = false;
  return true;
}

bool ImageDocument::load_raw_from_file(const std::string& file_path,
                                       int width, int height, bool is_8bit) {
  if (!loadRawFile(file_path, image_data_, width, height, is_8bit)) {
    return false;
  }

  metadata_.file_path = file_path;
  metadata_.file_format = "raw";
  std::ifstream f(file_path, std::ios::binary | std::ios::ate);
  if (f.is_open()) {
    metadata_.file_size = f.tellg();
  }
  is_modified_ = false;
  return true;
}

bool ImageDocument::save_to_file(const std::string &file_path) { return false; }
bool ImageDocument::save_as(const std::string &file_path) { return false; }

void ImageDocument::set_image_data(const ImageData &image_data) {
  image_data_.copy_from(image_data);
  is_modified_ = true;
  notify_changed();
}

void ImageDocument::clear() {
  image_data_.clear();
  metadata_ = Metadata{};
  is_modified_ = false;
}

bool ImageDocument::has_file_path() const noexcept {
  return !metadata_.file_path.empty();
}

const std::string &ImageDocument::file_path() const noexcept {
  return metadata_.file_path;
}

const std::string &ImageDocument::file_name() const noexcept {
  if (metadata_.file_path.empty()) {
    static const std::string empty;
    return empty;
  }
  // Extract just the filename (cache it temporarily — basic approach)
  thread_local std::string cached_name;
  cached_name = metadata_.file_path;
  size_t sep = cached_name.find_last_of("/\\");
  if (sep != std::string::npos) {
    cached_name = cached_name.substr(sep + 1);
  }
  return cached_name;
}

void ImageDocument::add_change_listener(DocumentChangedCallback callback) {}
void ImageDocument::remove_change_listener(DocumentChangedCallback callback) {}
void ImageDocument::notify_changed() {}

bool ImageDocument::serialize_to_buffer(std::vector<uint8_t> &buffer) const {
  return false;
}
bool ImageDocument::deserialize_from_buffer(
    const std::vector<uint8_t> &buffer) {
  return false;
}
