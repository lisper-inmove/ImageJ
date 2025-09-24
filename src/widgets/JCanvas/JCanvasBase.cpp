#include "widgets/JCanvas.h"
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

void JCanvas::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    if (!img_.isNull()) {
        double scale = effectiveScale();
        const QPointF origin = imageOrigin();
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.save();
        // 将画布起始点设置在 Frame 的某个位置(图片的原点)
        p.translate(origin);
        // 画布与图片做一样的缩放
        p.scale(scale, scale);
        // 在画布上展示图片(画布与图片其实是完全重合的)
        p.drawImage(0, 0, img_);
    }
    drawCrosshair(p);
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
        返回图片在Frame的原点位置。也就是左上角的起始点
    */
    if (img_.isNull() || img_.width() == 0 || img_.height() == 0) return rect().center();
    const double scale = effectiveScale();
    const double x = img_.width() * scale / 2.0;
    const double y = img_.height() * scale / 2.0;
    QSize s = size();
    return QPointF(s.width() / 2.0 - x, s.height() / 2.0 - y);
}
