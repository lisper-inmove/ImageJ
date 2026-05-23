#include <gtest/gtest.h>

#include "core/image_data.h"

class ImageDataTest : public ::testing::Test {};

// === Static utility tests ===

TEST_F(ImageDataTest, CalculateByteCount) {
  EXPECT_EQ(ImageData::calculate_byte_count(100, 100, ImageData::PixelFormat::kGray8), 10000);
  EXPECT_EQ(ImageData::calculate_byte_count(100, 100, ImageData::PixelFormat::kRGB24), 30000);
  EXPECT_EQ(ImageData::calculate_byte_count(100, 100, ImageData::PixelFormat::kRGBA32), 40000);
  EXPECT_EQ(ImageData::calculate_byte_count(0, 100, ImageData::PixelFormat::kGray8), 0);
  EXPECT_EQ(ImageData::calculate_byte_count(100, 0, ImageData::PixelFormat::kRGB24), 0);
}

TEST_F(ImageDataTest, CalculateStride) {
  EXPECT_EQ(ImageData::calculate_stride(100, ImageData::PixelFormat::kGray8), 100);
  EXPECT_EQ(ImageData::calculate_stride(100, ImageData::PixelFormat::kRGB24), 300);
  EXPECT_EQ(ImageData::calculate_stride(100, ImageData::PixelFormat::kRGBA32), 400);
}

// === Default constructor tests ===

TEST_F(ImageDataTest, DefaultConstructor) {
  ImageData data;
  EXPECT_FALSE(data.is_valid());
  EXPECT_EQ(data.width(), 0);
  EXPECT_EQ(data.height(), 0);
  EXPECT_EQ(data.format(), ImageData::PixelFormat::kUnknown);
  EXPECT_EQ(data.channels(), 0);
  EXPECT_EQ(data.byte_count(), 0);
  EXPECT_EQ(data.data(), nullptr);
}

// === Create tests ===

TEST_F(ImageDataTest, CreateGray8) {
  ImageData data;
  EXPECT_TRUE(data.create(64, 48, ImageData::PixelFormat::kGray8));
  EXPECT_TRUE(data.is_valid());
  EXPECT_EQ(data.width(), 64);
  EXPECT_EQ(data.height(), 48);
  EXPECT_EQ(data.format(), ImageData::PixelFormat::kGray8);
  EXPECT_EQ(data.channels(), 1);
  EXPECT_EQ(data.byte_count(), 64 * 48);
  EXPECT_NE(data.data(), nullptr);
}

TEST_F(ImageDataTest, CreateRGB24) {
  ImageData data;
  EXPECT_TRUE(data.create(100, 200, ImageData::PixelFormat::kRGB24));
  EXPECT_TRUE(data.is_valid());
  EXPECT_EQ(data.width(), 100);
  EXPECT_EQ(data.height(), 200);
  EXPECT_EQ(data.format(), ImageData::PixelFormat::kRGB24);
  EXPECT_EQ(data.channels(), 3);
  EXPECT_EQ(data.byte_count(), 100 * 200 * 3);
  EXPECT_NE(data.data(), nullptr);
}

TEST_F(ImageDataTest, CreateRGBA32) {
  ImageData data;
  EXPECT_TRUE(data.create(10, 10, ImageData::PixelFormat::kRGBA32));
  EXPECT_TRUE(data.is_valid());
  EXPECT_EQ(data.channels(), 4);
  EXPECT_EQ(data.byte_count(), 10 * 10 * 4);
  EXPECT_NE(data.data(), nullptr);
}

TEST_F(ImageDataTest, CreateFailsWithZeroDimensions) {
  ImageData data;
  EXPECT_FALSE(data.create(0, 100, ImageData::PixelFormat::kGray8));
  EXPECT_FALSE(data.is_valid());
  EXPECT_FALSE(data.create(100, 0, ImageData::PixelFormat::kRGB24));
  EXPECT_FALSE(data.is_valid());
  EXPECT_FALSE(data.create(0, 0, ImageData::PixelFormat::kRGBA32));
  EXPECT_FALSE(data.is_valid());
}

TEST_F(ImageDataTest, CreateFailsWithUnknownFormat) {
  ImageData data;
  EXPECT_FALSE(data.create(100, 100, ImageData::PixelFormat::kUnknown));
  EXPECT_FALSE(data.is_valid());
}

TEST_F(ImageDataTest, CreateReallocatesBuffer) {
  ImageData data;
  EXPECT_TRUE(data.create(32, 32, ImageData::PixelFormat::kGray8));
  const uint8_t* ptr1 = data.data();

  // Re-create with different size — pointer should change
  EXPECT_TRUE(data.create(64, 64, ImageData::PixelFormat::kRGB24));
  const uint8_t* ptr2 = data.data();

  EXPECT_NE(ptr1, ptr2);
  EXPECT_EQ(data.width(), 64);
  EXPECT_EQ(data.height(), 64);
  EXPECT_EQ(data.format(), ImageData::PixelFormat::kRGB24);
}

// === Pixel access tests ===

TEST_F(ImageDataTest, PixelIndexing) {
  ImageData data;
  ASSERT_TRUE(data.create(10, 10, ImageData::PixelFormat::kRGB24));

  // pixel(0, 0) == data()
  EXPECT_EQ(data.pixel(0, 0), data.data());

  // pixel(1, 0) == data() + 3 (RGB = 3 bytes per pixel)
  EXPECT_EQ(data.pixel(1, 0), data.data() + 3);

  // pixel(0, 1) == data() + 30 (stride = 10 * 3 = 30)
  EXPECT_EQ(data.pixel(0, 1), data.data() + 30);

  // pixel(5, 5) == data() + (5 * 30) + (5 * 3) = data() + 165
  EXPECT_EQ(data.pixel(5, 5), data.data() + 165);
}

TEST_F(ImageDataTest, PixelDataMutable) {
  ImageData data;
  ASSERT_TRUE(data.create(8, 8, ImageData::PixelFormat::kGray8));

  // Write to pixel
  uint8_t* p = data.pixel(3, 4);
  *p = 128;

  EXPECT_EQ(*data.pixel(3, 4), 128);
}

TEST_F(ImageDataTest, DataWritable) {
  ImageData data;
  ASSERT_TRUE(data.create(4, 4, ImageData::PixelFormat::kRGB24));

  uint8_t* buf = data.data();
  size_t total = data.byte_count();

  // Fill buffer with pattern
  for (size_t i = 0; i < total; i++) {
    buf[i] = static_cast<uint8_t>(i % 256);
  }

  // Verify
  for (size_t i = 0; i < total; i++) {
    EXPECT_EQ(buf[i], static_cast<uint8_t>(i % 256));
  }
}

TEST_F(ImageDataTest, ConstPixelAccess) {
  ImageData data;
  ASSERT_TRUE(data.create(2, 2, ImageData::PixelFormat::kGray8));
  data.data()[0] = 42;

  const ImageData& cdata = data;
  EXPECT_EQ(*cdata.pixel(0, 0), 42);
  EXPECT_EQ(*cdata.data(), 42);
}

// === Clear tests ===

TEST_F(ImageDataTest, ClearResetsState) {
  ImageData data;
  ASSERT_TRUE(data.create(100, 100, ImageData::PixelFormat::kRGB24));
  ASSERT_TRUE(data.is_valid());

  data.clear();
  EXPECT_FALSE(data.is_valid());
  EXPECT_EQ(data.width(), 0);
  EXPECT_EQ(data.height(), 0);
  EXPECT_EQ(data.format(), ImageData::PixelFormat::kUnknown);
  EXPECT_EQ(data.channels(), 0);
  EXPECT_EQ(data.byte_count(), 0);
  EXPECT_EQ(data.data(), nullptr);
}

// === Copy tests ===

TEST_F(ImageDataTest, CopyFrom) {
  ImageData src;
  ASSERT_TRUE(src.create(6, 4, ImageData::PixelFormat::kRGB24));

  // Fill source with pattern
  uint8_t* src_buf = src.data();
  for (size_t i = 0; i < src.byte_count(); i++) {
    src_buf[i] = static_cast<uint8_t>(i + 1);
  }

  ImageData dst;
  EXPECT_TRUE(dst.copy_from(src));

  EXPECT_TRUE(dst.is_valid());
  EXPECT_EQ(dst.width(), src.width());
  EXPECT_EQ(dst.height(), src.height());
  EXPECT_EQ(dst.format(), src.format());
  EXPECT_EQ(dst.byte_count(), src.byte_count());
  EXPECT_NE(dst.data(), src.data());  // Deep copy, different pointers

  // Verify pixel data is identical
  const uint8_t* dst_buf = dst.data();
  for (size_t i = 0; i < src.byte_count(); i++) {
    EXPECT_EQ(dst_buf[i], src_buf[i]) << "Mismatch at byte " << i;
  }
}

TEST_F(ImageDataTest, CopyFromInvalidSource) {
  ImageData dst;
  ASSERT_TRUE(dst.create(10, 10, ImageData::PixelFormat::kGray8));

  ImageData invalid;
  EXPECT_FALSE(dst.copy_from(invalid));
  // dst should be unchanged
  EXPECT_TRUE(dst.is_valid());
  EXPECT_EQ(dst.width(), 10);
  EXPECT_EQ(dst.height(), 10);
}

// === Channels tests ===

TEST_F(ImageDataTest, ChannelsPerFormat) {
  ImageData data;
  // Before create
  EXPECT_EQ(data.channels(), 0);

  data.create(1, 1, ImageData::PixelFormat::kGray8);
  EXPECT_EQ(data.channels(), 1);

  data.create(1, 1, ImageData::PixelFormat::kRGB24);
  EXPECT_EQ(data.channels(), 3);

  data.create(1, 1, ImageData::PixelFormat::kRGBA32);
  EXPECT_EQ(data.channels(), 4);
}

// === Parameterized constructor tests ===

TEST_F(ImageDataTest, ParameterizedConstructorDoesNotAllocate) {
  ImageData data(100, 200, ImageData::PixelFormat::kRGB24);
  // The parameterized constructor stores values but does NOT allocate a buffer
  EXPECT_EQ(data.width(), 100);
  EXPECT_EQ(data.height(), 200);
  EXPECT_EQ(data.format(), ImageData::PixelFormat::kRGB24);
  EXPECT_FALSE(data.is_valid());  // No buffer allocated
  EXPECT_EQ(data.data(), nullptr);
}
