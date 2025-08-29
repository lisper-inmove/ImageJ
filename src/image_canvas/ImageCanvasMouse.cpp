#include "image_canvas/ImageCanvas.h"
#include "image_canvas/ImageCanvasDetail.h"

#include <QMouseEvent>

using namespace imgcanvas_detail;

void ImageCanvas::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton) {
        if (img_.isNull()) {
            openImage(); // 没图时才允许打开
        } else {
            dragging_ = true;
            lastPos_  = ev->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }
    QWidget::mousePressEvent(ev);
}

void ImageCanvas::mouseMoveEvent(QMouseEvent* ev) {
    if (dragging_ && !img_.isNull()) {
        const QPoint delta = ev->pos() - lastPos_;
        lastPos_ = ev->pos();

        const double s = baseScale(size()) * zoom_;
        const QPointF wanted = offset_ + delta;
        offset_ = clampOffsetForImage(size(), img_.size(), s, wanted);

        update();
        notifyViewChanged();
        notifyMousePos(ev->pos());
        ev->accept();
        return;
    }
    // 非拖拽也实时通报鼠标位置
    notifyMousePos(ev->pos());
    QWidget::mouseMoveEvent(ev);
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton && dragging_) {
        dragging_ = false;
        setCursor(Qt::CrossCursor);
        ev->accept();
        return;
    }
    QWidget::mouseReleaseEvent(ev);
}
