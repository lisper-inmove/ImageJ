#include "image_canvas/ImageCanvas.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QUrl>

void ImageCanvas::dragEnterEvent(QDragEnterEvent* ev) {
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    } else {
        ev->ignore();
    }
}

void ImageCanvas::dropEvent(QDropEvent* ev) {
    if (!ev->mimeData()->hasUrls()) {
        ev->ignore();
        return;
    }
    const auto urls = ev->mimeData()->urls();
    for (const QUrl& url : urls) {
        const QString path = url.toLocalFile();
        if (loadImage(path)) {
            ev->acceptProposedAction();
            return;
        }
    }
    ev->ignore();
}

QPointF ImageCanvas::widgetToImage_(const QPoint& wpos) const {
    /**
        将视窗坐标转换成图片坐标
    */

    const double s = effectiveScale();
    const double sw = img_.size().width() * s;
    const double sh = img_.size().height() * s;
    const QPointF center(size_.width() * 0.5, size_.height() * 0.5);
    const QPointF offset = center - QPointF(sw * 0.5, sh * 0.5) + offset_;
    return (QPointF(wpos) - offset) / s;
}

QRect ImageCanvas::widgetRectToImageRect_(QRect wr) const {
    wr = wr.normalized();
    QPointF p0 = widgetToImage_(wr.topLeft());
    QPointF p1 = widgetToImage_(wr.bottomRight());
    // 转成整像素并裁剪到图像边界
    int x0 = qFloor(qMin(p0.x(), p1.x()));
    int y0 = qFloor(qMin(p0.y(), p1.y()));
    int x1 = qCeil (qMax(p0.x(), p1.x()));
    int y1 = qCeil (qMax(p0.y(), p1.y()));
    QRect r(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
    if (!img_.isNull()) {
        r = r.intersected(QRect(0, 0, img_.width(), img_.height()));
    } else {
        r = QRect();
    }
    return r;
}
