#include "widgets/HistogramDialog.h"
#include "widgets/HistogramWidget.h"

#ifdef UNIT_TEST
#include <QTimer>
#endif

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QtMath>
#include <algorithm>

HistogramDialog::HistogramDialog(const QImage& img, QWidget* parent)
    : QDialog(parent)
{
    auto* histWidget = new HistogramWidget;
    histWidget->setImage(img);
    setWindowTitle(tr("灰度直方图"));
    auto* lay = new QVBoxLayout(this);
    lay->addWidget(histWidget, 1);

    imageLabel_ = new QLabel;
    imageLabel_->setAlignment(Qt::AlignCenter);
    statsLabel_ = new QLabel;
    statsLabel_->setAlignment(Qt::AlignCenter);

    lay->addWidget(imageLabel_);
    lay->addWidget(statsLabel_);
}

void HistogramDialog::showForImage(const QImage& img, QWidget* parent) {
    if (img.isNull()) return;
    HistogramDialog dlg(img, parent);
    dlg.setObjectName("HistograDialog");

#ifdef UNIT_TEST
    QTimer::singleShot(5000, &dlg, &HistogramDialog::accept);  // 5秒后自动关闭
#endif
    dlg.exec();
}

void HistogramDialog::computeGrayHistogram(const QImage& img, quint64 (&hist)[256], quint64& maxv) {
    std::fill(std::begin(hist), std::end(hist), 0);
    maxv = 1;

    if (img.isNull()) return;
    const int w = img.width();
    const int h = img.height();

    if (img.format() == QImage::Format_Grayscale8) {
        for (int y = 0; y < h; ++y) {
            const uchar* line = img.constScanLine(y);
            for (int x = 0; x < w; ++x) {
                ++hist[line[x]];
            }
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

    for (int i = 0; i < 256; ++i) maxv = std::max(maxv, hist[i]);
}

QImage HistogramDialog::renderHistogramImage(const quint64 (&hist)[256], quint64 maxv,
                                             int plotW, int plotH)
{
    const int padT = 10, padB = 20, padL = 10, padR = 10;
    const int innerW = plotW - padL - padR;
    const int innerH = plotH - padT - padB;

    QImage plot(plotW, plotH, QImage::Format_ARGB32_Premultiplied);
    plot.fill(Qt::white);

    QPainter p(&plot);
    p.setRenderHint(QPainter::Antialiasing, false);

    // 边框与轴
    p.setPen(QColor(200,200,200));
    p.drawRect(QRect(padL, padT, innerW, innerH));
    p.setPen(Qt::black);

    // 柱状条：每个灰度 1px 宽
    if (maxv == 0) maxv = 1;
    for (int i = 0; i < 256; ++i) {
        const double v = double(hist[i]) / double(maxv);
        const int hgt = int(std::round(v * innerH));
        const int x   = padL + i;
        const int y   = padT + innerH - hgt;
        if (hgt > 0) p.drawLine(x, padT + innerH, x, y);
    }

    // x 轴刻度（0, 128, 255）
    p.setPen(Qt::darkGray);
    p.drawText(QPointF(padL, plotH - 4), "0");
    p.drawText(QPointF(padL + innerW/2 - 8, plotH - 4), "128");
    p.drawText(QPointF(padL + innerW - 22, plotH - 4), "255");

    // 标题
    p.setPen(Qt::black);
    p.drawText(QRect(0, 0, plotW, padT), Qt::AlignCenter, QObject::tr("灰度直方图"));

    p.end();
    return plot;
}
