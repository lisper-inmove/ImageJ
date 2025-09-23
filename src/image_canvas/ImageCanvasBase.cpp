#include <QFileDialog>
#include <QPainter>
#include <QResizeEvent>
#include "logger/logger.h"
#include <opencv2/opencv.hpp>

#include "image_canvas/ImageCanvas.h"

ImageCanvas::ImageCanvas(QWidget* parent)
    : QWidget(parent) {
    setWindowTitle(QStringLiteral("Image Selector"));
    // 设置Qt的容器属性。这个控件在paintEvent里会完全绘制自己的区域，不需要Qt在绘制之前帮你清理背景
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setMinimumSize(size_.width(), size_.height());
    // 开启drag & drop
    setAcceptDrops(true);

    //QPalette pal = palette();
    //pal.setColor(QPalette::Window, Qt::white);
    //setAutoFillBackground(true);
    //setPalette(pal);
}

void ImageCanvas::openImage() {
    chooseImage();
}

void ImageCanvas::chooseImage() {
    const QString filter = QStringLiteral("Images (*.png *.xpm *.jpg *.jpeg *.bmp);;All files (*.*)");
    const QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), filter);
    if (!path.isEmpty()) {
        loadImage(path);
        emit imageInfoChanged(path, img_.size());
    }
}

bool ImageCanvas::loadImage(const QString path) {
    QImage tmp;
    if (!tmp.load(path)) {
        return false;
    }
    cv_img_ = cv::imread(path.toStdString());
    img_ = std::move(tmp);
    update();
    return true;
}

void ImageCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
    if (!img_.isNull()) {
        const double scale = effectiveScale();

        const QPointF iOrigin = imageOrigin();
        const QPointF origin = iOrigin + offset_;
        // 启用平滑滤波进行缩放（双线性/更平滑的采样），避免缩放锯齿；代价是耗时更高。常用于图像缩放。
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);

        p.save();
        // 将坐标系原点平移到图片绘制的起点
		p.translate(origin);
        p.scale(scale, scale);

        p.drawImage(0, 0, img_);
    }
	drawCrosshair(p);
}

void ImageCanvas::resizeEvent(QResizeEvent* event) {
    QSize oldSize = event->oldSize();
	QSize newSize = event->size();
    LOG_DEBUG("ResizeEvent oldSize: {}, {}", oldSize.height(), oldSize.width());
    LOG_DEBUG("ResizeEvent newSize: {}, {}", newSize.height(), newSize.width());
}

void ImageCanvas::wheelEvent(QWheelEvent* event) {
    if (img_.isNull()) {
        event->ignore();
        return;
    }
    const QPointF m = event->position();

	double oldScale = effectiveScale();
    const int delta = event->angleDelta().y();
    const QPointF cboOld = imageOrigin();
    const QPointF originOld = cboOld + offset_;

    if (delta > 0) {
        scale_ *= 1.1;
    } else if (delta < 0) {
        scale_ /= 1.1;
    }
    scale_ = std::clamp(scale_, 0.1, 10.0);
	double newScale = effectiveScale();

    const QPointF u = (m - originOld) / oldScale;
    const QPointF cboNew = imageOrigin();
    const QPointF originNew = m - u * newScale;
    QPointF wanted = originNew - cboNew;
	offset_ = clampOffsetForImage(wanted);
    // const QPointF iOrigin = imageOrigin();

    update();
    event->accept();
}

void ImageCanvas::drawCrosshair(QPainter& p) {
	const bool hasImg = !img_.isNull();
    int L = int(0.3 * std::min(width(), height()));
	QPoint center = rect().center();
    if (hasImg) {
        L = int(0.3 * std::min(img_.width(), img_.height()));
        center = QPoint(img_.width() / 2, img_.height() / 2);
    }
    QPen pen(hasImg ? Qt::red : Qt::green);
    pen.setWidth(1);
    pen.setCosmetic(true);
	p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(pen);

    p.drawLine(QPoint(center.x() - L / 2, center.y()), QPoint(center.x() + L / 2, center.y()));
    p.drawLine(QPoint(center.x(), center.y() - L / 2), QPoint(center.x(), center.y() + L / 2));
    p.setBrush(hasImg ? Qt::red : Qt::green);
	p.drawEllipse(center, 3, 3);
}

double ImageCanvas::baseScale(const QSize& widgetSize) const {
    /**
        将图片等比缩放以适应窗口大小，返回窗口与图片的缩放比例
    */
    if (img_.isNull() || img_.width() == 0 || img_.height() == 0) return 1.0;
	const double sx = double(widgetSize.width()) / img_.width();
	const double sy = double(widgetSize.height()) / img_.height();
    return std::min(sx, sy);
}

double ImageCanvas::effectiveScale() const {
    /**
        实际缩放比例 = 用户缩放比例 * 基础缩放比例
    */
    return scale_ * baseScale(size());
}

QPointF ImageCanvas::imageOrigin() const {
    if (img_.isNull() || img_.width() == 0 || img_.height() == 0) return rect().center();
    const double scale = effectiveScale();
    const double x = img_.width() * scale / 2.0;
    const double y = img_.height() * scale / 2.0;
    QSize widgetSize = size();
	return QPointF(widgetSize.width() / 2.0 - x, widgetSize.height() / 2.0 - y);
}

// 将 offset 限制在 [-dx,+dx] × [-dy,+dy]，避免图片被拖出可视区域
QPointF ImageCanvas::clampOffsetForImage(const QPointF& desiredOffset) {
    /**
        图片中心与视窗的中心的偏移。用图片的中心减去视窗的中心
    */
	double scale = effectiveScale();
	QSize widgetSize = size();
	QSize imgSize = img_.size();

    const double w = imgSize.width() * scale;
    const double h = imgSize.height() * scale;
    const double dx = std::max(0.0, (w - widgetSize.width()) / 2.0);
    const double dy = std::max(0.0, (h - widgetSize.height()) / 2.0);

    const double ox = std::clamp(desiredOffset.x(), -dx, dx);
    const double oy = std::clamp(desiredOffset.y(), -dy, dy);
    return QPointF(ox, oy);
}

void ImageCanvas::cancel() {
    hSelecting_ = false;
    selecting_ = false;
}
