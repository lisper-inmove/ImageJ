#include <gtest/gtest.h>
#include <QAction>
#include <QFileDialog>
#include <QImage>
#include <QTest>
#include "frames/MainFrame.h"

TEST(UiSmoke, MainFrameShowsAndHides) {
    MainFrame w;
    w.show();  // 显示窗口

    // 给 Qt 事件队列一点时间处理 show 事件
    QTest::qWait(2000);
    EXPECT_TRUE(w.isVisible());
    // 基本尺寸验证
    EXPECT_GT(w.size().width(), 0);
    EXPECT_GT(w.size().height(), 0);

    QMenuBar* menuBar = w.menuBar();
    ASSERT_TRUE(menuBar != nullptr);

    QMenu* fMenu = menuBar->findChild<QMenu*>("File");
    ASSERT_TRUE(fMenu != nullptr);  // 确保找到了 File 菜单

    // 查找 'Open' 菜单项并模拟点击
    QAction* openAction = fMenu->findChild<QAction*>("act_open_");
    ASSERT_TRUE(openAction != nullptr);  // 确保找到了正确的 QAction

    // 通过 trigger() 方法模拟点击 'Open' 菜单项
    openAction->trigger();
    QTest::qWait(1000);  // 等待文件对话框弹出
}
