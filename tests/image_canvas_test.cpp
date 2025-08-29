#include <gtest/gtest.h>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QTest>
#include <iostream>

#include "image_canvas/ImageCanvas.h"   // 如果路径/名字不同，改成你的
#include "test_header.h"
// 或者 #include "widgets/ImageCanvas.h" 视你实际类名/路径

TEST(ImageCanvas, EmitsImageInfoChanged) {
    ImageCanvas canvas;
    canvas.setObjectName("ImageCanvas");
    canvas.resize(640, 480);
    canvas.show();
    const QString& filepath= makeTempPng(80, 40);
    QTest::qWait(10);
    QSignalSpy infoSpy(&canvas, &ImageCanvas::imageInfoChanged);
    canvas.openImageFromPath(filepath);
    infoSpy.wait(2000);
    ASSERT_GE(infoSpy.count(), 1);
}

TEST(ImageCanvas, EmitsMousPos) {
    ImageCanvas canvas;
    canvas.setObjectName("ImageCanvas");
    canvas.resize(640, 480);
    canvas.show();

    QTest::qWait(30);
    QSignalSpy infoSpy(&canvas, &ImageCanvas::mousePositionChanged);

    const QString& filepath= makeTempPng(80, 40);
    canvas.openImageFromPath(filepath);
    QTest::qWait(300);

    QTest::mouseMove(&canvas, QPoint(10, 10));
    QTest::qWait(300);
    QTest::mouseMove(&canvas, QPoint(20, 20));
    QTest::qWait(100);
    QTest::mouseMove(&canvas, QPoint(100, 120));

    const int timeoutMs = 1000, sliceMs = 20;
    int waited = 0;
    while (infoSpy.count() < 1 && waited < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, sliceMs);
        QThread::msleep(sliceMs);
        waited += sliceMs;
    }

    ASSERT_GE(infoSpy.count(), 1) << "mousePositionChanged not emitted";

    // 取最后一次参数，做点断言（可选）
    const auto args = infoSpy.takeLast();
    // args[0]: QPoint widgetPos, args[1]: QPointF imagePos, args[2]: bool inside
    EXPECT_TRUE(args.size() == 3);
    EXPECT_TRUE(args.at(0).toPoint().x() >= 0);
}
