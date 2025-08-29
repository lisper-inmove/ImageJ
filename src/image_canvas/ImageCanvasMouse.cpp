#include "image_canvas/ImageCanvas.h"
#include "image_canvas/ImageCanvasDetail.h"

#include <QMouseEvent>
#include <QRubberBand>

using namespace imgcanvas_detail;

void ImageCanvas::mousePressEvent(QMouseEvent* ev) {
    const bool ctrl = ev->modifiers() & Qt::ControlModifier;
    const bool left = (ev->button() == Qt::LeftButton);
    if (ctrl && left && !img_.isNull()) {
        // Ctrl + left + 图片不为空: 区域选择
        selecting_ = true;
        selStartW_ = ev->pos();
        selEndW_   = selStartW_;
        if (!rb_) rb_ = std::make_unique<QRubberBand>(QRubberBand::Rectangle, this);
        rb_->setGeometry(QRect(selStartW_, QSize()));
        rb_->show();
        ev->accept();
        return;
    } else {
        if (img_.isNull()) {
            // 没有图片的时候，选择图片
            openImage();
        } else {
            // 有图片的时候 拖动图片
            dragging_ = true;
            lastPos_  = ev->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }
    QWidget::mousePressEvent(ev);
}

void ImageCanvas::mouseMoveEvent(QMouseEvent* ev) {
    if (selecting_ && rb_) {
        selEndW_ = ev->pos();
        rb_->setGeometry(QRect(selStartW_, selEndW_).normalized());
        ev->accept();
        return;
    }

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
    if (selecting_) {
        selecting_ = false;
        if (rb_) rb_->hide();

        QRect wr = QRect(selStartW_, selEndW_).normalized();
        QRect ir = widgetRectToImageRect_(wr);
        if (ir.width() > 0 && ir.height() > 0 && !img_.isNull()) {
            QImage cropped = img_.copy(ir);
            pushSelection_(cropped);
            emit selectionCreated(cropped);
        }

        ev->accept();
        return;
    }
    if (ev->button() == Qt::LeftButton && dragging_) {
        dragging_ = false;
        setCursor(Qt::CrossCursor);
        ev->accept();
        return;
    }
    QWidget::mouseReleaseEvent(ev);
}

void ImageCanvas::notifyMousePos(const QPoint& widgetPos) {
    if (img_.isNull()) {
        emit mousePositionChanged(widgetPos, QPointF(-1, -1), false);
        return;
    }
    const double s  = baseScale(size()) * zoom_;
    const QPointF cbo = centerBaseOrigin(this->size(), img_.size(), s);
    const QPointF origin = cbo + offset_;
    const QPointF u = (QPointF(widgetPos) - origin) / s; // 图像坐标
    const bool inside = (u.x() >= 0 && u.y() >= 0 &&
                         u.x() < img_.width() && u.y() < img_.height());
    emit mousePositionChanged(widgetPos, u, inside);
}
