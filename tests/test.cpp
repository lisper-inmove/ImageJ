#include <gtest/gtest.h>
#include <QApplication>
#include "frames/main_frame.h"

TEST(MainFrameTest, ConstructAndShow) {
    int argc = 0;
    QApplication app(argc, nullptr);

    MainFrame w;
    w.show();

    EXPECT_EQ(w.windowTitle().toStdString(), "ImageJ");
    EXPECT_TRUE(w.isVisible());
}
