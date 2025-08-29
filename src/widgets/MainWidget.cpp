#include "widgets/MainWidget.h"
#include "image_canvas/ImageCanvas.h"
#include "widgets/RightSidebar.h"   // 新增

#include <QSplitter>
#include <QHBoxLayout>
#include <QDialog>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QtMath>
#include <QLabel>
#include <QVBoxLayout>

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    connectSignals();
}

void MainWidget::buildUi() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    canvas_  = new ImageCanvas(splitter);
    sidebar_ = new RightSidebar(splitter);  // 用独立控件

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->addWidget(splitter);

    QList<int> sizes; sizes << 800 << 300;
    splitter->setSizes(sizes);
}

void MainWidget::connectSignals() {
    // ImageCanvas -> Sidebar 显示
    connect(canvas_, &ImageCanvas::imageInfoChanged, this,
            [this](const QString& path, const QSize& size){ sidebar_->setInfo(path, size); });

    connect(canvas_, &ImageCanvas::viewChanged, this,
            [this](double /*zoom*/, const QPointF& offset, double scale){ sidebar_->setView(offset, scale); });

    connect(canvas_, &ImageCanvas::mousePositionChanged, this,
            [this](const QPoint& wPos, const QPointF& iPos, bool inside){ sidebar_->setMouse(wPos, iPos, inside); });

    // Sidebar -> 业务（把按钮转换成对 canvas_ 的调用）
    connect(sidebar_, &RightSidebar::openImageRequested,         canvas_, &ImageCanvas::openImage);
    connect(sidebar_, &RightSidebar::fitToWindowRequested,       canvas_, &ImageCanvas::fitToWindow);
    connect(sidebar_, &RightSidebar::resetViewRequested,         canvas_, &ImageCanvas::resetView);
    connect(sidebar_, &RightSidebar::zoomActualPixelsRequested,  canvas_, &ImageCanvas::zoomActualPixels);
    connect(sidebar_, &RightSidebar::histogramRequested,         this,    &MainWidget::showHistogram);
}

// 这三个改成把数据传给 sidebar（若你更喜欢信号中转，也可保留原函数体不变）
void MainWidget::updateInfoTab(const QString& path, const QSize& size) {
    sidebar_->setInfo(path, size);
}
void MainWidget::updateViewTab(double /*zoom*/, const QPointF& offset, double scale) {
    sidebar_->setView(offset, scale);
}
void MainWidget::updateMouseLabel(const QPoint& widgetPos, const QPointF& imagePos, bool inside) {
    sidebar_->setMouse(widgetPos, imagePos, inside);
}

// showHistogram() 原样保留（内部仍然调用 canvas_->currentImage()）
void MainWidget::showHistogram() {
    QImage img = canvas_->currentImage();
    if (img.isNull()) return;

    quint64 hist[256] = {0};
    const int w = img.width();
    const int h = img.height();

    if (img.format() == QImage::Format_Grayscale8) {
        for (int y = 0; y < h; ++y) {
            const uchar* line = img.constScanLine(y);
            for (int x = 0; x < w; ++x) ++hist[line[x]];
        }
    } else {
        QImage conv = img.convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < conv.height(); ++y) {
            const QRgb* line = reinterpret_cast<const QRgb*>(conv.constScanLine(y));
            for (int x = 0; x < conv.width(); ++x) {
                const int gray = qGray(line[x]);
                ++hist[gray];
            }
        }
    }

    quint64 maxv = 1;
    for (int i = 0; i < 256; ++i) maxv = std::max(maxv, hist[i]);

    const int plotW = 256, plotH = 160;
    const int padT = 10, padB = 20, padL = 10, padR = 10;
    const int innerW = plotW - padL - padR;
    const int innerH = plotH - padT - padB;

    QImage plot(plotW, plotH, QImage::Format_ARGB32_Premultiplied);
    plot.fill(Qt::white);
    QPainter p(&plot);
    p.setRenderHint(QPainter::Antialiasing, false);

    p.setPen(QColor(200,200,200));
    p.drawRect(QRect(padL, padT, innerW, innerH));
    p.setPen(Qt::black);

    for (int i = 0; i < 256; ++i) {
        const double v = double(hist[i]) / double(maxv);
        const int hgt = int(std::round(v * innerH));
        const int x   = padL + i;
        const int y   = padT + innerH - hgt;
        if (hgt > 0) p.drawLine(x, padT + innerH, x, y);
    }

    p.setPen(Qt::darkGray);
    p.drawText(QPointF(padL, plotH - 4), "0");
    p.drawText(QPointF(padL + innerW/2 - 8, plotH - 4), "128");
    p.drawText(QPointF(padL + innerW - 22, plotH - 4), "255");

    p.setPen(Qt::black);
    p.drawText(QRect(0, 0, plotW, padT), Qt::AlignCenter, tr("灰度直方图"));
    p.end();

    QDialog dlg(this);
    dlg.setWindowTitle(tr("灰度直方图"));
    auto* lay = new QVBoxLayout(&dlg);

    auto* imgLabel = new QLabel;
    imgLabel->setPixmap(QPixmap::fromImage(plot));
    imgLabel->setAlignment(Qt::AlignCenter);
    lay->addWidget(imgLabel);

    quint64 pixels = static_cast<quint64>(w) * static_cast<quint64>(h);
    auto* stats = new QLabel(
        tr("像素: %1 × %2 = %3,  最大柱高: %4")
            .arg(w).arg(h).arg(QString::number(pixels))
            .arg(QString::number(maxv)));
    stats->setAlignment(Qt::AlignCenter);
    lay->addWidget(stats);

    dlg.resize(plotW + 40, plotH + 80);
    dlg.exec();
}
