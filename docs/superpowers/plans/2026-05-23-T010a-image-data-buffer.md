# T010a: ImageData Pixel Buffer Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement ImageData pixel buffer: `create()`, accessors, format helpers, lifecycle methods.

**Architecture:** `std::vector<uint8_t>` stores raw pixels. `channels()` and stride computed from `PixelFormat`. `pixel()` uses stride arithmetic for indexing. Header unchanged — all changes in `.cc` and tests.

**Tech Stack:** C++17, GTest

**Design doc:** `docs/superpowers/specs/2026-05-23-T010a-image-data-buffer-design.md`

---

## File Structure

| File | Action | Purpose |
|------|--------|---------|
| `src/core/image_data.cc` | Rewrite | Replace all stubs with real implementations |
| `tests/core/image_data_test.cc` | Rewrite | Comprehensive pixel buffer tests |

---

## Chunk 1: Test Suite Rewrite

### Task 1.1: Write comprehensive tests

**Files:**
- Modify: `tests/core/image_data_test.cc`

- [ ] **Step 1: Replace test file with comprehensive tests**

Replace entire `tests/core/image_data_test.cc` content:

```cpp
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
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter="*ImageData*" -v`
Expected: Most tests FAIL — `create()` returns false, `channels()` returns 0, etc.

---

## Chunk 2: Implementation

### Task 2.1: Implement all ImageData methods

**Files:**
- Modify: `src/core/image_data.cc`

- [ ] **Step 1: Replace entire image_data.cc with real implementation**

Replace entire `src/core/image_data.cc` content:

```cpp
#include "core/image_data.h"

ImageData::ImageData()
    : width_(0), height_(0), format_(PixelFormat::kUnknown) {}

ImageData::ImageData(int width, int height, PixelFormat format)
    : width_(width), height_(height), format_(format) {}

ImageData::~ImageData() = default;

ImageData::ImageData(const ImageData &) = default;
ImageData &ImageData::operator=(const ImageData &) = default;
ImageData::ImageData(ImageData &&) = default;
ImageData &ImageData::operator=(ImageData &&) = default;

int ImageData::width() const noexcept { return width_; }
int ImageData::height() const noexcept { return height_; }

ImageData::PixelFormat ImageData::format() const noexcept { return format_; }

int ImageData::channels() const noexcept {
  switch (format_) {
    case PixelFormat::kGray8:
      return 1;
    case PixelFormat::kRGB24:
      return 3;
    case PixelFormat::kRGBA32:
      return 4;
    default:
      return 0;
  }
}

size_t ImageData::byte_count() const noexcept {
  return data_.size();
}

bool ImageData::is_valid() const noexcept {
  return width_ > 0 && height_ > 0 &&
         format_ != PixelFormat::kUnknown &&
         !data_.empty();
}

const uint8_t *ImageData::data() const noexcept {
  return data_.empty() ? nullptr : data_.data();
}

uint8_t *ImageData::data() noexcept {
  return data_.empty() ? nullptr : data_.data();
}

const uint8_t *ImageData::pixel(int x, int y) const {
  int stride = calculate_stride(width_, format_);
  int offset = y * stride + x * channels();
  return data_.data() + offset;
}

uint8_t *ImageData::pixel(int x, int y) {
  int stride = calculate_stride(width_, format_);
  int offset = y * stride + x * channels();
  return data_.data() + offset;
}

bool ImageData::create(int width, int height, PixelFormat format) {
  if (width <= 0 || height <= 0 || format == PixelFormat::kUnknown) {
    return false;
  }
  size_t bytes = calculate_byte_count(width, height, format);
  data_.resize(bytes);
  width_ = width;
  height_ = height;
  format_ = format;
  return true;
}

void ImageData::clear() {
  data_.clear();
  width_ = 0;
  height_ = 0;
  format_ = PixelFormat::kUnknown;
}

bool ImageData::copy_from(const ImageData &other) {
  if (!other.is_valid()) {
    return false;
  }
  if (!create(other.width_, other.height_, other.format_)) {
    return false;
  }
  std::copy(other.data_.begin(), other.data_.end(), data_.begin());
  return true;
}

int ImageData::calculate_stride(int width, PixelFormat format) {
  switch (format) {
    case PixelFormat::kGray8:
      return width;
    case PixelFormat::kRGB24:
      return width * 3;
    case PixelFormat::kRGBA32:
      return width * 4;
    default:
      return 0;
  }
}

int ImageData::calculate_byte_count(int width, int height, PixelFormat format) {
  if (width <= 0 || height <= 0) {
    return 0;
  }
  return calculate_stride(width, format) * height;
}
```

- [ ] **Step 2: Run all ImageData tests to verify they pass**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter="*ImageData*" -v`
Expected: All ImageData tests PASS (approximately 20 tests)

- [ ] **Step 3: Run full test suite to verify no regressions**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests`
Expected: No regressions. Pre-existing ImageCanvas segfault may still exist.

- [ ] **Step 4: Commit**

```bash
git add src/core/image_data.cc tests/core/image_data_test.cc
git commit -m "feat: implement ImageData pixel buffer (T010a)

Implement create(), data(), pixel(), channels(), clear(), copy_from()
and static helpers calculate_stride(), calculate_byte_count().
All previously stub methods now work with std::vector<uint8_t> buffer.

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```

---

## Acceptance Criteria

- [ ] `create(w, h, format)` allocates correct buffer
- [ ] `data()` returns valid pointer after create
- [ ] `pixel(x, y)` returns correct byte offset
- [ ] `channels()` returns 1/3/4 per format
- [ ] `is_valid()` returns false for zero dims, unknown format, no buffer
- [ ] `clear()` resets all state
- [ ] `copy_from()` deep-copies pixel data
- [ ] `calculate_stride()` / `calculate_byte_count()` work correctly
- [ ] ~20 tests pass
- [ ] No regressions
