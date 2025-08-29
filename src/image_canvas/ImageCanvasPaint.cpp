#include "image_canvas/ImageCanvas.h"
#include "image_canvas/ImageCanvasDetail.h"

#include <QPainter>

using namespace imgcanvas_detail;

void ImageCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (!img_.isNull()) {
        const double bs = baseScale(size());
        const double s  = bs * zoom_;

        const QPointF cbo = centerBaseOrigin(this->size(), img_.size(), s);
        const QPointF origin = cbo + offset_;

        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.save();
        p.translate(origin);
        p.scale(s, s);
        p.drawImage(QPointF(0, 0), img_);
        p.restore();
    }

    drawCrosshair(p);

    if (img_.isNull()) {
        p.setPen(QColor(220, 220, 220));
        QFont f = p.font(); f.setPointSizeF(f.pointSizeF() + 2); p.setFont(f);
        p.drawText(rect().adjusted(0, 30, 0, 0),
                   Qt::AlignHCenter | Qt::AlignTop,
                   QStringLiteral("单击选择图片…（也可拖拽图片到此处）"));
    }
}

void ImageCanvas::drawCrosshair(QPainter& p) {
    const int L = int(0.3 * std::min(width(), height()));

    QPoint center = QPoint(width()/2, height()/2);
    const bool hasImage = !img_.isNull();

    if (hasImage) {
        const double s  = baseScale(size()) * zoom_;
        const QPointF cbo = centerBaseOrigin(this->size(), img_.size(), s);
        const QPointF origin = cbo + offset_;
        const double w = img_.width()  * s;
        const double h = img_.height() * s;
        const QPointF imgCenter = origin + QPointF(w/2.0, h/2.0);
        center = imgCenter.toPoint();
    }

    QPen pen(hasImage ? Qt::red : Qt::white);
    pen.setWidth(2);
    pen.setCosmetic(true);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(pen);

    p.drawLine(QPoint(center.x() - L/2, center.y()),
               QPoint(center.x() + L/2, center.y()));
    p.drawLine(QPoint(center.x(), center.y() - L/2),
               QPoint(center.x(), center.y() + L/2));

    p.setBrush(hasImage ? Qt::red : Qt::white);
    p.drawEllipse(center, 3, 3);
}
