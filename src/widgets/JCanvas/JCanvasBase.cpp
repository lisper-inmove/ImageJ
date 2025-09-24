#include "widgets/JCanvas.h"
#include "utils/logger.h"
#include <QPainter>
#include <QResizeEvent>
#include <QFileDialog>

JCanvas::JCanvas(QWidget* parent): QWidget(parent) {
    setWindowTitle(QStringLiteral("JCanvas"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setAcceptDrops(true);
}

void JCanvas::setSize(QSize& size) {
    size_.setWidth(size.width());
    size_.setHeight(size.height());
    setMinimumSize(size_);
}

void JCanvas::drawCrosshair(QPainter& p) {
    const bool hasImage = !cvImg_.empty();
    int L = int(0.3 * std::min(width(), height()));
    QPoint center = rect().center();
    if (hasImage) {
        L = int(0.3 * std::min(img_.width(), img_.height()));
        center = QPoint(img_.width() / 2, img_.height() / 2);
    }
    QPen pen(hasImage ? Qt::red : Qt::green);
    pen.setWidth(1);
    pen.setCosmetic(true);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(pen);

    p.drawLine(QPoint(center.x() - L / 2, center.y()), QPoint(center.x() + L / 2, center.y()));
    p.drawLine(QPoint(center.x(), center.y() - L / 2), QPoint(center.x(), center.y() + L / 2));
    p.setBrush(hasImage ? Qt::red : Qt::green);
    p.drawEllipse(center, 3, 3);
}

void JCanvas::open() {
#ifdef TEST_ENVIRONMENT  // 如果是在测试环境中
    // 使用默认文件路径进行测试
    const QString path = "C:/Users/Administrator/Desktop/Desktop.jpg";  // 默认路径
#else
    const QString filter = QStringLiteral("Images (*.png *.xpm *.jpg *.jpeg);; All files (*.*)");
    const QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), filter);
#endif
    if (!path.isEmpty()) {
        reset();
        QImage tmp;
        if (tmp.load(path)) {
            cvImg_ = cv::imread(path.toStdString());
            img_ = std::move(tmp);
            update();
        } else {

        }
    }
}

double JCanvas::baseScale() {
    if (img_.isNull() || img_.width() == 0 || img_.height() == 0) {
        return 1.0;
    }
    const double sx = double(width()) / img_.width();
    const double sy = double(height()) / img_.height();
    return std::min(sx, sy);
}

double JCanvas::effectiveScale() {
    return baseScale() * scale_;
}

QPointF JCanvas::imageOrigin() {
    /**
        返回Frame与图片的偏移
        同时也是图片原点相对于Frame原点的位置偏移
    */
    if (img_.isNull() || img_.width() == 0 || img_.height() == 0) return rect().center();
    const double scale = effectiveScale();
    const double x = img_.width() * scale / 2.0;
    const double y = img_.height() * scale / 2.0;
    QSize s = size();
    LOG_INFO("ImageOrigin: {}, {}", x, y);
    return QPointF(s.width() / 2.0 - x, s.height() / 2.0 - y);
}

void JCanvas::reset() {
    scale_ = 1.0;
    offset_ = QPointF(0.0, 0.0);
}
