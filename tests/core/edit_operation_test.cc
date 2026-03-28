#include <gtest/gtest.h>

#include "core/edit_operation.h"

// Mock implementation for testing
class MockEditOperation : public EditOperation {
 public:
  bool apply() override { return false; }
  bool undo() override { return false; }
  std::string description() const override { return "Mock Operation"; }
  bool can_undo() const override { return false; }
  bool can_redo() const override { return false; }
};

TEST(EditOperationTest, AbstractClass) {
  MockEditOperation op;
  EXPECT_EQ(op.description(), "Mock Operation");
  EXPECT_FALSE(op.can_undo());
}

TEST(EditOperationTest, TargetDocument) {
  MockEditOperation op;
  EXPECT_EQ(op.target_document(), nullptr);

  // Can't test set_target_document without actual ImageDocument
}

TEST(EditOperationTest, Timestamp) {
  MockEditOperation op;
  auto now = std::chrono::system_clock::now();
  op.set_timestamp(now);
  EXPECT_EQ(op.timestamp(), now);
}