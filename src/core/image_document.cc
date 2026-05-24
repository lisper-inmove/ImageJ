#include "core/image_document.h"

#include <fstream>
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
bool ImageDocument::load_from_file(const std::string &file_path) {
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
