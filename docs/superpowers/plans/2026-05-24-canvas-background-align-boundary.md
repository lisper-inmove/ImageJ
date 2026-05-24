# Canvas Background, Alignment & Boundary Clamp Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace checkerboard default with solid dark-gray background, auto-reset view offset to (0,0) on new image, and clamp view offset so the image never exposes canvas edges.

**Architecture:** All changes are in ImageCanvas. A new private `clampViewOffset()` method encapsulates boundary logic and is called from every method that modifies `view_offset_` or `zoom_factor_`. Defaults changed at construction. `set_document()` auto-resets offset.

**Tech Stack:** C++17, Qt6, GTest

---

## Chunk 1: Change default background

### Task 1: Update default background and tests

**Files:**
- Modify: `src/widgets/image_canvas.cc:15-21` — constructor defaults
- Modify: `tests/widgets/image_canvas_test.cc:29-56` — background tests

- [ ] **Step 1: Update tests for new defaults**

In `tests/widgets/image_canvas_test.cc`, replace the background tests:

```cpp
TEST_F(ImageCanvasTest, DefaultBackgroundStyleIsSolidColor) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.background_style(), ImageCanvas::BackgroundStyle::kSolidColor);
}

TEST_F(ImageCanvasTest, DefaultBackgroundColor) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.background_color(), QColor(0x2D, 0x2D, 0x2D));
}
```

- [ ] **Step 2: Run tests to verify they FAIL**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.DefaultBackground*'`
Expected: 2 FAIL (defaults still kCheckerboard / Qt::white)

- [ ] **Step 3: Change defaults in constructor**

In `src/widgets/image_canvas.cc`, change constructor:
```cpp
background_style_(BackgroundStyle::kSolidColor),
background_color_(QColor(0x2D, 0x2D, 0x2D)) {
```

- [ ] **Step 4: Run tests to verify PASS**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.DefaultBackground*'`
Expected: 2 PASS

- [ ] **Step 5: Commit**

```bash
git add tests/widgets/image_canvas_test.cc src/widgets/image_canvas.cc
git commit -m "feat: change default canvas background to solid dark gray"
```

---

## Chunk 2: Auto-reset view offset on set_document

### Task 2: Reset view_offset_ in set_document

**Files:**
- Modify: `src/widgets/image_canvas.cc:28-32` — set_document()
- Modify: `tests/widgets/image_canvas_test.cc` — add test

- [ ] **Step 1: Add test**

Append to `tests/widgets/image_canvas_test.cc` before the Constructor tests:

```cpp
TEST_F(ImageCanvasTest, SetDocumentResetsViewOffset) {
  ImageCanvas canvas;
  canvas.set_view_offset(QPoint(50, 100));
  canvas.set_zoom_factor(2.0);

  ImageDocument doc;
  doc.image_data().create(32, 32, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);

  EXPECT_EQ(canvas.view_offset(), QPoint(0, 0));
}
```

- [ ] **Step 2: Run test to verify it FAILS**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.SetDocumentResetsViewOffset'`
Expected: FAIL (offset still (50, 100))

- [ ] **Step 3: Add reset in set_document()**

In `src/widgets/image_canvas.cc`, change `set_document()` to:
```cpp
void ImageCanvas::set_document(ImageDocument *document) {
  document_ = document;
  view_offset_ = QPoint(0, 0);
  emit document_changed(document_);
  update();
}
```

- [ ] **Step 4: Run test to verify PASS**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.SetDocumentResetsViewOffset'`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add tests/widgets/image_canvas_test.cc src/widgets/image_canvas.cc
git commit -m "feat: reset view offset to (0,0) on set_document"
```

---

## Chunk 3: Clamp view offset to canvas bounds

### Task 3: Add clampViewOffset and integrate

**Files:**
- Modify: `include/widgets/image_canvas.h:67-72` — add private method declaration
- Modify: `src/widgets/image_canvas.cc` — add clampViewOffset(), call it from wheelEvent, set_view_offset, set_zoom_factor, fit_to_window
- Modify: `tests/widgets/image_canvas_test.cc` — add clamp tests

- [ ] **Step 1: Add clamp tests**

Append to `tests/widgets/image_canvas_test.cc`:

```cpp
// === View offset clamp tests ===

TEST_F(ImageCanvasTest, ViewOffsetClampedToBounds) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);
  canvas.set_zoom_factor(1.0);
  QTest::qWait(50);

  // Try to scroll image left — should be clamped (image is smaller than canvas)
  canvas.set_view_offset(QPoint(-999, -999));
  // With 100x100 image at 1.0 zoom in 200x200 canvas:
  // image fits entirely, clamp range: [-(100-200), 0] = [100, 0] — invalid range
  // So offset shouldn't change when image is smaller than canvas
  // Set view_offset to large positive — should be clamped
  canvas.set_view_offset(QPoint(999, 999));
  EXPECT_LE(canvas.view_offset().x(), 0);
  EXPECT_LE(canvas.view_offset().y(), 0);
}

TEST_F(ImageCanvasTest, ViewOffsetClampedAfterScrolling) {
  ImageCanvas canvas;
  canvas.resize(100, 100);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  doc.image_data().create(200, 200, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);
  canvas.set_zoom_factor(1.0);
  QTest::qWait(50);

  // Scroll far right — image left edge should not go past canvas left edge
  canvas.set_view_offset(QPoint(50, 50));
  EXPECT_LE(canvas.view_offset().x(), 0);
  EXPECT_LE(canvas.view_offset().y(), 0);

  // Scroll far left — image right edge should not leave canvas
  canvas.set_view_offset(QPoint(-300, -300));
  // Scaled image is 200x200 in 100x100 canvas
  // offset range: [-(200-100), 0] = [-100, 0]
  EXPECT_GE(canvas.view_offset().x(), -100);
  EXPECT_GE(canvas.view_offset().y(), -100);
}

TEST_F(ImageCanvasTest, ClampViewOffsetNoDocumentDoesNotCrash) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  // No document — wheel event should not crash
  QWheelEvent event(QPointF(100, 100), QPointF(100, 100), QPoint(0, 0),
                    QPoint(0, -120), Qt::NoButton, Qt::NoModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  SUCCEED();
}
```

- [ ] **Step 2: Run tests to verify they FAIL**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.ViewOffset*:ImageCanvasTest.ClampView*'`
Expected: 3 FAIL (no clamping yet)

- [ ] **Step 3: Add clampViewOffset declaration to header**

In `include/widgets/image_canvas.h`, add to private section:
```cpp
  void clampViewOffset();
```

- [ ] **Step 4: Implement clampViewOffset and integrate calls**

In `src/widgets/image_canvas.cc`:

Add `clampViewOffset()` method:
```cpp
void ImageCanvas::clampViewOffset() {
  if (!document_ || !document_->is_valid()) {
    return;
  }
  const auto &image_data = document_->image_data();
  int scaled_w = static_cast<int>(image_data.width() * zoom_factor_);
  int scaled_h = static_cast<int>(image_data.height() * zoom_factor_);

  int max_x_offset = -(scaled_w - width());
  int max_y_offset = -(scaled_h - height());

  // Only clamp when image is larger than canvas in that dimension
  if (max_x_offset > 0) {
    view_offset_.rx() = std::clamp(view_offset_.x(), 0, max_x_offset);
  } else {
    view_offset_.rx() = std::clamp(view_offset_.x(), max_x_offset, 0);
  }
  if (max_y_offset > 0) {
    view_offset_.ry() = std::clamp(view_offset_.y(), 0, max_y_offset);
  } else {
    view_offset_.ry() = std::clamp(view_offset_.y(), max_y_offset, 0);
  }
}
```

Add `clampViewOffset()` call at the end of `wheelEvent()` (before `update()`):
```cpp
  clampViewOffset();
  update();
```

Add `clampViewOffset()` call in `set_view_offset()`:
```cpp
void ImageCanvas::set_view_offset(const QPoint &offset) {
  view_offset_ = offset;
  clampViewOffset();
  update();
}
```

Add `clampViewOffset()` call in `set_zoom_factor()`:
```cpp
void ImageCanvas::set_zoom_factor(double factor) {
  zoom_factor_ = factor;
  clampViewOffset();
  update();
}
```

Add `clampViewOffset()` call at end of `fit_to_window()`:
```cpp
  clampViewOffset();
  update();
```

- [ ] **Step 5: Run tests to verify PASS**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.ViewOffset*:ImageCanvasTest.ClampView*'`
Expected: 3 PASS

- [ ] **Step 6: Run full test suite**

Run: `./build/ImageJ_tests`
Expected: all tests pass

- [ ] **Step 7: Commit**

```bash
git add include/widgets/image_canvas.h src/widgets/image_canvas.cc tests/widgets/image_canvas_test.cc
git commit -m "feat: clamp image view offset to canvas bounds"
```
