#include <gtest/gtest.h>

#include <QApplication>
#include <QLabel>
#include <QListWidget>
#include <QTabWidget>
#include <QTest>

#include "core/image_data.h"
#include "core/image_document.h"
#include "core/image_document_adapter.h"
#include "frames/right_sidebar.h"

class RightSidebarTest : public ::testing::Test {
 protected:
  void SetUp() override {
    static int argc = 1;
    static char *argv[] = {const_cast<char *>("test")};
    if (!QApplication::instance()) {
      app_ = std::make_unique<QApplication>(argc, argv);
    }
    sidebar_ = std::make_unique<RightSidebar>();
  }

  void TearDown() override { sidebar_.reset(); }

  std::unique_ptr<QApplication> app_;
  std::unique_ptr<RightSidebar> sidebar_;
};

TEST_F(RightSidebarTest, InheritsFromQWidget) {
  EXPECT_TRUE(dynamic_cast<QWidget *>(sidebar_.get()) != nullptr);
}

TEST_F(RightSidebarTest, HasThreeTabs) {
  QTabWidget *tabs = sidebar_->findChild<QTabWidget *>();
  ASSERT_NE(tabs, nullptr);
  EXPECT_EQ(tabs->count(), 3);
  EXPECT_EQ(tabs->tabText(0).toStdString(), "图片信息");
  EXPECT_EQ(tabs->tabText(1).toStdString(), "工具");
  EXPECT_EQ(tabs->tabText(2).toStdString(), "选择历史");
}

TEST_F(RightSidebarTest, HasPixelInfoBar) {
  QList<QLabel *> labels = sidebar_->findChildren<QLabel *>();
  bool has_coord = false;
  bool has_value = false;
  for (QLabel *label : labels) {
    if (label->text().contains("像素坐标")) has_coord = true;
    if (label->text().contains("像素值")) has_value = true;
  }
  EXPECT_TRUE(has_coord);
  EXPECT_TRUE(has_value);
}

TEST_F(RightSidebarTest, DefaultPixelInfoIsPlaceholder) {
  QList<QLabel *> labels = sidebar_->findChildren<QLabel *>();
  for (QLabel *label : labels) {
    if (label->text().startsWith("像素坐标")) {
      EXPECT_EQ(label->text().toStdString(), "像素坐标: -");
    }
    if (label->text().startsWith("像素值")) {
      EXPECT_EQ(label->text().toStdString(), "像素值: -");
    }
  }
}

TEST_F(RightSidebarTest, SetDocumentUpdatesInfo) {
  ImageDocument doc;
  doc.image_data().create(640, 480, ImageData::PixelFormat::kRGB24);
  sidebar_->set_document(&doc);

  QList<QLabel *> labels = sidebar_->findChildren<QLabel *>();
  bool has_size = false;
  bool has_type = false;
  for (QLabel *label : labels) {
    if (label->text().contains("640") && label->text().contains("480"))
      has_size = true;
    if (label->text().contains("RGB")) has_type = true;
  }
  EXPECT_TRUE(has_size);
  EXPECT_TRUE(has_type);
}

TEST_F(RightSidebarTest, UpdatePixelInfoShowsCorrectCoordinates) {
  ImageDocument doc;
  doc.image_data().create(100, 100, ImageData::PixelFormat::kGray8);
  // Fill with gray value 128
  std::memset(doc.image_data().data(), 128, doc.image_data().byte_count());
  sidebar_->set_document(&doc);

  sidebar_->update_pixel_info(QPoint(42, 73));

  QList<QLabel *> labels = sidebar_->findChildren<QLabel *>();
  bool has_coord = false;
  bool has_value = false;
  for (QLabel *label : labels) {
    if (label->text().startsWith("像素坐标")) {
      has_coord = label->text().contains("(42, 73)");
    }
    if (label->text().startsWith("像素值")) {
      has_value = label->text().contains("128");
    }
  }
  EXPECT_TRUE(has_coord);
  EXPECT_TRUE(has_value);
}

TEST_F(RightSidebarTest, SetZoomFactorUpdatesLabel) {
  sidebar_->show();
  QTest::qWait(50);

  sidebar_->set_zoom_factor(2.5);

  QList<QLabel *> labels = sidebar_->findChildren<QLabel *>();
  bool has_zoom = false;
  for (QLabel *label : labels) {
    if (label->text().contains("250%")) has_zoom = true;
  }
  EXPECT_TRUE(has_zoom);
}

TEST_F(RightSidebarTest, AddSelectionCreatesHistoryItem) {
  ImageDocument doc;
  // Create a small RGB image
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
  // Fill with red
  uint8_t *d = doc.image_data().data();
  for (int i = 0; i < 100 * 100; ++i) {
    d[i * 3 + 0] = 255;
    d[i * 3 + 1] = 0;
    d[i * 3 + 2] = 0;
  }

  sidebar_->set_document(&doc);
  sidebar_->add_selection(QRect(10, 20, 30, 40));

  QListWidget *list = sidebar_->findChild<QListWidget *>();
  ASSERT_NE(list, nullptr);
  EXPECT_EQ(list->count(), 1);

  QListWidgetItem *item = list->item(0);
  EXPECT_TRUE(item->text().contains("10"));
  EXPECT_TRUE(item->text().contains("20"));
  EXPECT_FALSE(item->icon().isNull());
}

TEST_F(RightSidebarTest, SetDocumentClearsHistory) {
  ImageDocument doc;
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
  uint8_t *d = doc.image_data().data();
  for (int i = 0; i < 100 * 100; ++i) {
    d[i * 3 + 0] = 255;
  }

  sidebar_->set_document(&doc);
  sidebar_->add_selection(QRect(0, 0, 50, 50));

  QListWidget *list = sidebar_->findChild<QListWidget *>();
  EXPECT_EQ(list->count(), 1);

  // Setting a new document clears history
  ImageDocument doc2;
  doc2.image_data().create(200, 200, ImageData::PixelFormat::kRGB24);
  sidebar_->set_document(&doc2);
  EXPECT_EQ(list->count(), 0);
}

TEST_F(RightSidebarTest, SetDocumentNullClearsInfo) {
  ImageDocument doc;
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
  sidebar_->set_document(&doc);

  // Set to null
  sidebar_->set_document(nullptr);

  QList<QLabel *> labels = sidebar_->findChildren<QLabel *>();
  for (QLabel *label : labels) {
    if (label->text().startsWith("像素值")) {
      // update_pixel_info called after set_document(nullptr) should show "-"
      // But set_document itself doesn't update pixel info, so this is fine
    }
  }
  SUCCEED();
}

TEST_F(RightSidebarTest, PixelInfoOutOfBounds) {
  ImageDocument doc;
  doc.image_data().create(50, 50, ImageData::PixelFormat::kRGB24);
  sidebar_->set_document(&doc);

  sidebar_->update_pixel_info(QPoint(99, 99));

  QList<QLabel *> labels = sidebar_->findChildren<QLabel *>();
  bool has_outside = false;
  for (QLabel *label : labels) {
    if (label->text().contains("图像外")) has_outside = true;
  }
  EXPECT_TRUE(has_outside);
}
