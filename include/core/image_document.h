#ifndef IMAGE_DOCUMENT_H_
#define IMAGE_DOCUMENT_H_

#include <chrono>
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
  ~ImageDocument();

  const ImageData& image_data() const;
  bool is_valid() const;
  bool is_modified() const;
  void set_modified(bool modified);

  const Metadata& metadata() const;
  void set_metadata(const Metadata& metadata);

  bool create_new(int width, int height, ImageData::PixelFormat format);
  bool load_from_file(const std::string& file_path);
  bool save_to_file(const std::string& file_path);
  bool save_as(const std::string& file_path);

  void set_image_data(const ImageData& image_data);
  void clear();

  bool has_file_path() const;
  const std::string& file_path() const;
  const std::string& file_name() const;

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

#endif  // IMAGE_DOCUMENT_H_