#include "widgets/HistogramWidget.h"
#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>
#include <QtMath>
#include <algorithm>
#include <opencv2/opencv.hpp>

HistogramWidget::HistogramWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(600, 450);
    setMouseTracking(true);  // 允许无按键移动也触发 mouseMoveEvent
}

void HistogramWidget::setImage(const cv::Mat& img) {
    std::fill(std::begin(hist_), std::end(hist_), 0);
    maxv_ = 1;
    total_ = 0;
    hoverIndex_ = -1;

    if (!img.empty()) {
        if (img.channels() == 1) { // 灰度图
            for (int i = 0; i < img.rows; i++) {
                const uchar* line = img.ptr<uchar>(i);
                for (int x = 0; x < img.cols; ++x) {
                    ++hist_[line[x]];
                }
            }
        } else { // RGB图
            // 彩色图，先转灰度
            cv::Mat gray;
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            for (int y = 0; y < gray.rows; ++y) {
                const uchar* line = gray.ptr<uchar>(y);
                for (int x = 0; x < gray.cols; ++x) {
                    ++hist_[line[x]];
                }
            }
            std::vector<cv::Mat> mv;
            cv::split(img, mv);
            int channels[] = {0};
            int bins = 256;
            int histSize[] = {bins};
            float rgb_ranges[] = {0, 256};
            const float* ranges[] = {rgb_ranges};
            cv::calcHist(&mv[0], 1, channels, cv::Mat(), hist_g_, 1, histSize, ranges, true, false);
            cv::calcHist(&mv[1], 1, channels, cv::Mat(), hist_b_, 1, histSize, ranges, true, false);
            cv::calcHist(&mv[2], 1, channels, cv::Mat(), hist_r_, 1, histSize, ranges, true, false);
        }

        for (int i = 0; i < 256; ++i) {
            maxv_ = std::max(maxv_, hist_[i]);
            total_ += hist_[i];
        }
        if (maxv_ == 0) maxv_ = 1;
    }

    update();
}

void HistogramWidget::paintEvent(QPaintEvent* /*ev*/) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    const int innerW = width()  - padL_ - padR_;
    const int innerH = height() - padT_ - padB_;

    // 边框
    p.setPen(QColor(200,200,200));
    p.drawRect(QRect(padL_, padT_, innerW, innerH));
    p.setPen(Qt::black);

    if (maxv_ == 0 || innerW <= 0 || innerH <= 0) return;

    // 直方图
    for (int i = 0; i < 256; ++i) {
        const double v = double(hist_g_.at<float>(i)) / double(maxv_);
        const int hgt = int(std::round(v * innerH));
        const int x   = padL_ + (i * innerW) / 256;
        const int y   = padT_ + innerH - hgt;
        p.drawLine(x, padT_ + innerH, x, y);
    }

    // 悬停高亮线
    if (hoverIndex_ >= 0 && hoverIndex_ <= 255) {
        const int x = padL_ + (hoverIndex_ * innerW) / 256;
        QPen pen(QColor(0, 120, 215, 160)); // 半透明高亮
        pen.setWidth(2);
        p.setPen(pen);
        p.drawLine(x, padT_, x, padT_ + innerH);
    }

    // x 轴刻度
    p.setPen(Qt::darkGray);
    p.drawText(QPointF(padL_, height() - 4), "0");
    p.drawText(QPointF(padL_ + innerW/2 - 8, height() - 4), "128");
    p.drawText(QPointF(padL_ + innerW - 22, height() - 4), "255");
}

int HistogramWidget::xToBin(int x) const {
    const int innerW = width()  - padL_ - padR_;
    const int innerH = height() - padT_ - padB_;
    if (innerW <= 0 || innerH <= 0) return -1;

    if (x < padL_ || x >= width() - padR_) return -1;
    int bin = int(std::floor( (x - padL_) * 256.0 / innerW ));
    if (bin < 0)   bin = 0;
    if (bin > 255) bin = 255;
    return bin;
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* ev) {
    const int bin = xToBin(ev->position().toPoint().x());
    if (bin != -1) {
        hoverIndex_ = bin;
        const quint64 count = hist_[bin];
        const double pct = (total_ > 0) ? (100.0 * double(count) / double(total_)) : 0.0;
        const QString tip = tr("灰度: %1\n计数: %2\n占比: %3%")
                                .arg(bin)
                                .arg(QString::number(count), QString::number(pct, 'f', 3));
        QToolTip::showText(ev->globalPosition().toPoint(), tip, this);
    } else {
        hoverIndex_ = -1;
        QToolTip::hideText();
    }
    update(); // 触发重绘以更新高亮线
}

void HistogramWidget::leaveEvent(QEvent* /*ev*/) {
    hoverIndex_ = -1;
    QToolTip::hideText();
    update();
}
