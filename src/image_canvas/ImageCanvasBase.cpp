#include "image_canvas/ImageCanvas.h"
#include <QString>
#include <QFileDialog>

ImageCanvas::ImageCanvas(QWidget* parent)
    : QWidget(parent) {
    setWindowTitle(QStringLiteral("Image Selector"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setMinimumSize(size_.width(), size_.height());
    setAcceptDrops(true);
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

void ImageCanvas::openImage() {
    chooseImage();
}

void ImageCanvas::openImageFromPath(const QString &filepath) {
    loadImage(filepath);
}
