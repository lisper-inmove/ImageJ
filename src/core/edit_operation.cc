#include "core/edit_operation.h"

EditOperation::EditOperation() = default;

EditOperation::~EditOperation() = default;

void EditOperation::set_target_document(ImageDocument* document) noexcept {
  target_document_ = document;
}

ImageDocument* EditOperation::target_document() const noexcept {
  return target_document_;
}

bool EditOperation::can_merge_with(const EditOperation& other) const {
  // Default implementation: operations cannot be merged
  return false;
}

std::unique_ptr<EditOperation> EditOperation::merge_with(const EditOperation& other) const {
  // Default implementation: return nullptr (cannot merge)
  return nullptr;
}

bool EditOperation::serialize(std::vector<uint8_t>& buffer) const {
  // Default implementation: not serializable
  return false;
}

bool EditOperation::deserialize(const std::vector<uint8_t>& buffer) {
  // Default implementation: not deserializable
  return false;
}

std::chrono::system_clock::time_point EditOperation::timestamp() const noexcept {
  return timestamp_;
}

void EditOperation::set_timestamp(std::chrono::system_clock::time_point timestamp) noexcept {
  timestamp_ = timestamp;
}