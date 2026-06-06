#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QWheelEvent>

#include "widgets/image_canvas.h"
#include "core/image_document.h"
#include "core/image_document_adapter.h"

class ImageCanvasTest : public ::testing::Test {
 protected:
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

TEST_F(ImageCanvasTest, DefaultBackgroundStyleIsSolidColor) {
  ImageCanvas canvas;
  EXPECT_EQ(canvas.background_style(), ImageCanvas::BackgroundStyle::kSolidColor);
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
  EXPECT_EQ(canvas.background_color(), QColor(0x2D, 0x2D, 0x2D));
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

// === Paint tests ===

TEST_F(ImageCanvasTest, PaintEventDoesNotCrash) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  canvas.update();
  QTest::qWait(50);
  canvas.repaint();
  QTest::qWait(50);

  SUCCEED();
}

TEST_F(ImageCanvasTest, PaintEventWithDifferentSizes) {
  ImageCanvas canvas;
  canvas.show();
  QTest::qWait(50);

  QList<QSize> sizes = {{100, 100}, {200, 50}, {50, 200}, {1, 1}, {800, 600}};
  for (const auto& size : sizes) {
    canvas.resize(size);
    canvas.repaint();
    QTest::qWait(10);
    SUCCEED();
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

  SUCCEED();
}

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

// === Coordinate conversion tests ===

TEST_F(ImageCanvasTest, ImageToCanvasAtDefaultZoom) {
  ImageCanvas canvas;
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

  QPoint canvas_pt = canvas.image_to_canvas(QPoint(0, 0));
  EXPECT_EQ(canvas_pt, QPoint(10, 20));

  QPoint image_pt = canvas.canvas_to_image(QPoint(10, 20));
  EXPECT_EQ(image_pt, QPoint(0, 0));
}

// === Document binding tests ===

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

  received_doc = nullptr;
  canvas.set_document(&doc);
  EXPECT_EQ(received_doc, &doc);
}

TEST_F(ImageCanvasTest, OnDocumentModifiedTriggersUpdate) {
  ImageCanvas canvas;
  canvas.show();
  QTest::qWait(50);

  canvas.on_document_modified();
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
  SUCCEED();
}

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

  QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(50);

  EXPECT_EQ(received_pos, QPoint(50, 50));
}

TEST_F(ImageCanvasTest, RightClickEmitsImageClicked) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  Qt::MouseButton received_btn = Qt::NoButton;
  QObject::connect(&canvas, &ImageCanvas::image_clicked,
                   [&](const QPoint& pos, Qt::MouseButton btn) {
                     received_btn = btn;
                   });

  QTest::mouseClick(&canvas, Qt::RightButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(50);

  EXPECT_EQ(received_btn, Qt::RightButton);
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

  QTest::mouseMove(&canvas, QPoint(80, 120));
  QTest::qWait(50);

  // Signal should have been emitted with some position (not the default)
  EXPECT_NE(received_pos, QPoint(-1, -1));
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

  canvas.resize(500, 400);
  QTest::qWait(50);

  bool clicked = false;
  QObject::connect(&canvas, &ImageCanvas::image_clicked,
                   [&](const QPoint&, Qt::MouseButton) { clicked = true; });

  QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(250, 200));
  QTest::qWait(50);

  EXPECT_TRUE(clicked);
}

// === Image rendering tests ===

TEST_F(ImageCanvasTest, PaintEventWithDocumentNoCrash) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  doc.image_data().create(32, 32, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);
  QTest::qWait(50);

  canvas.repaint();
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, PaintEventWithImageData) {
  ImageCanvas canvas;
  canvas.resize(128, 128);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  ImageDocumentAdapter adapter(&doc);

  QImage src(16, 16, QImage::Format_RGB888);
  src.fill(Qt::red);
  adapter.update_from_qimage(src);

  canvas.set_document(&doc);
  QTest::qWait(50);

  canvas.repaint();
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, PaintEventWithZoomAndImage) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  ImageDocumentAdapter adapter(&doc);

  QImage src(8, 8, QImage::Format_RGB888);
  src.fill(Qt::blue);
  adapter.update_from_qimage(src);

  canvas.set_document(&doc);
  canvas.set_zoom_factor(4.0);
  QTest::qWait(50);

  canvas.repaint();
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, PaintEventWithOffsetAndImage) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  ImageDocumentAdapter adapter(&doc);

  QImage src(32, 32, QImage::Format_RGB888);
  src.fill(Qt::green);
  adapter.update_from_qimage(src);

  canvas.set_document(&doc);
  canvas.set_view_offset(QPoint(50, 50));
  QTest::qWait(50);

  canvas.repaint();
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, PaintEventNullDocument) {
  ImageCanvas canvas;
  canvas.resize(100, 100);
  canvas.show();
  QTest::qWait(50);

  // Paint without document — should not crash
  canvas.repaint();
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, SetDocumentResetsViewOffset) {
  ImageCanvas canvas;
  canvas.set_view_offset(QPoint(50, 100));
  canvas.set_zoom_factor(2.0);

  ImageDocument doc;
  doc.image_data().create(32, 32, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);

  EXPECT_EQ(canvas.view_offset(), QPoint(0, 0));
}

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

  // Set view_offset to large positive — should be clamped to canvas bounds
  canvas.set_view_offset(QPoint(999, 999));
  EXPECT_GE(canvas.view_offset().x(), 0);
  EXPECT_GE(canvas.view_offset().y(), 0);
  EXPECT_LE(canvas.view_offset().x(), 100);
  EXPECT_LE(canvas.view_offset().y(), 100);
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
  // Scaled image is 200x200 in 100x100 canvas
  // offset range: [-(200-100), 0] = [-100, 0]
  canvas.set_view_offset(QPoint(-300, -300));
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

// === Constructor tests ===

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

  // Scrolling inward (negative delta) scrolls image down:
  // view_offset.y() decreases, revealing lower content
  EXPECT_LT(canvas.view_offset().y(), initial_offset.y());
  EXPECT_EQ(canvas.view_offset().x(), initial_offset.x());
}

TEST_F(ImageCanvasTest, WheelEventHorizontalScroll) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

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

  ImageDocument doc;
  doc.image_data().create(64, 64, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);
  canvas.set_zoom_factor(1.0);
  canvas.set_view_offset(QPoint(0, 0));
  QTest::qWait(50);

  QPointF cursor_pos(100, 50);
  QPoint image_before = canvas.canvas_to_image(QPoint(100, 50));

  QWheelEvent event(cursor_pos, cursor_pos, QPoint(0, 0),
                    QPoint(0, 120), Qt::NoButton, Qt::ControlModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  EXPECT_GT(canvas.zoom_factor(), 1.0);

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

  QWheelEvent event(cursor_pos, cursor_pos, QPoint(0, 0),
                    QPoint(0, -120), Qt::NoButton, Qt::ControlModifier,
                    Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &event);
  QTest::qWait(50);

  EXPECT_LT(canvas.zoom_factor(), 2.0);

  QPoint image_after = canvas.canvas_to_image(QPoint(100, 50));
  EXPECT_EQ(image_after, image_before);
}

TEST_F(ImageCanvasTest, ZoomClampedAtMinimum) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  canvas.set_zoom_factor(0.02);

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

  QWheelEvent zoom_event(QPointF(100, 100), QPointF(100, 100), QPoint(0, 0),
                         QPoint(0, 120), Qt::NoButton, Qt::ControlModifier,
                         Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &zoom_event);
  QTest::qWait(50);

  QWheelEvent scroll_event(QPointF(100, 100), QPointF(100, 100), QPoint(0, 0),
                           QPoint(0, -120), Qt::NoButton, Qt::NoModifier,
                           Qt::NoScrollPhase, false);
  QApplication::sendEvent(&canvas, &scroll_event);
  QTest::qWait(50);

  SUCCEED();
}

// === Selection tests ===

TEST_F(ImageCanvasTest, DefaultSelectionIsEmpty) {
  ImageCanvas canvas;
  EXPECT_FALSE(canvas.selection().isValid());
}

TEST_F(ImageCanvasTest, Ctrl1LeftClickWithoutDragSelectsPixel) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 150));
  QTest::qWait(50);

  QRect sel = canvas.selection();
  EXPECT_TRUE(sel.isValid());
  // Left-click at (100,150) in selection mode → normalized 1x1 rect
  EXPECT_EQ(sel, QRect(100, 150, 1, 1));
}

TEST_F(ImageCanvasTest, Ctrl1LeftClickDragCreatesSelection) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(150, 100));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(150, 100));
  QTest::qWait(50);

  QRect sel = canvas.selection();
  EXPECT_TRUE(sel.isValid());
  EXPECT_EQ(sel, QRect(50, 50, 101, 51));
}

TEST_F(ImageCanvasTest, SelectionChangedSignalEmitted) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  QRect received_rect;
  QObject::connect(&canvas, &ImageCanvas::selection_changed,
                   [&](const QRect& rect) { received_rect = rect; });

  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 20));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(100, 200));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 200));
  QTest::qWait(50);

  EXPECT_EQ(received_rect, QRect(10, 20, 91, 181));
}

TEST_F(ImageCanvasTest, LeftClickStillEmitsImageClicked) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QPoint received_pos(-1, -1);
  QObject::connect(&canvas, &ImageCanvas::image_clicked,
                   [&](const QPoint& pos, Qt::MouseButton) {
                     received_pos = pos;
                   });

  QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(77, 88));
  QTest::qWait(50);

  EXPECT_EQ(received_pos, QPoint(77, 88));
}

TEST_F(ImageCanvasTest, MouseMoveDuringSelectionNoHoverSignal) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  bool hover_emitted = false;
  QObject::connect(&canvas, &ImageCanvas::mouse_over_image,
                   [&](const QPoint&) { hover_emitted = true; });

  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(100, 100));
  QTest::qWait(20);

  EXPECT_FALSE(hover_emitted);
}

TEST_F(ImageCanvasTest, ClearSelectionWorks) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  // Create a selection first
  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(100, 100));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(50);

  EXPECT_TRUE(canvas.selection().isValid());

  canvas.clear_selection();
  EXPECT_FALSE(canvas.selection().isValid());
}

TEST_F(ImageCanvasTest, SelectionScalesWithZoom) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  canvas.set_zoom_factor(2.0);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(200, 200));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(200, 200));
  QTest::qWait(50);

  // Selection should be in image coords (half of canvas due to 2x zoom)
  QRect sel = canvas.selection();
  EXPECT_EQ(sel, QRect(50, 50, 51, 51));

  // Change zoom — selection stays in image coords
  canvas.set_zoom_factor(1.0);
  EXPECT_EQ(canvas.selection(), QRect(50, 50, 51, 51));
}

TEST_F(ImageCanvasTest, SelectionMovesWithPan) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(200, 200));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(200, 200));
  QTest::qWait(50);

  QRect sel = canvas.selection();
  EXPECT_EQ(sel, QRect(100, 100, 101, 101));

  // Pan — selection stays in image coords
  canvas.set_view_offset(QPoint(50, 50));
  EXPECT_EQ(canvas.selection(), QRect(100, 100, 101, 101));
}

TEST_F(ImageCanvasTest, PaintWithSelectionDoesNotCrash) {
  ImageCanvas canvas;
  canvas.resize(200, 200);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  // Create a selection
  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(150, 150));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(150, 150));
  QTest::qWait(50);

  // Repaint should not crash
  canvas.repaint();
  QTest::qWait(50);
  SUCCEED();
}

TEST_F(ImageCanvasTest, SecondSelectionReplacesFirst) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  // First selection
  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(50);

  EXPECT_EQ(canvas.selection(), QRect(10, 10, 41, 41));

  // Second selection
  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(200, 200));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(200, 200));
  QTest::qWait(50);

  EXPECT_EQ(canvas.selection(), QRect(100, 100, 101, 101));
}

TEST_F(ImageCanvasTest, SelectionClampedToImageBounds) {
  ImageCanvas canvas;
  canvas.resize(400, 400);
  canvas.show();
  QTest::qWait(50);

  ImageDocument doc;
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
  canvas.set_document(&doc);
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  // Drag beyond image bounds (from 50,50 to 200,200 — image is only 100x100)
  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(200, 200));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(200, 200));
  QTest::qWait(50);

  // Selection should be clamped to image rect (0,0 100x100)
  EXPECT_EQ(canvas.selection(), QRect(50, 50, 50, 50));
}

TEST_F(ImageCanvasTest, EscapeClearsCompletedSelection) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  // Create a selection first
  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(150, 150));
  QTest::qWait(20);
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(150, 150));
  QTest::qWait(50);

  EXPECT_TRUE(canvas.selection().isValid());

  // Press escape
  QKeyEvent key_event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
  QApplication::sendEvent(&canvas, &key_event);
  QTest::qWait(50);

  EXPECT_FALSE(canvas.selection().isValid());
}

TEST_F(ImageCanvasTest, EscapeClearsSelectionDuringDrag) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  QTest::keyClick(&canvas, Qt::Key_1, Qt::ControlModifier);
  QTest::qWait(20);

  // Start a selection drag
  QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
  QTest::qWait(20);
  QTest::mouseMove(&canvas, QPoint(100, 100));
  QTest::qWait(20);

  // Press escape mid-drag
  QKeyEvent key_event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
  QApplication::sendEvent(&canvas, &key_event);
  QTest::qWait(50);

  EXPECT_FALSE(canvas.selection().isValid());

  // Release left button — should not emit selection_changed
  QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
  QTest::qWait(50);

  EXPECT_FALSE(canvas.selection().isValid());
}

TEST_F(ImageCanvasTest, EscapeDoesNothingWithoutSelection) {
  ImageCanvas canvas;
  canvas.resize(300, 300);
  canvas.show();
  QTest::qWait(50);

  // Press escape with no selection — should not crash
  QKeyEvent key_event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
  QApplication::sendEvent(&canvas, &key_event);
  QTest::qWait(50);

  EXPECT_FALSE(canvas.selection().isValid());
  SUCCEED();
}
