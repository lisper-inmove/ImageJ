#include "image_canvas/ImageCanvas.h"
#include "image_canvas/ImageCanvasDetail.h"

#include <QResizeEvent>
#include <QFileDialog>

using namespace imgcanvas_detail;

void ImageCanvas::resizeEvent(QResizeEvent* ev) {
    size_ = ev->size();
    if (img_.isNull() || !ev->oldSize().isValid()) {
        QWidget::resizeEvent(ev);
        notifyViewChanged();
        return;
    }
    const double bs_old = baseScale(ev->oldSize());
    const double bs_new = baseScale(ev->size());
    const double s_old  = bs_old * zoom_;
    const double s_new  = bs_new * zoom_;

    const QPointF cbo_old = centerBaseOrigin(ev->oldSize(), img_.size(), s_old);
    const QPointF cbo_new = centerBaseOrigin(ev->size(),    img_.size(), s_new);

    QPointF wantedOffset = offset_ + (cbo_old - cbo_new);
    wantedOffset = clampOffsetForImage(ev->size(), img_.size(), s_new, wantedOffset);
    offset_ = wantedOffset;

    QWidget::resizeEvent(ev);
    notifyViewChanged();
}

double ImageCanvas::baseScale(const QSize& widgetSize) const {
    if (img_.isNull() || img_.width() == 0 || img_.height() == 0) return 1.0;
    const double sx = double(widgetSize.width())  / img_.width();
    const double sy = double(widgetSize.height()) / img_.height();
    return std::min(sx, sy);
}

double ImageCanvas::effectiveScale() const {
    return baseScale(size()) * zoom_;
}


void ImageCanvas::resetView() {
    if (img_.isNull()) return;
    zoom_   = 1.0;
    offset_ = QPointF(0, 0);
    update();
    notifyViewChanged();
}

void ImageCanvas::fitToWindow() {
    resetView();
}

void ImageCanvas::zoomActualPixels() {
    if (img_.isNull()) return;
    const double bs = baseScale(size());
    if (bs > 0.0) {
        zoom_ = 1.0 / bs;               // 1:1 像素
        offset_ = QPointF(0, 0);
        const double s = baseScale(size()) * zoom_;
        offset_ = imgcanvas_detail::clampOffsetForImage(size(), img_.size(), s, offset_);
        update();
        notifyViewChanged();
    }
}

void ImageCanvas::notifyViewChanged() {
    emit viewChanged(zoom_, offset_, effectiveScale());
}

