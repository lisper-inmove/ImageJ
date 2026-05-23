#ifndef IMAGEJ_CORE_IMAGE_DOCUMENT_H_
#define IMAGEJ_CORE_IMAGE_DOCUMENT_H_

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "image_data.h"

class ImageDocument {
 public:
  // Document metadata structure
  struct Metadata {
    std::string file_path;
    std::string file_format;
    int64_t file_size = 0;
    std::chrono::system_clock::time_point created_time;
    std::chrono::system_clock::time_point modified_time;
    std::map<std::string, std::string> custom_fields;
  };

  // Core interface
  ImageDocument();
  virtual ~ImageDocument();

  // Copy and move operations (Rule of Five)
  ImageDocument(const ImageDocument&);
  ImageDocument& operator=(const ImageDocument&);
  ImageDocument(ImageDocument&&) noexcept;
  ImageDocument& operator=(ImageDocument&&) noexcept;

  const ImageData& image_data() const noexcept;
  ImageData& image_data() noexcept;
  bool is_valid() const noexcept;
  bool is_modified() const noexcept;
  void set_modified(bool modified) noexcept;

  const Metadata& metadata() const noexcept;
  void set_metadata(const Metadata& metadata);

  bool create_new(int width, int height, ImageData::PixelFormat format);
  bool load_from_file(const std::string& file_path);
  bool save_to_file(const std::string& file_path);
  bool save_as(const std::string& file_path);

  void set_image_data(const ImageData& image_data);
  void clear();

  bool has_file_path() const noexcept;
  const std::string& file_path() const noexcept;
  const std::string& file_name() const noexcept;

  // Change notification support
  using DocumentChangedCallback = std::function<void(ImageDocument*)>;
  void add_change_listener(DocumentChangedCallback callback);
  void remove_change_listener(DocumentChangedCallback callback);
  void notify_changed();

  // Serialization support
  bool serialize_to_buffer(std::vector<uint8_t>& buffer) const;
  bool deserialize_from_buffer(const std::vector<uint8_t>& buffer);

 private:
  ImageData image_data_;
  Metadata metadata_;
  bool is_modified_ = false;
  std::vector<DocumentChangedCallback> change_listeners_;
};

#endif  // IMAGEJ_CORE_IMAGE_DOCUMENT_H_