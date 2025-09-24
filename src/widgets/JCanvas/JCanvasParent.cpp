#include "widgets/JCanvas.h"
#include "utils/logger.h"
#include <QPainter>
#include <QWheelEvent>

void JCanvas::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    if (!img_.isNull()) {
        double scale = effectiveScale();
        const QPointF origin = imageOrigin() + offset_;
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.save();
        // 设置QPainter坐标系统原点
        p.translate(origin);
        // 画布与图片做一样的缩放
        p.scale(scale, scale);
        // 在画布上展示图片(画布与图片其实是完全重合的)
        p.drawImage(0, 0, img_);
    }
    drawCrosshair(p);
}

void JCanvas::wheelEvent(QWheelEvent* event) {
    if (img_.isNull()) {
        event->ignore();
        return;
    }
    // 鼠标当前位置
    const QPointF m = event->position();
    double oldScale = effectiveScale();
    // 鼠标滚轮移动的角度。y: Vertical scroll amount. x: Horizontal scroll amount
    const int delta = event->angleDelta().y();
    const QPointF originOld = imageOrigin() + offset_;
    if (delta > 0) scale_ *= 1.1;
    else if (delta < 0) scale_ /= 1.1;

    scale_ = std::clamp(scale_, 0.1, 10.0);
    double newScale = effectiveScale();

    // 新的鼠标位置
    const QPointF u = (m - originOld) / oldScale;
    const QPointF cboNew = imageOrigin();
    const QPointF originNew = m - u * newScale;
    QPointF wanted = originNew - cboNew;

    offset_ = clampOffsetForImage(wanted);
    LOG_DEBUG("New Offset: ({}, {})", offset_.x(), offset_.y());
    update();
    event->accept();
}

void JCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (img_.isNull()) {
        event->ignore();
        return;
    }
    if (draggling_) {
        QPointF pos = event->position();
        QPointF delta = pos.toPoint() - prev_pos_;
        QPointF wanted = delta + offset_;
        offset_ = clampOffsetForImage(wanted);
        prev_pos_ = event->position();
        update();
        QWidget::mouseMoveEvent(event);
    }
}

void JCanvas::mousePressEvent(QMouseEvent* event) {
    setCursor(Qt::ClosedHandCursor);
    prev_pos_ = event->position();
    draggling_ = true;
}

void JCanvas::mouseReleaseEvent(QMouseEvent* event) {
    setCursor(Qt::CrossCursor);
    prev_pos_ = event->position();
    draggling_ = false;
}

QPointF JCanvas::clampOffsetForImage(const QPointF& desiredOffset) {
    /**
        图片中心与视窗的中心的偏移。用图片的中心减去视窗的中心
    */
    double scale = effectiveScale();
    QSize widgetSize = size();
    QSize imgSize = img_.size();

    const double imageCenterX = (imgSize.width() * scale) / 2.0;
    const double imageCenterY = (imgSize.height() * scale) / 2.0;
    const double frameCenterX = widgetSize.width() / 2.0;
    const double frameCenterY = widgetSize.height() / 2.0;
    const double dx = std::max(0.0, (imageCenterX - frameCenterX));
    const double dy = std::max(0.0, (imageCenterY - frameCenterY));

    const double ox = std::clamp(desiredOffset.x(), -dx, dx);
    // clamp(value, low, high): 把value规范到 [low, high] 以内
    const double oy = std::clamp(desiredOffset.y(), -dy, dy);
    return QPointF(ox, oy);
}

