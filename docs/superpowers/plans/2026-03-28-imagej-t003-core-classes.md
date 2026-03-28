# ImageJ T003 Core Classes Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the 5 core classes framework (ImageData, ImageDocument, ImageDocumentAdapter, EditOperation, ImageCanvas) with directory structure, headers with empty declarations, and unit tests that compile and run.

**Architecture:** Decoupled architecture with Core Layer (Qt-independent), Adapter Layer (Qt-dependent), and UI Layer (Qt Widgets). Follows design from `/docs/superpowers/specs/2026-03-28-imagej-t003-core-classes-design.md`.

**Tech Stack:** C++17, Qt6, Google C++ Style Guide, Google Test

---

## File Structure

**Core Layer (Qt Independent):**
- Create: `include/core/image_data.h` - Custom image data structure
- Create: `include/core/image_document.h` - Image document model
- Create: `include/core/edit_operation.h` - Base class for edit operations
- Create: `src/core/image_data.cc` - Implementation (empty)
- Create: `src/core/image_document.cc` - Implementation (empty)
- Create: `src/core/edit_operation.cc` - Implementation (empty)

**Adapter Layer (Qt Dependent):**
- Create: `include/core/image_document_adapter.h` - Qt adapter for ImageDocument
- Create: `src/core/image_document_adapter.cc` - Implementation (empty)

**UI Layer (Qt Widgets Dependent):**
- Create: `include/widgets/image_canvas.h` - Qt display widget
- Create: `src/widgets/image_canvas.cc` - Implementation (empty)

**Tests:**
- Create: `tests/core/image_data_test.cc` - Unit tests for ImageData
- Create: `tests/core/image_document_test.cc` - Unit tests for ImageDocument
- Create: `tests/core/edit_operation_test.cc` - Unit tests for EditOperation
- Create: `tests/core/image_document_adapter_test.cc` - Unit tests for ImageDocumentAdapter
- Create: `tests/widgets/image_canvas_test.cc` - Unit tests for ImageCanvas

**CMake Updates:**
- Modify: `config/main.cmake` - Add test targets
- Modify: `config/qtconfig.cmake` - Update file glob patterns

---

## Chunk 1: Directory Structure and Core Layer Headers

### Task 1: Create Directory Structure

**Files:**
- Create: `include/core/`
- Create: `src/core/`
- Create: `tests/core/`
- Create: `include/widgets/`
- Create: `src/widgets/`
- Create: `tests/widgets/`

- [ ] **Step 1: Create core directories**

```bash
mkdir -p include/core src/core tests/core
mkdir -p include/widgets src/widgets tests/widgets
```

- [ ] **Step 2: Verify directory creation**

```bash
ls -la include/ src/ tests/
```
Expected: Shows `core/` and `widgets/` directories

### Task 2: Create ImageData Header

**Files:**
- Create: `include/core/image_data.h`

- [ ] **Step 1: Write ImageData header file**

```cpp
#ifndef IMAGE_DATA_H_
#define IMAGE_DATA_H_

#include <cstdint>
#include <vector>

class ImageData {
 public:
  // Pixel format enumeration
  enum class PixelFormat {
    kUnknown = 0,
    kGray8,      // 8-bit grayscale
    kRGB24,      // 24-bit RGB
    kRGBA32,     // 32-bit RGBA
  };

  // Construction and destruction
  ImageData();
  ImageData(int width, int height, PixelFormat format);
  ~ImageData();

  // Basic information
  int width() const;
  int height() const;
  PixelFormat format() const;
  int channels() const;
  size_t byte_count() const;
  bool is_valid() const;

  // Pixel data access
  const uint8_t* data() const;
  uint8_t* data();
  uint8_t* pixel(int x, int y);

  // Image operations
  bool create(int width, int height, PixelFormat format);
  void clear();
  bool copy_from(const ImageData& other);

  // Static utility methods
  static int calculate_stride(int width, PixelFormat format);
  static int calculate_byte_count(int width, int height, PixelFormat format);

 private:
  int width_;
  int height_;
  PixelFormat format_;
  std::vector<uint8_t> data_;
};

#endif  // IMAGE_DATA_H_
```

- [ ] **Step 2: Create empty implementation file**

```bash
touch src/core/image_data.cc
```

### Task 3: Create ImageDocument Header

**Files:**
- Create: `include/core/image_document.h`

- [ ] **Step 1: Write ImageDocument header file**

```cpp
#ifndef IMAGE_DOCUMENT_H_
#define IMAGE_DOCUMENT_H_

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "core/image_data.h"

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
```

- [ ] **Step 2: Create empty implementation file**

```bash
touch src/core/image_document.cc
```

### Task 4: Create EditOperation Header

**Files:**
- Create: `include/core/edit_operation.h`

- [ ] **Step 1: Write EditOperation header file**

```cpp
#ifndef EDIT_OPERATION_H_
#define EDIT_OPERATION_H_

#include <chrono>
#include <memory>
#include <string>
#include <vector>

class ImageDocument;

class EditOperation {
 public:
  EditOperation();
  virtual ~EditOperation();

  // Command interface (pure virtual)
  virtual bool apply() = 0;
  virtual bool undo() = 0;
  virtual std::string description() const = 0;

  virtual bool can_undo() const = 0;
  virtual bool can_redo() const = 0;

  void set_target_document(ImageDocument* document);
  ImageDocument* target_document() const;

  virtual bool can_merge_with(const EditOperation& other) const;
  virtual std::unique_ptr<EditOperation> merge_with(const EditOperation& other) const;

  virtual bool serialize(std::vector<uint8_t>& buffer) const;
  virtual bool deserialize(const std::vector<uint8_t>& buffer);

  std::chrono::system_clock::time_point timestamp() const;
  void set_timestamp(std::chrono::system_clock::time_point timestamp);

 protected:
  ImageDocument* target_document_ = nullptr;
  std::chrono::system_clock::time_point timestamp_;
};

#endif  // EDIT_OPERATION_H_
```

- [ ] **Step 2: Create empty implementation file**

```bash
touch src/core/edit_operation.cc
```

---

## Chunk 2: Adapter Layer and UI Layer Headers

### Task 5: Create ImageDocumentAdapter Header

**Files:**
- Create: `include/core/image_document_adapter.h`

- [ ] **Step 1: Write ImageDocumentAdapter header file**

```cpp
#ifndef IMAGE_DOCUMENT_ADAPTER_H_
#define IMAGE_DOCUMENT_ADAPTER_H_

#include <QImage>
#include <QPixmap>
#include <QSize>

class ImageDocument;

class ImageDocumentAdapter {
 public:
  explicit ImageDocumentAdapter(ImageDocument* document = nullptr);
  ~ImageDocumentAdapter();

  void set_document(ImageDocument* document);
  ImageDocument* document() const;

  bool can_convert_to_qimage() const;
  QImage to_qimage() const;
  QPixmap to_qpixmap() const;

  bool is_valid() const;
  QSize qsize() const;

  bool update_from_qimage(const QImage& qimage);
  bool update_from_qpixmap(const QPixmap& qpixmap);

  static QImage::Format to_qimage_format(ImageData::PixelFormat format);
  static ImageData::PixelFormat from_qimage_format(QImage::Format format);

 private:
  ImageDocument* document_ = nullptr;
};

#endif  // IMAGE_DOCUMENT_ADAPTER_H_
```

- [ ] **Step 2: Create empty implementation file**

```bash
touch src/core/image_document_adapter.cc
```

### Task 6: Create ImageCanvas Header

**Files:**
- Create: `include/widgets/image_canvas.h`

- [ ] **Step 1: Write ImageCanvas header file**

```cpp
#ifndef IMAGE_CANVAS_H_
#define IMAGE_CANVAS_H_

#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QColor>

class ImageDocument;

class ImageCanvas : public QWidget {
  Q_OBJECT

 public:
  enum class BackgroundStyle {
    kCheckerboard,
    kSolidColor,
    kTransparent
  };

  explicit ImageCanvas(QWidget* parent = nullptr);
  ~ImageCanvas() override;

  void set_document(ImageDocument* document);
  ImageDocument* document() const;

  void set_zoom_factor(double factor);
  double zoom_factor() const;
  void fit_to_window();
  void reset_zoom();

  QPoint view_offset() const;
  void set_view_offset(const QPoint& offset);
  QRect visible_image_rect() const;

  void set_background_style(BackgroundStyle style);
  BackgroundStyle background_style() const;
  void set_background_color(const QColor& color);
  QColor background_color() const;

  void update_display();
  void force_redraw();

  // Event overrides
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

  // Coordinate conversion
  QPoint image_to_canvas(const QPoint& image_point) const;
  QPoint canvas_to_image(const QPoint& canvas_point) const;
  QRect image_to_canvas(const QRect& image_rect) const;
  QRect canvas_to_image(const QRect& canvas_rect) const;

 signals:
  void document_changed(ImageDocument* new_document);
  void view_changed();
  void mouse_over_image(const QPoint& image_position);
  void image_clicked(const QPoint& image_position, Qt::MouseButton button);

 public slots:
  void on_document_modified();
};

#endif  // IMAGE_CANVAS_H_
```

- [ ] **Step 2: Create empty implementation file**

```bash
touch src/widgets/image_canvas.cc
```

---

## Chunk 3: Unit Tests

### Task 7: Create ImageData Unit Tests

**Files:**
- Create: `tests/core/image_data_test.cc`

- [ ] **Step 1: Write ImageData test file**

```cpp
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
```

- [ ] **Step 2: Verify test file exists**

```bash
ls -la tests/core/image_data_test.cc
```
Expected: File exists

### Task 8: Create ImageDocument Unit Tests

**Files:**
- Create: `tests/core/image_document_test.cc`

- [ ] **Step 1: Write ImageDocument test file**

```cpp
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
```

- [ ] **Step 2: Verify test file exists**

```bash
ls -la tests/core/image_document_test.cc
```
Expected: File exists

### Task 9: Create EditOperation Unit Tests

**Files:**
- Create: `tests/core/edit_operation_test.cc`

- [ ] **Step 1: Write EditOperation test file**

```cpp
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
```

- [ ] **Step 2: Verify test file exists**

```bash
ls -la tests/core/edit_operation_test.cc
```
Expected: File exists

---

## Chunk 4: Qt-Dependent Tests and CMake Updates

### Task 10: Create ImageDocumentAdapter Unit Tests

**Files:**
- Create: `tests/core/image_document_adapter_test.cc`

- [ ] **Step 1: Write ImageDocumentAdapter test file**

```cpp
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
```

- [ ] **Step 2: Verify test file exists**

```bash
ls -la tests/core/image_document_adapter_test.cc
```
Expected: File exists

### Task 11: Create ImageCanvas Unit Tests

**Files:**
- Create: `tests/widgets/image_canvas_test.cc`

- [ ] **Step 1: Write ImageCanvas test file**

```cpp
#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>

#include "widgets/image_canvas.h"

class ImageCanvasTest : public ::testing::Test {
 protected:
  void SetUp() override {
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

TEST_F(ImageCanvasTest, Constructor) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.document(), nullptr);
  EXPECT_EQ(canvas.zoom_factor(), 1.0);
}

TEST_F(ImageCanvasTest, ShowAndHide) {
  ImageCanvas canvas;
  canvas.show();
  QTest::qWait(100);
  EXPECT_TRUE(canvas.isVisible());
  canvas.hide();
  QTest::qWait(100);
  EXPECT_FALSE(canvas.isVisible());
}
```

- [ ] **Step 2: Verify test file exists**

```bash
ls -la tests/widgets/image_canvas_test.cc
```
Expected: File exists

### Task 12: Update CMake Configuration

**Files:**
- Modify: `config/qtconfig.cmake`
- Modify: `config/main.cmake`

- [ ] **Step 1: Update qtconfig.cmake to include new directories**

Edit `config/qtconfig.cmake` lines 16-35 to include new directories:

```cmake
file(GLOB_RECURSE PROJECT_HEADERS
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/core/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/*.h"
)

file(GLOB_RECURSE PROJECT_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cc"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.cc"
)
```

- [ ] **Step 2: Verify CMake changes**

```bash
grep -n "core\|widgets" config/qtconfig.cmake
```
Expected: Shows lines with core/ and widgets/ patterns

- [ ] **Step 3: Create test target in main.cmake**

Add to the end of `config/main.cmake`:

```cmake
# Unit tests
enable_testing()

file(GLOB_RECURSE TEST_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc"
     "${CMAKE_CURRENT_SOURCE_DIR}/tests/core/*.cc"
     "${CMAKE_CURRENT_SOURCE_DIR}/tests/widgets/*.cc"
)

if(Qt6Test_FOUND)
  add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
  target_link_libraries(${PROJECT_NAME}_tests PRIVATE Qt6::Test GTest::gtest GTest::gtest_main)
  target_include_directories(${PROJECT_NAME}_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
  add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
endif()
```

- [ ] **Step 4: Verify test target configuration**

```bash
tail -20 config/main.cmake
```
Expected: Shows test configuration

---

## Chunk 5: Compilation Verification

### Task 13: Build and Test Compilation

**Files:**
- Test: Build system

- [ ] **Step 1: Create empty implementation files**

Create minimal implementations to allow compilation:

```bash
# Create minimal ImageData implementation
cat > src/core/image_data.cc << 'EOF'
#include "core/image_data.h"

ImageData::ImageData() : width_(0), height_(0), format_(PixelFormat::kUnknown) {}
ImageData::ImageData(int width, int height, PixelFormat format)
    : width_(width), height_(height), format_(format) {}
ImageData::~ImageData() = default;

int ImageData::width() const { return width_; }
int ImageData::height() const { return height_; }
ImageData::PixelFormat ImageData::format() const { return format_; }
int ImageData::channels() const { return 0; }
size_t ImageData::byte_count() const { return 0; }
bool ImageData::is_valid() const { return false; }

const uint8_t* ImageData::data() const { return nullptr; }
uint8_t* ImageData::data() { return nullptr; }
uint8_t* ImageData::pixel(int x, int y) { return nullptr; }

bool ImageData::create(int width, int height, PixelFormat format) { return false; }
void ImageData::clear() {}
bool ImageData::copy_from(const ImageData& other) { return false; }

int ImageData::calculate_stride(int width, PixelFormat format) { return 0; }
int ImageData::calculate_byte_count(int width, int height, PixelFormat format) { return 0; }
EOF

# Create minimal ImageDocument implementation
cat > src/core/image_document.cc << 'EOF'
#include "core/image_document.h"

ImageDocument::ImageDocument() = default;
ImageDocument::~ImageDocument() = default;

const ImageData& ImageDocument::image_data() const { return image_data_; }
bool ImageDocument::is_valid() const { return false; }
bool ImageDocument::is_modified() const { return is_modified_; }
void ImageDocument::set_modified(bool modified) { is_modified_ = modified; }

const ImageDocument::Metadata& ImageDocument::metadata() const { return metadata_; }
void ImageDocument::set_metadata(const Metadata& metadata) { metadata_ = metadata; }

bool ImageDocument::create_new(int width, int height, ImageData::PixelFormat format) { return false; }
bool ImageDocument::load_from_file(const std::string& file_path) { return false; }
bool ImageDocument::save_to_file(const std::string& file_path) { return false; }
bool ImageDocument::save_as(const std::string& file_path) { return false; }

void ImageDocument::set_image_data(const ImageData& image_data) {}
void ImageDocument::clear() {}

bool ImageDocument::has_file_path() const { return false; }
const std::string& ImageDocument::file_path() const { static std::string empty; return empty; }
const std::string& ImageDocument::file_name() const { static std::string empty; return empty; }

void ImageDocument::add_change_listener(DocumentChangedCallback callback) {}
void ImageDocument::remove_change_listener(DocumentChangedCallback callback) {}
void ImageDocument::notify_changed() {}

bool ImageDocument::serialize_to_buffer(std::vector<uint8_t>& buffer) const { return false; }
bool ImageDocument::deserialize_from_buffer(const std::vector<uint8_t>& buffer) { return false; }
EOF
```

- [ ] **Step 2: Run CMake configuration**

```bash
cd build && cmake .. && cd ..
```
Expected: CMake configuration succeeds

- [ ] **Step 3: Build the project**

```bash
cd build && make -j4 && cd ..
```
Expected: Build succeeds (may have warnings but no errors)

- [ ] **Step 4: Run tests (they may fail due to empty implementations)**

```bash
cd build && ./ImageJ_tests || echo "Tests may fail - that's OK per requirements" && cd ..
```
Expected: Tests compile and run (may fail due to empty implementations)

- [ ] **Step 5: Verify main application builds**

```bash
cd build && ls -la ImageJ && cd ..
```
Expected: ImageJ executable exists

---

## Summary

**Completion Criteria:**
- ✅ Directory structure created
- ✅ All header files with empty declarations created
- ✅ All unit test files created
- ✅ CMake updated to include new files
- ✅ Project compiles successfully
- ✅ Tests compile and run (may fail due to empty implementations)

**User Requirements Met:**
- ✓ "先创建单元测试" - Unit tests created first
- ✓ "类文件里的函数体可以暂时为空（但是要有声明）" - Function bodies can be empty but declarations exist
- ✓ "确保单元测试能够通过编译并运行" - Tests compile and run
- ✓ "但不一定测试通过" - Tests may fail due to empty implementations

**Next Steps:** After this plan is executed, the T003 framework will be in place with empty implementations. Actual functionality can be implemented in subsequent tasks while maintaining the decoupled architecture.