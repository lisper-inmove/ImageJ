#include "widgets/JCanvas.h"
#include "utils/logger.h"
#include <memory>
#include <QPainter>
#include <QWheelEvent>
#include <QRect>

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
    QPointF pos = event->position();
    if (draggling_) {
        QPointF delta = pos.toPoint() - prevPos_;
        QPointF wanted = delta + offset_;
        offset_ = clampOffsetForImage(wanted);
        prevPos_ = event->position();
        update();
    }
    if (selecting_) {
        QPointF imgPoint = toImageCoord(event->pos());
        if (imgPoint.x() != -1) {
            endPos_ = event->pos();
        }
        rb_->setGeometry(QRect(startPos_, endPos_));
    }
    event->accept();
    QWidget::mouseMoveEvent(event);
}

void JCanvas::mousePressEvent(QMouseEvent* event) {
    const bool isCtrl = event->modifiers() & Qt::ControlModifier;
    const bool isLeft = event->button() & Qt::LeftButton;
    prevPos_ = event->position();
    startPos_ = event->pos();
    endPos_ = event->pos();
    if (isLeft) {
        setCursor(Qt::ClosedHandCursor);
    }
    if (isCtrl && isLeft) {
        selecting_ = true;
        if (!rb_)
            rb_ = std::make_unique<QRubberBand>(QRubberBand::Rectangle, this);
        rb_->setGeometry(QRect(startPos_, QSize()));
        rb_->show();
    } else {
        draggling_ = true;
    }
}

void JCanvas::mouseReleaseEvent(QMouseEvent* event) {
    setCursor(Qt::CrossCursor);
    prevPos_ = event->position();
    endPos_ = event->pos();
    if (selecting_) {
        onSelectFinish();
    }
    draggling_ = false;
    selecting_ = false;

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

