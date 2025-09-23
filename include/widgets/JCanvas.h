#pragma once
#include <QWidget>
#include <opencv2/opencv.hpp>


class JCanvas: public QWidget {
    Q_OBJECT
public:
    explicit JCanvas(QWidget* parent = nullptr);
    void open();
    void setSize(QSize& size);

private:
    QSize size_ {1080, 800};
    QString path_;
    cv::Mat cvImg_;
    QImage img_;
    double scale_ = 1;

private:
    void paintEvent(QPaintEvent* event) override;

private:
    // 中间画十字
    void drawCrosshair(QPainter& p);
    // 基础缩放
    double baseScale();
    // 实际缩放
    double effectiveScale();
    QPointF imageOrigin();
};
