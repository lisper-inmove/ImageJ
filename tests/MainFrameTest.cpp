#include <gtest/gtest.h>

#include <QTest>

#include "frames/MainFrame.h"

TEST(UiSmoke, MainFrameShowsAndHides) {
    MainFrame w;
    w.show();
    // 给 Qt 事件队列一点时间处理 show 事件
    QTest::qWait(200);
    EXPECT_TRUE(w.isVisible());
    // 可以再做点基本断言：尺寸是否有效
    EXPECT_GT(w.size().width(), 0);
    EXPECT_GT(w.size().height(), 0);
    w.hide();
    QTest::qWait(10);
    EXPECT_FALSE(w.isVisible());
}
