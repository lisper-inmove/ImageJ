#pragma once

#include <QString>
#include <QTemporaryDir>
#include <QPainter>

static QString makeTempPng(int w=64, int h=48) {
    QTemporaryDir dir;
    // QTemporaryDir 生命周期要延长；这里为了示例简单，改用固定临时路径更保险
    // 生产建议：把 dir 作为类成员保持到测试结束
    const QString path = QDir::temp().filePath("unit_test_img.png");
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setPen(Qt::black);
    p.drawText(QPoint(5, 20), "TEST");
    p.end();
    img.save(path);
    return path;
}
