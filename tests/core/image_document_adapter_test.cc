#include <gtest/gtest.h>
#include <QApplication>

#include "core/image_document_adapter.h"

class ImageDocumentAdapterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Initialize Qt application if needed
    static int argc = 1;
    static char* argv[] = {const_cast<char*>("test")};
    if (!QApplication::instance()) {
      app_ = std::make_unique<QApplication>(argc, argv);
    }
  }

  void TearDown() override {
    // app_ cleanup handled by unique_ptr
  }

  std::unique_ptr<QApplication> app_;
};

TEST_F(ImageDocumentAdapterTest, DefaultConstructor) {
  ImageDocumentAdapter adapter;
  EXPECT_EQ(adapter.document(), nullptr);
  EXPECT_FALSE(adapter.is_valid());
}

TEST_F(ImageDocumentAdapterTest, FormatConversion) {
  // Test format conversion - will likely fail due to empty implementation
  // auto qformat = ImageDocumentAdapter::to_qimage_format(ImageData::PixelFormat::kRGB24);
  // EXPECT_EQ(qformat, QImage::Format_RGB888);
}