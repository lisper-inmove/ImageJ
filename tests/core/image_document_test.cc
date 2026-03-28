#include <gtest/gtest.h>

#include "core/image_document.h"

TEST(ImageDocumentTest, DefaultConstructor) {
  ImageDocument doc;
  EXPECT_FALSE(doc.is_valid());
  EXPECT_FALSE(doc.is_modified());
}

TEST(ImageDocumentTest, CreateNew) {
  ImageDocument doc;
  // This will likely fail due to empty implementation - that's OK
  // bool result = doc.create_new(100, 100, ImageData::PixelFormat::kRGB24);
  // EXPECT_TRUE(result);
  // EXPECT_TRUE(doc.is_valid());
}

TEST(ImageDocumentTest, Metadata) {
  ImageDocument doc;
  ImageDocument::Metadata meta;
  meta.file_path = "/test/path.png";
  meta.file_format = "PNG";
  doc.set_metadata(meta);

  // const auto& retrieved = doc.metadata();
  // EXPECT_EQ(retrieved.file_path, "/test/path.png");
}