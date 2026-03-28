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