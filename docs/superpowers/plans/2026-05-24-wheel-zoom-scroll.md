# Wheel Zoom & Scroll Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement Ctrl+Wheel zoom (pixel-anchored), Wheel vertical scroll, and Shift+Wheel horizontal scroll in ImageCanvas::wheelEvent().

**Architecture:** Single-method change in ImageCanvas. wheelEvent() checks modifiers: Ctrl → zoom anchored to cursor pixel; Shift → horizontal scroll; none → vertical scroll. All logic uses existing canvas_to_image/image_to_canvas coordinate conversions and view_offset_.

**Tech Stack:** C++17, Qt6 (QWheelEvent, QMouseEvent)

---

## Chunk 1: Implement wheelEvent with tests

### Task 1: Add wheel event tests to image_canvas_test.cc

**Files:**
- Modify: `tests/widgets/image_canvas_test.cc` — add 6 test cases

- [ ] **Step 1: Add wheel event test cases**

Append the following tests before the end of the file (before `// === Constructor tests ===`):

```cpp
// === Wheel event tests ===

TEST_F(ImageCanvasTest, WheelEventVerticalScroll) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QPoint initial_offset = canvas.view_offset();
  EXPECT_EQ(initial_offset, QPoint(0, 0));

  QWheelEvent event(QPointF(150, 150), QPointF(150, 150), QPoint(0, 0),
                    QPoint(0, -120), Qt::NoButton, Qt::NoModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  // Scrolling down (negative delta) should increase view_offset.y()
  EXPECT_GT(canvas.view_offset().y(), initial_offset.y());
  EXPECT_EQ(canvas.view_offset().x(), initial_offset.x());
}

TEST_F(ImageCanvasTest, WheelEventHorizontalScroll) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  // Shift+Wheel: Qt may put delta in angleDelta().x() or .y()
  // We test with delta in x() which is the most common case on X11/Wayland
  QWheelEvent event(QPointF(150, 150), QPointF(150, 150), QPoint(0, 0),
                    QPoint(-120, 0), Qt::NoButton, Qt::ShiftModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  QPoint offset = canvas.view_offset();
  EXPECT_GT(offset.x(), 0);
  EXPECT_EQ(offset.y(), 0);
}

TEST_F(ImageCanvasTest, CtrlWheelZoomInKeepsPixelUnderCursor) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  // Create a document so coordinate conversions work meaningfully
  ImageDocument doc;
  doc.image_data().create(64, 64, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);
  canvas.set_zoom_factor(1.0);
  canvas.set_view_offset(QPoint(0, 0));
  QTest::qWait(50);

  // Cursor at canvas position (100, 50)
  QPointF cursor_pos(100, 50);
  QPoint image_before = canvas.canvas_to_image(QPoint(100, 50));

  // Ctrl+Wheel up (positive y delta = zoom in)
  QWheelEvent event(cursor_pos, cursor_pos, QPoint(0, 0),
                    QPoint(0, 120), Qt::NoButton, Qt::ControlModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  // Zoom should now be ~1.1
  EXPECT_GT(canvas.zoom_factor(), 1.0);

  // The image pixel that was under cursor should still be under cursor
  QPoint image_after = canvas.canvas_to_image(QPoint(100, 50));
  EXPECT_EQ(image_after, image_before);
}

TEST_F(ImageCanvasTest, CtrlWheelZoomOutKeepsPixelUnderCursor) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  doc.image_data().create(64, 64, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);
  canvas.set_zoom_factor(2.0);
  canvas.set_view_offset(QPoint(0, 0));
  QTest::qWait(50);

  QPointF cursor_pos(100, 50);
  QPoint image_before = canvas.canvas_to_image(QPoint(100, 50));

  // Ctrl+Wheel down (negative y delta = zoom out)
  QWheelEvent event(cursor_pos, cursor_pos, QPoint(0, 0),
                    QPoint(0, -120), Qt::NoButton, Qt::ControlModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  // Zoom should now be less than 2.0
  EXPECT_LT(canvas.zoom_factor(), 2.0);

  // Same pixel should still be under cursor
  QPoint image_after = canvas.canvas_to_image(QPoint(100, 50));
  EXPECT_EQ(image_after, image_before);
}

TEST_F(ImageCanvasTest, ZoomClampedAtMinimum) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  canvas.set_zoom_factor(0.02);  // close to min

  QPointF cursor_pos(100, 100);
  QWheelEvent event(cursor_pos, cursor_pos, QPoint(0, 0),
                    QPoint(0, -120), Qt::NoButton, Qt::ControlModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  EXPECT_GE(canvas.zoom_factor(), 0.01);
}

TEST_F(ImageCanvasTest, WheelEventWithoutDocumentDoesNotCrash) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  // Ctrl+Wheel without document
  QWheelEvent zoom_event(QPointF(100, 100), QPointF(100, 100), QPoint(0, 0),
                         QPoint(0, 120), Qt::NoButton, Qt::ControlModifier,
                         Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &zoom_event);
  QTest::qWait(50);

  // Plain wheel without document
  QWheelEvent scroll_event(QPointF(100, 100), QPointF(100, 100), QPoint(0, 0),
                           QPoint(0, -120), Qt::NoButton, Qt::NoModifier,
                           Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &scroll_event);
  QTest::qWait(50);

  SUCCEED();
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.WheelEvent*:ImageCanvasTest.CtrlWheel*:ImageCanvasTest.ZoomClamped*'`
Expected: 6 FAIL (tests compile but assert failures since wheelEvent is no-op)

- [ ] **Step 3: Implement wheelEvent() in image_canvas.cc**

Replace the no-op `wheelEvent` in `src/widgets/image_canvas.cc` (line 169-171):

```cpp
void ImageCanvas::wheelEvent(QWheelEvent *event) {
  const QPoint delta = event->angleDelta();

  if (event->modifiers() & Qt::ControlModifier) {
    // Zoom anchored to cursor pixel
    QPoint canvas_pos = event->position().toPoint();
    QPoint image_pos = canvas_to_image(canvas_pos);

    double new_zoom = zoom_factor_;
    if (delta.y() > 0) {
      new_zoom = zoom_factor_ * 1.1;
    } else {
      new_zoom = zoom_factor_ / 1.1;
    }
    new_zoom = std::clamp(new_zoom, 0.01, 100.0);
    zoom_factor_ = new_zoom;

    QPoint new_canvas = image_to_canvas(image_pos);
    view_offset_ += (canvas_pos - new_canvas);
  } else if (event->modifiers() & Qt::ShiftModifier) {
    // Horizontal scroll
    int dx = delta.x() != 0 ? delta.x() : delta.y();
    view_offset_.rx() -= dx;
  } else {
    // Vertical scroll
    view_offset_.ry() -= delta.y();
  }

  update();
  QWidget::wheelEvent(event);
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build build -j$(nproc) && ./build/ImageJ_tests --gtest_filter='ImageCanvasTest.WheelEvent*:ImageCanvasTest.CtrlWheel*:ImageCanvasTest.ZoomClamped*'`
Expected: 6 PASS

- [ ] **Step 5: Run full test suite**

Run: `./build/ImageJ_tests`
Expected: all existing tests still pass

- [ ] **Step 6: Commit**

```bash
git add tests/widgets/image_canvas_test.cc src/widgets/image_canvas.cc docs/superpowers/specs/2026-05-24-wheel-zoom-scroll-design.md docs/superpowers/plans/2026-05-24-wheel-zoom-scroll.md
git commit -m "feat: add wheel zoom and scroll to ImageCanvas"
```
