#include "image_canvas/ImageCanvas.h"
#include "image_canvas/ImageCanvasDetail.h"

#include <QWheelEvent>
#include <cmath>

using namespace imgcanvas_detail;

void ImageCanvas::wheelEvent(QWheelEvent* ev) {
    if (img_.isNull()) { ev->ignore(); return; }

    const QPointF m = ev->position();          // Qt6: position()
    const double bs = baseScale(size());
    const double s_old = bs * zoom_;

    const QPointF cbo_old = centerBaseOrigin(this->size(), img_.size(), s_old);
    const QPointF origin_old = cbo_old + offset_;

    const double steps = ev->angleDelta().y() / 120.0;
    if (steps == 0.0) { ev->ignore(); return; }

    const double factor = std::pow(1.2, steps);
    double newZoom = zoom_ * factor;
    newZoom = std::clamp(newZoom, 0.05, 50.0);

    const double s_new = bs * newZoom;

    // 保持鼠标处图像点位置不变
    const QPointF u = (m - origin_old) / s_old;
    const QPointF cbo_new = centerBaseOrigin(this->size(), img_.size(), s_new);
    const QPointF origin_new = m - u * s_new;
    QPointF wantedOffset = origin_new - cbo_new;

    wantedOffset = clampOffsetForImage(size(), img_.size(), s_new, wantedOffset);

    offset_ = wantedOffset;
    zoom_   = newZoom;

    update();
    notifyViewChanged();
    notifyMousePos(ev->position().toPoint());
    ev->accept();
}
