#include <gtest/gtest.h>
#include <QImage>

#include "core/image_data.h"
#include "core/image_document.h"
#include "core/image_document_adapter.h"

class ImageDocumentAdapterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    doc_ = std::make_unique<ImageDocument>();
  }

  std::unique_ptr<ImageDocument> doc_;
};

// === Format mapping tests ===

TEST_F(ImageDocumentAdapterTest, ToQImageFormatMapping) {
  EXPECT_EQ(ImageDocumentAdapter::to_qimage_format(ImageData::PixelFormat::kGray8),
            QImage::Format_Grayscale8);
  EXPECT_EQ(ImageDocumentAdapter::to_qimage_format(ImageData::PixelFormat::kRGB24),
            QImage::Format_RGB888);
  EXPECT_EQ(ImageDocumentAdapter::to_qimage_format(ImageData::PixelFormat::kRGBA32),
            QImage::Format_RGBA8888);
  EXPECT_EQ(ImageDocumentAdapter::to_qimage_format(ImageData::PixelFormat::kUnknown),
            QImage::Format_Invalid);
}

TEST_F(ImageDocumentAdapterTest, FromQImageFormatMapping) {
  EXPECT_EQ(ImageDocumentAdapter::from_qimage_format(QImage::Format_Grayscale8),
            ImageData::PixelFormat::kGray8);
  EXPECT_EQ(ImageDocumentAdapter::from_qimage_format(QImage::Format_RGB888),
            ImageData::PixelFormat::kRGB24);
  EXPECT_EQ(ImageDocumentAdapter::from_qimage_format(QImage::Format_RGBA8888),
            ImageData::PixelFormat::kRGBA32);
  EXPECT_EQ(ImageDocumentAdapter::from_qimage_format(QImage::Format_Invalid),
            ImageData::PixelFormat::kUnknown);
}

TEST_F(ImageDocumentAdapterTest, FormatMappingRoundTrip) {
  std::vector<ImageData::PixelFormat> formats = {
      ImageData::PixelFormat::kGray8,
      ImageData::PixelFormat::kRGB24,
      ImageData::PixelFormat::kRGBA32,
  };
  for (auto fmt : formats) {
    QImage::Format qfmt = ImageDocumentAdapter::to_qimage_format(fmt);
    ImageData::PixelFormat back = ImageDocumentAdapter::from_qimage_format(qfmt);
    EXPECT_EQ(back, fmt);
  }
}

// === Adapter with null document ===

TEST_F(ImageDocumentAdapterTest, NullDocument) {
  ImageDocumentAdapter adapter(nullptr);
  EXPECT_EQ(adapter.document(), nullptr);
  EXPECT_FALSE(adapter.is_valid());
  EXPECT_FALSE(adapter.can_convert_to_qimage());
  EXPECT_TRUE(adapter.to_qimage().isNull());
  EXPECT_EQ(adapter.qsize(), QSize());
}

// === to_qimage from valid document ===

TEST_F(ImageDocumentAdapterTest, ToQImageFromValidDocument) {
  ASSERT_TRUE(doc_->image_data().create(16, 16, ImageData::PixelFormat::kRGB24));
  ImageDocumentAdapter adapter(doc_.get());
  EXPECT_TRUE(adapter.is_valid());
  EXPECT_TRUE(adapter.can_convert_to_qimage());

  QImage img = adapter.to_qimage();
  EXPECT_FALSE(img.isNull());
  EXPECT_EQ(img.width(), 16);
  EXPECT_EQ(img.height(), 16);
  EXPECT_EQ(img.format(), QImage::Format_RGB888);
}

// === update_from_qimage ===

TEST_F(ImageDocumentAdapterTest, UpdateFromQImage) {
  ImageDocumentAdapter adapter(doc_.get());

  QImage src(8, 4, QImage::Format_RGB888);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 8; x++) {
      src.setPixelColor(x, y, QColor(x * 30, y * 60, 128));
    }
  }

  EXPECT_TRUE(adapter.update_from_qimage(src));
  EXPECT_TRUE(doc_->image_data().is_valid());
  EXPECT_EQ(doc_->image_data().width(), 8);
  EXPECT_EQ(doc_->image_data().height(), 4);
  EXPECT_EQ(doc_->image_data().format(), ImageData::PixelFormat::kRGB24);
}

// === Round-trip: QImage → ImageData → QImage ===

TEST_F(ImageDocumentAdapterTest, RoundTrip) {
  ImageDocumentAdapter adapter(doc_.get());

  QImage src(6, 3, QImage::Format_RGB888);
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 6; x++) {
      src.setPixelColor(x, y, QColor(x * 40, y * 80, 200));
    }
  }

  ASSERT_TRUE(adapter.update_from_qimage(src));

  QImage recovered = adapter.to_qimage();
  ASSERT_FALSE(recovered.isNull());
  EXPECT_EQ(recovered.size(), src.size());

  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 6; x++) {
      EXPECT_EQ(recovered.pixelColor(x, y), src.pixelColor(x, y))
          << "Mismatch at (" << x << ", " << y << ")";
    }
  }
}

// === Default constructor ===

TEST_F(ImageDocumentAdapterTest, DefaultConstructor) {
  ImageDocumentAdapter adapter;
  EXPECT_EQ(adapter.document(), nullptr);
  EXPECT_FALSE(adapter.is_valid());
}

// === set_document ===

TEST_F(ImageDocumentAdapterTest, SetDocument) {
  ImageDocumentAdapter adapter;
  adapter.set_document(doc_.get());
  EXPECT_EQ(adapter.document(), doc_.get());

  adapter.set_document(nullptr);
  EXPECT_EQ(adapter.document(), nullptr);
}
