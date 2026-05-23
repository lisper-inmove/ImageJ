#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

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
  EXPECT_EQ(canvas.background_color(), Qt::white);
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
