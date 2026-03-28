#include <gtest/gtest.h>

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include "frames/right_sidebar.h"

class RightSidebarTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char* argv[] = {nullptr};
        app_ = std::make_unique<QApplication>(argc, argv);
        sidebar_ = std::make_unique<RightSidebar>();
    }

    void TearDown() override {
        sidebar_.reset();
        app_.reset();
    }

    std::unique_ptr<QApplication> app_;
    std::unique_ptr<RightSidebar> sidebar_;
};

TEST_F(RightSidebarTest, InheritsFromQWidget) {
    // RightSidebar应该是QWidget的子类
    EXPECT_TRUE(dynamic_cast<QWidget*>(sidebar_.get()) != nullptr);
}

TEST_F(RightSidebarTest, ShowsPlaceholderText) {
    // 查找QLabel并验证文本
    sidebar_->show();

    // 查找子控件中的QLabel
    QLabel* label = sidebar_->findChild<QLabel*>();
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->text().toStdString(), "工具选项");
}