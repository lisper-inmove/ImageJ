#pragma once

#include <memory>

#include <QResizeEvent>
#include <QWidget>
#include <QRubberBand>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>


class ImageCanvas : public QWidget {
    Q_OBJECT
public:
    explicit ImageCanvas(QWidget* parent = nullptr);
    void openImage();
    void displayHistogram();
    void cancel();

private:
    QSize size_{ 1080, 800 };
    cv::Mat cv_img_;
    QImage img_;
    double scale_{ 1.0 };
    QPointF offset_{0.0, 0.0};
    bool dragging_{ false };
    QPoint lastPos_;

    // 选择部分区域
    QPoint selectStart_;
    QPoint selectEnd_;
    bool selecting_ {false};
    std::unique_ptr<QRubberBand> rb_;

    // 灰度直方图
    bool hSelecting_{false};

private:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent*) override;
    void wheelEvent(QWheelEvent*) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void chooseImage();
    bool loadImage(const QString path);
    void drawCrosshair(QPainter& p);
    double baseScale(const QSize& widgetSize) const;
    double effectiveScale() const;
    void selectRect(QMouseEvent* event);
    void onSelectFinish();
	QPointF toImgCoord(const QPoint& widgetCoord) const;
    QPointF imageOrigin() const;
    QPointF clampOffsetForImage(const QPointF& desiredOffset);

signals:
    void imageInfoChanged(const QString& path, const QSize& size);
	void mouseMoved(const QPoint& pos, const QPointF& imgPos);
};
