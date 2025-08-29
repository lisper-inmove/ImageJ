// #include "ImageCanvas.h"

// #include <QPainter>
// #include <QMouseEvent>
// #include <QFileDialog>
// #include <QMimeData>
// #include <QDragEnterEvent>
// #include <QDropEvent>
// #include <QWheelEvent>
// #include <QResizeEvent>
// #include <QUrl>

// #include <algorithm>
// #include <cmath>

// // 约束 offset_ 使图像不被拖出可视区域
// static QPointF clampOffsetForImage(const QSize& widgetSize,
//                                    const QSize& imgSize,
//                                    double scale,
//                                    const QPointF& desiredOffset)
// {
//     const QSizeF scaled(imgSize.width() * scale, imgSize.height() * scale);
//     const double dx = std::max(0.0, (scaled.width()  - widgetSize.width())  / 2.0);
//     const double dy = std::max(0.0, (scaled.height() - widgetSize.height()) / 2.0);
//     const double ox = std::clamp(desiredOffset.x(), -dx, dx);
//     const double oy = std::clamp(desiredOffset.y(), -dy, dy);
//     return QPointF(ox, oy);
// }

// ImageCanvas::ImageCanvas(QWidget* parent)
//     : QWidget(parent) {
//     setWindowTitle(QStringLiteral("Image Selector"));
//     setAttribute(Qt::WA_OpaquePaintEvent);
//     setMouseTracking(true);
//     setCursor(Qt::CrossCursor);
//     setMinimumSize(640, 480);
//     setAcceptDrops(true);
// }

// void ImageCanvas::paintEvent(QPaintEvent*) {
//     QPainter p(this);
//     p.fillRect(rect(), Qt::black);

//     if (!img_.isNull()) {
//         const double bs = baseScale(size());
//         const double s  = bs * zoom_;
//         const QSizeF scaled(img_.width() * s, img_.height() * s);

//         const QPointF centerBaseOrigin(
//             (width()  - scaled.width())  / 2.0,
//             (height() - scaled.height()) / 2.0
//             );
//         const QPointF origin = centerBaseOrigin + offset_;

//         p.setRenderHint(QPainter::SmoothPixmapTransform, true);
//         p.save();
//         p.translate(origin);
//         p.scale(s, s);
//         p.drawImage(QPointF(0, 0), img_);
//         p.restore();
//     }

//     drawCrosshair(p);

//     if (img_.isNull()) {
//         p.setPen(QColor(220, 220, 220));
//         QFont f = p.font(); f.setPointSizeF(f.pointSizeF() + 2); p.setFont(f);
//         p.drawText(rect().adjusted(0, 30, 0, 0),
//                    Qt::AlignHCenter | Qt::AlignTop,
//                    QStringLiteral("单击选择图片…（也可拖拽图片到此处）"));
//     }
// }

// void ImageCanvas::mousePressEvent(QMouseEvent* ev) {
//     if (ev->button() == Qt::LeftButton) {
//         if (img_.isNull()) {
//             openImage(); // 没图时才允许打开
//         } else {
//             dragging_ = true;
//             lastPos_  = ev->pos();
//             setCursor(Qt::ClosedHandCursor);
//         }
//     }
//     QWidget::mousePressEvent(ev);
// }

// void ImageCanvas::mouseMoveEvent(QMouseEvent* ev) {
//     if (dragging_ && !img_.isNull()) {
//         const QPoint delta = ev->pos() - lastPos_;
//         lastPos_ = ev->pos();

//         const double s = baseScale(size()) * zoom_;
//         const QPointF wanted = offset_ + delta;
//         offset_ = clampOffsetForImage(size(), img_.size(), s, wanted);

//         update();
//         notifyViewChanged();
//         notifyMousePos(ev->pos());
//         ev->accept();
//         return;
//     }
//     // 非拖拽也实时通报鼠标位置
//     notifyMousePos(ev->pos());
//     QWidget::mouseMoveEvent(ev);
// }

// void ImageCanvas::mouseReleaseEvent(QMouseEvent* ev) {
//     if (ev->button() == Qt::LeftButton && dragging_) {
//         dragging_ = false;
//         setCursor(Qt::CrossCursor);
//         ev->accept();
//         return;
//     }
//     QWidget::mouseReleaseEvent(ev);
// }

// void ImageCanvas::dragEnterEvent(QDragEnterEvent* ev) {
//     if (ev->mimeData()->hasUrls()) {
//         ev->acceptProposedAction();
//     } else {
//         ev->ignore();
//     }
// }

// void ImageCanvas::dropEvent(QDropEvent* ev) {
//     if (!ev->mimeData()->hasUrls()) {
//         ev->ignore();
//         return;
//     }
//     const auto urls = ev->mimeData()->urls();
//     for (const QUrl& url : urls) {
//         const QString path = url.toLocalFile();
//         if (loadImage(path)) {
//             ev->acceptProposedAction();
//             return;
//         }
//     }
//     ev->ignore();
// }

// void ImageCanvas::wheelEvent(QWheelEvent* ev) {
//     if (img_.isNull()) { ev->ignore(); return; }

//     const QPointF m = ev->position();
//     const double bs = baseScale(size());
//     const double s_old = bs * zoom_;

//     const QSizeF scaledOld(img_.width()*s_old, img_.height()*s_old);
//     const QPointF cbo_old(
//         (width()  - scaledOld.width())  / 2.0,
//         (height() - scaledOld.height()) / 2.0
//         );
//     const QPointF origin_old = cbo_old + offset_;

//     const double steps = ev->angleDelta().y() / 120.0;
//     if (steps == 0.0) { ev->ignore(); return; }

//     const double factor = std::pow(1.2, steps);
//     double newZoom = zoom_ * factor;
//     newZoom = std::clamp(newZoom, 0.05, 50.0);

//     const double s_new = bs * newZoom;

//     // 保持鼠标处图像点位置不变
//     const QPointF u = (m - origin_old) / s_old;
//     const QSizeF  scaledNew(img_.width()*s_new, img_.height()*s_new);
//     const QPointF cbo_new(
//         (width()  - scaledNew.width())  / 2.0,
//         (height() - scaledNew.height()) / 2.0
//         );
//     const QPointF origin_new = m - u * s_new;
//     QPointF wantedOffset = origin_new - cbo_new;

//     wantedOffset = clampOffsetForImage(size(), img_.size(), s_new, wantedOffset);

//     offset_ = wantedOffset;
//     zoom_   = newZoom;

//     update();
//     notifyViewChanged();
//     notifyMousePos(ev->position().toPoint());
//     ev->accept();
// }

// void ImageCanvas::resizeEvent(QResizeEvent* ev) {
//     if (img_.isNull() || !ev->oldSize().isValid()) {
//         QWidget::resizeEvent(ev);
//         notifyViewChanged();
//         return;
//     }

//     const double bs_old = baseScale(ev->oldSize());
//     const double bs_new = baseScale(ev->size());
//     const double s_old  = bs_old * zoom_;
//     const double s_new  = bs_new * zoom_;

//     const QSizeF scaledOld(img_.width()*s_old, img_.height()*s_old);
//     const QSizeF scaledNew(img_.width()*s_new, img_.height()*s_new);

//     const QPointF cbo_old(
//         (ev->oldSize().width()  - scaledOld.width())  / 2.0,
//         (ev->oldSize().height() - scaledOld.height()) / 2.0
//         );
//     const QPointF cbo_new(
//         (ev->size().width()  - scaledNew.width())  / 2.0,
//         (ev->size().height() - scaledNew.height()) / 2.0
//         );

//     QPointF wantedOffset = offset_ + (cbo_old - cbo_new);
//     wantedOffset = clampOffsetForImage(ev->size(), img_.size(), s_new, wantedOffset);
//     offset_ = wantedOffset;

//     QWidget::resizeEvent(ev);
//     notifyViewChanged();
// }

// void ImageCanvas::drawCrosshair(QPainter& p) {
//     const int L = int(0.3 * std::min(width(), height()));

//     QPoint center = QPoint(width()/2, height()/2);
//     const bool hasImage = !img_.isNull();

//     if (hasImage) {
//         const double bs = baseScale(size());
//         const double s  = bs * zoom_;
//         const QSizeF scaled(img_.width() * s, img_.height() * s);

//         const QPointF centerBaseOrigin(
//             (width()  - scaled.width())  / 2.0,
//             (height() - scaled.height()) / 2.0
//             );
//         const QPointF origin = centerBaseOrigin + offset_;

//         const QPointF imgCenter = origin + QPointF(scaled.width()/2.0, scaled.height()/2.0);
//         center = imgCenter.toPoint();
//     }

//     QPen pen(hasImage ? Qt::red : Qt::white);
//     pen.setWidth(2);
//     pen.setCosmetic(true);
//     p.setRenderHint(QPainter::Antialiasing, true);
//     p.setPen(pen);

//     p.drawLine(QPoint(center.x() - L/2, center.y()),
//                QPoint(center.x() + L/2, center.y()));
//     p.drawLine(QPoint(center.x(), center.y() - L/2),
//                QPoint(center.x(), center.y() + L/2));

//     p.setBrush(hasImage ? Qt::red : Qt::white);
//     p.drawEllipse(center, 3, 3);
// }

// void ImageCanvas::chooseImage() {
//     const QString filter = QStringLiteral(
//         "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;All files (*)");
//     const QString path = QFileDialog::getOpenFileName(
//         this, QStringLiteral("选择图片"), QString(), filter);
//     if (!path.isEmpty()) {
//         loadImage(path);
//     }
// }

// bool ImageCanvas::loadImage(const QString& path) {
//     QImage tmp;
//     if (!tmp.load(path)) {
//         return false;
//     }
//     img_ = std::move(tmp);
//     imagePath_ = path;
//     zoom_   = 1.0;
//     offset_ = QPointF(0, 0);
//     update();

//     emit imageInfoChanged(imagePath_, img_.size());
//     notifyViewChanged();
//     return true;
// }

// double ImageCanvas::baseScale(const QSize& widgetSize) const {
//     if (img_.isNull() || img_.width() == 0 || img_.height() == 0) return 1.0;
//     const double sx = double(widgetSize.width())  / img_.width();
//     const double sy = double(widgetSize.height()) / img_.height();
//     return std::min(sx, sy);
// }

// double ImageCanvas::effectiveScale() const {
//     return baseScale(size()) * zoom_;
// }

// void ImageCanvas::openImage() {
//     chooseImage();
// }

// void ImageCanvas::resetView() {
//     if (img_.isNull()) return;
//     zoom_   = 1.0;
//     offset_ = QPointF(0, 0);
//     update();
//     notifyViewChanged();
// }

// void ImageCanvas::fitToWindow() {
//     resetView();
// }

// void ImageCanvas::zoomActualPixels() {
//     if (img_.isNull()) return;
//     const double bs = baseScale(size());
//     if (bs > 0.0) {
//         zoom_ = 1.0 / bs;
//         offset_ = QPointF(0, 0);
//         const double s = baseScale(size()) * zoom_;
//         offset_ = clampOffsetForImage(size(), img_.size(), s, offset_);
//         update();
//         notifyViewChanged();
//     }
// }

// void ImageCanvas::notifyViewChanged() {
//     emit viewChanged(zoom_, offset_, effectiveScale());
// }

// void ImageCanvas::notifyMousePos(const QPoint& widgetPos) {
//     if (img_.isNull()) {
//         emit mousePositionChanged(widgetPos, QPointF(-1, -1), false);
//         return;
//     }
//     const double bs = baseScale(size());
//     const double s  = bs * zoom_;

//     const QSizeF scaled(img_.width() * s, img_.height() * s);
//     const QPointF centerBaseOrigin(
//         (width()  - scaled.width())  / 2.0,
//         (height() - scaled.height()) / 2.0
//         );
//     const QPointF origin = centerBaseOrigin + offset_;

//     const QPointF u = (QPointF(widgetPos) - origin) / s; // 图像坐标
//     const bool inside = (u.x() >= 0 && u.y() >= 0 &&
//                          u.x() < img_.width() && u.y() < img_.height());
//     emit mousePositionChanged(widgetPos, u, inside);
// }
