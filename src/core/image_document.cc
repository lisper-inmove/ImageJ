#include "core/image_document.h"

ImageDocument::ImageDocument() = default;
ImageDocument::~ImageDocument() = default;

ImageDocument::ImageDocument(const ImageDocument &) = default;
ImageDocument &ImageDocument::operator=(const ImageDocument &) = default;
ImageDocument::ImageDocument(ImageDocument &&) noexcept = default;
ImageDocument &ImageDocument::operator=(ImageDocument &&) noexcept = default;

const ImageData &ImageDocument::image_data() const noexcept {
  return image_data_;
}
bool ImageDocument::is_valid() const noexcept { return false; }
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
  return false;
}
bool ImageDocument::save_to_file(const std::string &file_path) { return false; }
bool ImageDocument::save_as(const std::string &file_path) { return false; }

void ImageDocument::set_image_data(const ImageData &image_data) {}
void ImageDocument::clear() {}

bool ImageDocument::has_file_path() const noexcept { return false; }
const std::string &ImageDocument::file_path() const noexcept {
  static std::string empty;
  return empty;
}
const std::string &ImageDocument::file_name() const noexcept {
  static std::string empty;
  return empty;
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
