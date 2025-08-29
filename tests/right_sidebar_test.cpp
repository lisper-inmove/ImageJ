#include <gtest/gtest.h>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QTest>
#include <QPushButton>  // 确保包含 QPushButton 头文件
#include <iostream>

#include "image_canvas/ImageCanvas.h"
#include "widgets/histogram/HistogramDialog.h"

#include "frames/MainFrame.h"
#include "frames/RightSidebar.h"
#include "test_header.h"

TEST(ImageCanvas, EmitShowHistogram) {
    MainFrame w;
    w.show();
    QTest::qWaitForWindowExposed(&w);

    // 获取 RightSidebar 控件
    auto canvas = w.findChild<ImageCanvas*>("ImageCanvas");
    ASSERT_NE(canvas, nullptr);

    const QString& filepath= makeTempPng(80, 40);
    canvas->openImageFromPath(filepath);
    QTest::qWait(300);

    // 获取 RightSidebar 控件
    auto sidebar = w.findChild<RightSidebar*>("RightSidebar");
    ASSERT_NE(sidebar, nullptr);

    QPushButton* btnHist = sidebar->findChild<QPushButton*>("btnHist_");
    ASSERT_NE(btnHist, nullptr) << "缺少 objectName=btnHist 的按钮";

    // 通过 QSignalSpy 监听 histogramRequested 信号
    QSignalSpy spy(sidebar, &RightSidebar::histogramRequested);

    // 模拟点击按钮
    QTest::mouseClick(btnHist, Qt::LeftButton);
    QTest::qWait(500);
    QCoreApplication::processEvents();  // 确保事件被处理
    auto hist_dlg = w.findChild<HistogramDialog*>("HistograDialog");

    // 断言信号是否被触发
    // ASSERT_TRUE(spy.wait(2000));  // 等待信号，最多 2 秒
    EXPECT_GT(spy.count(), 0);  // 确保信号被触发
    QCoreApplication::exit();
}
