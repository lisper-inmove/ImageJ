#include <gtest/gtest.h>
#include <QSplitter>
#include <QApplication>

#include "frames/main_frame.h"
#include "widgets/image_canvas.h"
#include "frames/right_sidebar.h"

class MainFrameTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char* argv[] = {nullptr};
        app_ = std::make_unique<QApplication>(argc, argv);
        frame_ = std::make_unique<MainFrame>();
    }

    void TearDown() override {
        frame_.reset();
        app_.reset();
    }

    std::unique_ptr<QApplication> app_;
    std::unique_ptr<MainFrame> frame_;
};

TEST_F(MainFrameTestFixture, HasRequiredMethods) {
    // Test that MainFrame can be instantiated
    // This will fail to link until all methods are implemented
    EXPECT_NE(frame_.get(), nullptr);
}

TEST_F(MainFrameTestFixture, HasSplitterLayout) {
    frame_->show();

    // 验证splitter存在
    QSplitter* splitter = frame_->findChild<QSplitter*>();
    ASSERT_NE(splitter, nullptr);

    // 验证splitter有两个子控件
    EXPECT_EQ(splitter->count(), 2);

    // 验证左侧是ImageCanvas
    ImageCanvas* canvas = qobject_cast<ImageCanvas*>(splitter->widget(0));
    EXPECT_NE(canvas, nullptr);

    // 验证右侧是RightSidebar
    RightSidebar* sidebar = qobject_cast<RightSidebar*>(splitter->widget(1));
    EXPECT_NE(sidebar, nullptr);
}