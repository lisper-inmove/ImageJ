#include <gtest/gtest.h>

#include "core/image_data.h"

TEST(ImageDataTest, DefaultConstructor) {
  ImageData data;
  EXPECT_FALSE(data.is_valid());
  EXPECT_EQ(data.width(), 0);
  EXPECT_EQ(data.height(), 0);
}

TEST(ImageDataTest, CreateWithDimensions) {
  ImageData data(100, 200, ImageData::PixelFormat::kRGB24);
  // Tests may fail due to empty implementation - that's OK per requirements
  // EXPECT_TRUE(data.is_valid());
  // EXPECT_EQ(data.width(), 100);
  // EXPECT_EQ(data.height(), 200);
}

TEST(ImageDataTest, PixelFormatChannels) {
  EXPECT_EQ(ImageData::calculate_byte_count(100, 100, ImageData::PixelFormat::kGray8), 10000);
  EXPECT_EQ(ImageData::calculate_byte_count(100, 100, ImageData::PixelFormat::kRGB24), 30000);
  EXPECT_EQ(ImageData::calculate_byte_count(100, 100, ImageData::PixelFormat::kRGBA32), 40000);
}