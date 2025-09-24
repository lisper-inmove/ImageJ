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
    double scale_{1};
    QPointF offset_{0.0, 0.0};
    QPointF prev_pos_;
    bool draggling_ = false;

private:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    // 中间画十字
    void drawCrosshair(QPainter& p);
    // 基础缩放
    double baseScale();
    // 实际缩放
    double effectiveScale();
    // 新打开一张图片时，将所有参数都重置
    void reset();
    QPointF imageOrigin();
    QPointF clampOffsetForImage(const QPointF& desiredOffset);
};
