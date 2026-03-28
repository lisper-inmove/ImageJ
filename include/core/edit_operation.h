#ifndef IMAGEJ_CORE_EDIT_OPERATION_H_
#define IMAGEJ_CORE_EDIT_OPERATION_H_

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ImageDocument;

class EditOperation {
 public:
  EditOperation();
  virtual ~EditOperation();

  // Disable copy and move operations for polymorphic base class
  EditOperation(const EditOperation&) = delete;
  EditOperation& operator=(const EditOperation&) = delete;
  EditOperation(EditOperation&&) = delete;
  EditOperation& operator=(EditOperation&&) = delete;

  // Command interface (pure virtual)
  virtual bool apply() = 0;
  virtual bool undo() = 0;
  virtual std::string description() const = 0;

  virtual bool can_undo() const = 0;
  virtual bool can_redo() const = 0;

  void set_target_document(ImageDocument* document) noexcept;
  ImageDocument* target_document() const noexcept;

  virtual bool can_merge_with(const EditOperation& other) const;
  virtual std::unique_ptr<EditOperation> merge_with(const EditOperation& other) const;

  virtual bool serialize(std::vector<uint8_t>& buffer) const;
  virtual bool deserialize(const std::vector<uint8_t>& buffer);

  std::chrono::system_clock::time_point timestamp() const noexcept;
  void set_timestamp(std::chrono::system_clock::time_point timestamp) noexcept;

 protected:
  ImageDocument* target_document_ = nullptr;
  std::chrono::system_clock::time_point timestamp_;
};

#endif  // IMAGEJ_CORE_EDIT_OPERATION_H_