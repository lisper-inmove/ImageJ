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

void ImageCanvas::chooseImage() {
    const QString filter = QStringLiteral(
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;All files (*)");
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择图片"), QString(), filter);
    if (!path.isEmpty()) {
        loadImage(path);
    }
}

bool ImageCanvas::loadImage(const QString& path) {
    QImage tmp;
    if (!tmp.load(path)) {
        return false;
    }
    img_ = std::move(tmp);
    imagePath_ = path;
    zoom_   = 1.0;
    offset_ = QPointF(0, 0);
    update();

    emit imageInfoChanged(imagePath_, img_.size());
    notifyViewChanged();
    return true;
}
