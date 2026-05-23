# T005: ImageCanvas Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement ImageCanvas core functionality: checkerboard background, mouse events, coordinate conversion, zoom, and document binding.

**Architecture:** ImageCanvas is a QWidget subclass that renders a checkerboard background and handles mouse events. It stores zoom factor, view offset, background style, and a pointer to ImageDocument. Coordinate conversion between image space and canvas space uses zoom + offset math.

**Tech Stack:** C++17, Qt 6 Widgets, GTest + QTest

**Design doc:** `docs/superpowers/specs/2026-05-23-T005-image-canvas-design.md`

---

## File Map

| File | Action | Purpose |
|------|--------|---------|
| `include/widgets/image_canvas.h` | No change | Header is already complete |
| `src/widgets/image_canvas.cc` | Rewrite | Replace all stubs with real implementations |
| `tests/widgets/image_canvas_test.cc` | Rewrite | Comprehensive tests for all implemented methods |

---

## Chunk 1: State, Background Drawing, and Resize

### Task 1.1: Write tests for background style and checkerboard drawing

**Files:**
- Modify: `tests/widgets/image_canvas_test.cc`

- [ ] **Step 1: Add tests for background style defaults and setter/getter**

Replace the entire test file with:

```cpp
#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include "widgets/image_canvas.h"

class ImageCanvasTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // QApplication is created once for all tests
  }

  void SetUp() override {
    static int argc = 1;
    static char* argv[] = {const_cast<char*>("test")};
    if (!QApplication::instance()) {
      app_ = std::make_unique<QApplication>(argc, argv);
    }
  }

  void TearDown() override {}

  std::unique_ptr<QApplication> app_;
};

// === Background style tests ===

TEST_F(ImageCanvasTest, DefaultBackgroundStyleIsCheckerboard) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.background_style(), ImageCanvas::BackgroundStyle::kCheckerboard);
}

TEST_F(ImageCanvasTest, SetBackgroundStyle) {
  ImageCanvas canvas;
  canvas.set_background_style(ImageCanvas::BackgroundStyle::kSolidColor);
  EXPECT_EQ(canvas.background_style(), ImageCanvas::BackgroundStyle::kSolidColor);

  canvas.set_background_style(ImageCanvas::BackgroundStyle::kTransparent);
  EXPECT_EQ(canvas.background_style(), ImageCanvas::BackgroundStyle::kTransparent);

  canvas.set_background_style(ImageCanvas::BackgroundStyle::kCheckerboard);
  EXPECT_EQ(canvas.background_style(), ImageCanvas::BackgroundStyle::kCheckerboard);
}

TEST_F(ImageCanvasTest, DefaultBackgroundColor) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.background_color(), QColor());
}

TEST_F(ImageCanvasTest, SetBackgroundColor) {
  ImageCanvas canvas;
  canvas.set_background_color(QColor(255, 0, 0));
  EXPECT_EQ(canvas.background_color(), QColor(255, 0, 0));
}

TEST_F(ImageCanvasTest, BackgroundStylePersists) {
  ImageCanvas canvas;
  canvas.set_background_style(ImageCanvas::BackgroundStyle::kSolidColor);
  canvas.set_background_color(QColor(0, 255, 0));

  EXPECT_EQ(canvas.background_style(), ImageCanvas::BackgroundStyle::kSolidColor);
  EXPECT_EQ(canvas.background_color(), QColor(0, 255, 0));
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd build && cmake .. && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Background*"`
Expected: Tests fail - `background_style()` returns default (which is already `kCheckerboard`), but setters have no effect since they're stubs.

- [ ] **Step 3: Implement background style/color state**

In `src/widgets/image_canvas.cc`, add member variables and implement getters/setters:

```cpp
#include "widgets/image_canvas.h"
#include "core/image_document.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent),
      document_(nullptr),
      zoom_factor_(1.0),
      view_offset_(0, 0),
      background_style_(BackgroundStyle::kCheckerboard),
      background_color_(Qt::white) {
  setMinimumSize(100, 100);
  setMouseTracking(true);
}

ImageCanvas::~ImageCanvas() = default;

void ImageCanvas::set_background_style(BackgroundStyle style) {
  background_style_ = style;
  update();
}

ImageCanvas::BackgroundStyle ImageCanvas::background_style() const {
  return background_style_;
}

void ImageCanvas::set_background_color(const QColor &color) {
  background_color_ = color;
  update();
}

QColor ImageCanvas::background_color() const {
  return background_color_;
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Background*"`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/widgets/image_canvas.cc tests/widgets/image_canvas_test.cc
git commit -m "feat: implement ImageCanvas background style state

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```

---

### Task 1.2: Implement checkerboard paintEvent

- [ ] **Step 1: Add test for paintEvent not crashing**

Append to `tests/widgets/image_canvas_test.cc`:

```cpp
// === Paint tests ===

TEST_F(ImageCanvasTest, PaintEventDoesNotCrash) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  // Trigger a repaint - should not crash
  canvas.update();
  QTest::qWait(50);
  canvas.repaint();
  QTest::qWait(50);

  SUCCEED();  // If we got here without crashing, the test passes
}

TEST_F(ImageCanvasTest, PaintEventWithDifferentSizes) {
  ImageCanvas canvas;
  canvas.show();
  QTest::qWait(50);

  // Test painting at various sizes
  QList<QSize> sizes = {{100, 100}, {200, 50}, {50, 200}, {1, 1}, {800, 600}};
  for (const auto& size : sizes) {
    canvas.resize(size);
    canvas.repaint();
    QTest::qWait(10);
    SUCCEED();  // No crash = pass
  }
}

TEST_F(ImageCanvasTest, PaintAfterBackgroundStyleChange) {
  ImageCanvas canvas;
  canvas.resize(150, 150);
  canvas.show();
  QTest::qWait(50);

  canvas.set_background_style(ImageCanvas::BackgroundStyle::kCheckerboard);
  canvas.repaint();
  QTest::qWait(20);

  canvas.set_background_style(ImageCanvas::BackgroundStyle::kSolidColor);
  canvas.repaint();
  QTest::qWait(20);

  SUCCEED();  // No crash = pass
}
```

- [ ] **Step 2: Run tests to verify they pass (paintEvent is currently empty, won't crash)**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Paint*"`
Expected: PASS (empty paintEvent doesn't crash)

- [ ] **Step 3: Implement checkerboard paintEvent and resizeEvent**

Update `paintEvent` and `resizeEvent` in `src/widgets/image_canvas.cc`:

```cpp
void ImageCanvas::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  QRect rect = event->rect();

  switch (background_style_) {
    case BackgroundStyle::kCheckerboard: {
      const int square_size = 16;
      const QColor light(0xCC, 0xCC, 0xCC);
      const QColor dark(0x99, 0x99, 0x99);

      int start_col = rect.left() / square_size;
      int end_col = (rect.right() + square_size - 1) / square_size;
      int start_row = rect.top() / square_size;
      int end_row = (rect.bottom() + square_size - 1) / square_size;

      for (int row = start_row; row <= end_row; ++row) {
        for (int col = start_col; col <= end_col; ++col) {
          QColor color = ((row + col) % 2 == 0) ? light : dark;
          painter.fillRect(col * square_size, row * square_size,
                           square_size, square_size, color);
        }
      }
      break;
    }
    case BackgroundStyle::kSolidColor:
      painter.fillRect(rect, background_color_);
      break;
    case BackgroundStyle::kTransparent:
      // Transparent - let parent widget show through
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.fillRect(rect, Qt::transparent);
      break;
  }
}

void ImageCanvas::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  // View offset may need adjustment when widget resizes
  // Full implementation will come with T013/T014
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Paint*:*Background*"`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/widgets/image_canvas.cc tests/widgets/image_canvas_test.cc
git commit -m "feat: implement checkerboard background paintEvent

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```

---

## Chunk 2: Zoom and Coordinate Conversion

### Task 2.1: Implement zoom methods

- [ ] **Step 1: Add zoom tests**

Append to `tests/widgets/image_canvas_test.cc`:

```cpp
// === Zoom tests ===

TEST_F(ImageCanvasTest, DefaultZoomFactorIsOne) {
  ImageCanvas canvas;
  EXPECT_DOUBLE_EQ(canvas.zoom_factor(), 1.0);
}

TEST_F(ImageCanvasTest, SetZoomFactor) {
  ImageCanvas canvas;
  canvas.set_zoom_factor(2.0);
  EXPECT_DOUBLE_EQ(canvas.zoom_factor(), 2.0);

  canvas.set_zoom_factor(0.5);
  EXPECT_DOUBLE_EQ(canvas.zoom_factor(), 0.5);

  canvas.set_zoom_factor(10.0);
  EXPECT_DOUBLE_EQ(canvas.zoom_factor(), 10.0);
}

TEST_F(ImageCanvasTest, ResetZoom) {
  ImageCanvas canvas;
  canvas.set_zoom_factor(3.5);
  EXPECT_DOUBLE_EQ(canvas.zoom_factor(), 3.5);

  canvas.reset_zoom();
  EXPECT_DOUBLE_EQ(canvas.zoom_factor(), 1.0);
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Zoom*"`
Expected: FAIL - `set_zoom_factor` and `reset_zoom` are stubs

- [ ] **Step 3: Implement zoom methods**

Update `src/widgets/image_canvas.cc`:

```cpp
void ImageCanvas::set_zoom_factor(double factor) {
  zoom_factor_ = factor;
  update();
}

double ImageCanvas::zoom_factor() const {
  return zoom_factor_;
}

void ImageCanvas::reset_zoom() {
  zoom_factor_ = 1.0;
  update();
}

void ImageCanvas::fit_to_window() {
  if (!document_ || !document_->is_valid()) {
    return;
  }
  const auto& image_data = document_->image_data();
  double scale_x = static_cast<double>(width()) / image_data.width();
  double scale_y = static_cast<double>(height()) / image_data.height();
  zoom_factor_ = std::min(scale_x, scale_y);
  update();
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Zoom*"`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/widgets/image_canvas.cc tests/widgets/image_canvas_test.cc
git commit -m "feat: implement ImageCanvas zoom methods

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```

---

### Task 2.2: Implement coordinate conversion and view offset

- [ ] **Step 1: Add coordinate conversion tests**

Append to `tests/widgets/image_canvas_test.cc`:

```cpp
// === Coordinate conversion tests ===

TEST_F(ImageCanvasTest, ImageToCanvasAtDefaultZoom) {
  ImageCanvas canvas;
  // At zoom 1.0 and offset (0,0), image coords == canvas coords
  QPoint image_pt(100, 200);
  QPoint canvas_pt = canvas.image_to_canvas(image_pt);
  EXPECT_EQ(canvas_pt, QPoint(100, 200));
}

TEST_F(ImageCanvasTest, ImageToCanvasAtZoomTwo) {
  ImageCanvas canvas;
  canvas.set_zoom_factor(2.0);
  QPoint image_pt(50, 100);
  QPoint canvas_pt = canvas.image_to_canvas(image_pt);
  EXPECT_EQ(canvas_pt, QPoint(100, 200));
}

TEST_F(ImageCanvasTest, CanvasToImageAtDefaultZoom) {
  ImageCanvas canvas;
  QPoint canvas_pt(150, 300);
  QPoint image_pt = canvas.canvas_to_image(canvas_pt);
  EXPECT_EQ(image_pt, QPoint(150, 300));
}

TEST_F(ImageCanvasTest, CanvasToImageAtZoomHalf) {
  ImageCanvas canvas;
  canvas.set_zoom_factor(0.5);
  QPoint canvas_pt(100, 200);
  QPoint image_pt = canvas.canvas_to_image(canvas_pt);
  EXPECT_EQ(image_pt, QPoint(200, 400));
}

TEST_F(ImageCanvasTest, CoordinateConversionRoundTrip) {
  ImageCanvas canvas;
  canvas.set_zoom_factor(1.5);

  QPoint original(42, 73);
  QPoint canvas_pt = canvas.image_to_canvas(original);
  QPoint back = canvas.canvas_to_image(canvas_pt);
  // Rounding may cause slight deviation with division, allow 1px tolerance
  EXPECT_NEAR(back.x(), original.x(), 1);
  EXPECT_NEAR(back.y(), original.y(), 1);
}

TEST_F(ImageCanvasTest, RectCoordinateConversion) {
  ImageCanvas canvas;
  canvas.set_zoom_factor(2.0);

  QRect image_rect(10, 20, 30, 40);
  QRect canvas_rect = canvas.image_to_canvas(image_rect);
  EXPECT_EQ(canvas_rect.x(), 20);
  EXPECT_EQ(canvas_rect.y(), 40);
  EXPECT_EQ(canvas_rect.width(), 60);
  EXPECT_EQ(canvas_rect.height(), 80);
}

// === View offset tests ===

TEST_F(ImageCanvasTest, DefaultViewOffset) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.view_offset(), QPoint(0, 0));
}

TEST_F(ImageCanvasTest, SetViewOffset) {
  ImageCanvas canvas;
  canvas.set_view_offset(QPoint(50, 100));
  EXPECT_EQ(canvas.view_offset(), QPoint(50, 100));
}

TEST_F(ImageCanvasTest, ViewOffsetAffectsCoordinateConversion) {
  ImageCanvas canvas;
  canvas.set_zoom_factor(1.0);
  canvas.set_view_offset(QPoint(10, 20));

  // Image (0,0) should map to canvas (10, 20) with offset
  QPoint canvas_pt = canvas.image_to_canvas(QPoint(0, 0));
  EXPECT_EQ(canvas_pt, QPoint(10, 20));

  // Canvas (10, 20) should map back to image (0, 0)
  QPoint image_pt = canvas.canvas_to_image(QPoint(10, 20));
  EXPECT_EQ(image_pt, QPoint(0, 0));
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Coordinate*:*View*"`
Expected: FAIL - coordinate conversion methods are stubs

- [ ] **Step 3: Implement coordinate conversion and view offset**

Update `src/widgets/image_canvas.cc`:

```cpp
QPoint ImageCanvas::view_offset() const {
  return view_offset_;
}

void ImageCanvas::set_view_offset(const QPoint &offset) {
  view_offset_ = offset;
  update();
}

QRect ImageCanvas::visible_image_rect() const {
  QPoint top_left = canvas_to_image(QPoint(0, 0));
  QPoint bottom_right = canvas_to_image(QPoint(width(), height()));
  return QRect(top_left, bottom_right);
}

QPoint ImageCanvas::image_to_canvas(const QPoint &image_point) const {
  return QPoint(
      static_cast<int>(image_point.x() * zoom_factor_) + view_offset_.x(),
      static_cast<int>(image_point.y() * zoom_factor_) + view_offset_.y());
}

QPoint ImageCanvas::canvas_to_image(const QPoint &canvas_point) const {
  return QPoint(
      static_cast<int>((canvas_point.x() - view_offset_.x()) / zoom_factor_),
      static_cast<int>((canvas_point.y() - view_offset_.y()) / zoom_factor_));
}

QRect ImageCanvas::image_to_canvas(const QRect &image_rect) const {
  QPoint top_left = image_to_canvas(image_rect.topLeft());
  QPoint bottom_right = image_to_canvas(image_rect.bottomRight());
  return QRect(top_left, bottom_right);
}

QRect ImageCanvas::canvas_to_image(const QRect &canvas_rect) const {
  QPoint top_left = canvas_to_image(canvas_rect.topLeft());
  QPoint bottom_right = canvas_to_image(canvas_rect.bottomRight());
  return QRect(top_left, bottom_right);
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Coordinate*:*View*"`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/widgets/image_canvas.cc tests/widgets/image_canvas_test.cc
git commit -m "feat: implement coordinate conversion and view offset

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```

---

## Chunk 3: Document Binding

### Task 3.1: Implement document setter/getter and change notification

- [ ] **Step 1: Add document binding tests**

Append to `tests/widgets/image_canvas_test.cc`:

```cpp
// === Document binding tests ===

#include "core/image_document.h"

TEST_F(ImageCanvasTest, DefaultDocumentIsNull) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.document(), nullptr);
}

TEST_F(ImageCanvasTest, SetDocument) {
  ImageCanvas canvas;
  ImageDocument doc;
  canvas.set_document(&doc);
  EXPECT_EQ(canvas.document(), &doc);
}

TEST_F(ImageCanvasTest, SetDocumentToNull) {
  ImageCanvas canvas;
  ImageDocument doc;
  canvas.set_document(&doc);
  EXPECT_EQ(canvas.document(), &doc);

  canvas.set_document(nullptr);
  EXPECT_EQ(canvas.document(), nullptr);
}

TEST_F(ImageCanvasTest, DocumentChangedSignal) {
  ImageCanvas canvas;
  ImageDocument doc;

  ImageDocument* received_doc = nullptr;
  QObject::connect(&canvas, &ImageCanvas::document_changed,
                   [&received_doc](ImageDocument* doc) {
                     received_doc = doc;
                   });

  canvas.set_document(&doc);
  EXPECT_EQ(received_doc, &doc);

  // Setting same document should still emit
  received_doc = nullptr;
  canvas.set_document(&doc);
  EXPECT_EQ(received_doc, &doc);
}

TEST_F(ImageCanvasTest, OnDocumentModifiedTriggersUpdate) {
  ImageCanvas canvas;
  canvas.show();
  QTest::qWait(50);

  canvas.on_document_modified();
  // Should not crash - triggers repaint
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, SetDocumentTriggersUpdate) {
  ImageCanvas canvas;
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  canvas.set_document(&doc);
  QTest::qWait(50);
  SUCCEED();  // No crash = pass
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Document*"`
Expected: FAIL - `set_document` and `on_document_modified` are stubs

- [ ] **Step 3: Implement document binding**

Update `src/widgets/image_canvas.cc`:

```cpp
void ImageCanvas::set_document(ImageDocument *document) {
  document_ = document;
  emit document_changed(document_);
  update();
}

ImageDocument *ImageCanvas::document() const {
  return document_;
}

void ImageCanvas::update_display() {
  update();
}

void ImageCanvas::force_redraw() {
  repaint();
}

void ImageCanvas::on_document_modified() {
  update();
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Document*"`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/widgets/image_canvas.cc tests/widgets/image_canvas_test.cc
git commit -m "feat: implement document binding in ImageCanvas

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```

---

## Chunk 4: Mouse Events

### Task 4.1: Implement mouse event handlers

- [ ] **Step 1: Add mouse event tests**

Append to `tests/widgets/image_canvas_test.cc`:

```cpp
// === Mouse event tests ===

TEST_F(ImageCanvasTest, MousePressEmitsImageClicked) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QPoint received_pos(-1, -1);
  Qt::MouseButton received_button = Qt::NoButton;
  QObject::connect(&canvas, &ImageCanvas::image_clicked,
                   [&](const QPoint& pos, Qt::MouseButton btn) {
                     received_pos = pos;
                     received_button = btn;
                   });

  // Click at canvas position (100, 150), default zoom 1.0, offset (0,0) -> image (100, 150)
  QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 150));
  QTest::qWait(50);

  EXPECT_EQ(received_pos, QPoint(100, 150));
  EXPECT_EQ(received_button, Qt::LeftButton);
}

TEST_F(ImageCanvasTest, MouseClickWithZoom) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  canvas.set_zoom_factor(2.0);

  QPoint received_pos(-1, -1);
  QObject::connect(&canvas, &ImageCanvas::image_clicked,
                   [&](const QPoint& pos, Qt::MouseButton btn) {
                     received_pos = pos;
                   });

  // Canvas (100, 100) -> image (50, 50) at zoom 2.0
  QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(50);

  EXPECT_EQ(received_pos, QPoint(50, 50));
}

TEST_F(ImageCanvasTest, RightClickEmitsCorrectButton) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  Qt::MouseButton received_button = Qt::NoButton;
  QObject::connect(&canvas, &ImageCanvas::image_clicked,
                   [&](const QPoint& pos, Qt::MouseButton btn) {
                     received_button = btn;
                   });

  QTest::mouseClick(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(50);

  EXPECT_EQ(received_button, Qt::RightButton);
}

TEST_F(ImageCanvasTest, MouseMoveEmitsSignal) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QPoint received_pos(-1, -1);
  QObject::connect(&canvas, &ImageCanvas::mouse_over_image,
                   [&](const QPoint& pos) {
                     received_pos = pos;
                   });

  // Move mouse to a specific position
  QTest::mouseMove(&canvas, QPoint(80, 120));
  QTest::qWait(50);

  EXPECT_EQ(received_pos, QPoint(80, 120));
}

TEST_F(ImageCanvasTest, MouseMoveWithOffset) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  canvas.set_view_offset(QPoint(10, 20));

  QPoint received_pos(-1, -1);
  QObject::connect(&canvas, &ImageCanvas::mouse_over_image,
                   [&](const QPoint& pos) {
                     received_pos = pos;
                   });

  // Canvas (50, 70) with offset (10, 20) -> image (40, 50)
  QTest::mouseMove(&canvas, QPoint(50, 70));
  QTest::qWait(50);

  EXPECT_EQ(received_pos, QPoint(40, 50));
}

TEST_F(ImageCanvasTest, MouseReleaseDoesNotCrash) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, MouseEventsOnResizedCanvas) {
  ImageCanvas canvas;
  canvas.show();
  QTest::qWait(50);

  // Resize and verify events still work
  canvas.resize(500, 400);
  QTest::qWait(50);

  bool clicked = false;
  QObject::connect(&canvas, &ImageCanvas::image_clicked,
                   [&](const QPoint&, Qt::MouseButton) { clicked = true; });

  QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(250, 200));
  QTest::qWait(50);

  EXPECT_TRUE(clicked);
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Mouse*"`
Expected: FAIL - mouse event handlers are stubs

- [ ] **Step 3: Implement mouse event handlers**

Update `src/widgets/image_canvas.cc`:

```cpp
void ImageCanvas::mousePressEvent(QMouseEvent *event) {
  QPoint image_pos = canvas_to_image(event->pos());
  emit image_clicked(image_pos, event->button());
  QWidget::mousePressEvent(event);
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event) {
  QPoint image_pos = canvas_to_image(event->pos());
  emit mouse_over_image(image_pos);
  QWidget::mouseMoveEvent(event);
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event) {
  // Reserved for future tool support (selection, drawing, etc.)
  QWidget::mouseReleaseEvent(event);
}

void ImageCanvas::wheelEvent(QWheelEvent *event) {
  // Reserved for zoom with Ctrl+wheel (T013)
  QWidget::wheelEvent(event);
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd build && make ImageJ_tests && ./ImageJ_tests --gtest_filter="*Mouse*"`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/widgets/image_canvas.cc tests/widgets/image_canvas_test.cc
git commit -m "feat: implement mouse event handlers in ImageCanvas

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```

---

## Final Verification

### Task 5.1: Run all tests

- [ ] **Step 1: Run the full test suite**

```bash
cd build && cmake .. && make ImageJ_tests && ./ImageJ_tests
```

Expected: All tests PASS (approximately 25 tests)

- [ ] **Step 2: Run ctest**

```bash
cd build && ctest --output-on-failure
```

Expected: All tests PASS

- [ ] **Step 3: Update todo-list.md to mark T005 as completed**

Update `design/todo-list.md`:
- Change T005 status from `未开始` to `已完成`

- [ ] **Step 4: Final commit**

```bash
git add design/todo-list.md
git commit -m "docs: mark T005 as completed

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
```
