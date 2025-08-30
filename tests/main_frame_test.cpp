#include <gtest/gtest.h>

#include <QApplication>
#include <QTest>
#include <QTimer>

#include "frames/MainFrame.h"
#include "image_canvas/ImageCanvas.h"
#include "test_header.h"

TEST(UiSmoke, MainFrameShowsAndHides) {
    MainFrame w;
    w.show();
    // 给 Qt 事件队列一点时间处理 show 事件
    QTest::qWait(2000);
    EXPECT_TRUE(w.isVisible());
    // 可以再做点基本断言：尺寸是否有效
    EXPECT_GT(w.size().width(), 0);
    EXPECT_GT(w.size().height(), 0);
    w.hide();
    QTest::qWait(10);
    EXPECT_FALSE(w.isVisible());
}

TEST(UiSmoke, Croppe) {
    MainFrame w;
    w.show();
    QTest::qWaitForWindowExposed(&w);
    auto canvas = w.findChild<ImageCanvas*>("ImageCanvas");
    ASSERT_NE(canvas, nullptr);

    const QString& filepath= makeTempPng(80, 40);
    canvas->openImageFromPath(filepath);
    QTest::qWait(500);

    QTest::mouseMove(canvas, QPoint(50, 50));
    canvas->setFocus();

    QTest::qWait(500);
    QTest::keyPress(canvas, Qt::Key_Control);
    QTest::mousePress(canvas, Qt::LeftButton);

    QTest::qWait(500);
    QTest::mouseMove(canvas, QPoint(90, 90));
    QTest::qWait(500);

    QTest::mouseRelease(canvas, Qt::LeftButton);
    QTest::keyRelease(canvas, Qt::Key_Control);

    // Optional: add any necessary waits if the GUI updates after this action
    QTest::qWait(100);  // Add appropriate wait time if needed

}
