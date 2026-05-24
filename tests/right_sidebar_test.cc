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

TEST_F(RightSidebarTest, SetZoomFactorUpdatesLabel) {
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
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
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

  ImageDocument doc2;
  doc2.image_data().create(200, 200, ImageData::PixelFormat::kRGB24);
  sidebar_->set_document(&doc2);
  EXPECT_EQ(list->count(), 0);
}

TEST_F(RightSidebarTest, SetDocumentNullClearsInfo) {
  ImageDocument doc;
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
  sidebar_->set_document(&doc);
  sidebar_->set_document(nullptr);
  SUCCEED();
}

TEST_F(RightSidebarTest, SelectionListUsesListMode) {
  QListWidget *list = sidebar_->findChild<QListWidget *>();
  ASSERT_NE(list, nullptr);
  EXPECT_EQ(list->viewMode(), QListView::ListMode);
}

TEST_F(RightSidebarTest, NewestSelectionIsFirst) {
  ImageDocument doc;
  doc.image_data().create(100, 100, ImageData::PixelFormat::kRGB24);
  uint8_t *d = doc.image_data().data();
  for (int i = 0; i < 100 * 100; ++i) {
    d[i * 3 + 0] = 255;
  }

  sidebar_->set_document(&doc);
  sidebar_->add_selection(QRect(0, 0, 10, 10));
  sidebar_->add_selection(QRect(20, 20, 30, 30));

  QListWidget *list = sidebar_->findChild<QListWidget *>();
  ASSERT_NE(list, nullptr);
  EXPECT_EQ(list->count(), 2);
  // Newest selection (20,20) should be at index 0
  EXPECT_TRUE(list->item(0)->text().contains("20"));
  EXPECT_TRUE(list->item(1)->text().contains("0,0"));
}
