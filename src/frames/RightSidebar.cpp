#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QFormLayout>
#include <QLabel>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include "frames/RightSidebar.h"

RightSidebar::RightSidebar(QWidget* parent): QWidget(parent) {
    build();
    setFixedWidth(width_);
    connectSignals();
}

void RightSidebar::build() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    tabs_ = new QTabWidget(this);
    root->addWidget(tabs_);

    auto* infoPage = new QWidget();
    auto* infoLay = new QFormLayout(infoPage);

    lbPath_ = new QLabel(tr("-"));
    lbPath_->setWordWrap(true);
	lbSize_ = new QLabel(tr("-"));
    lbMouse_ = new QLabel(tr("-"));
    lbPixel_ = new QLabel(tr("-"));

    infoLay->addRow(tr("Path: "), lbPath_);
    infoLay->addRow(tr("Size: "), lbSize_);
    infoLay->addRow(tr("Mouse: "), lbMouse_);
    infoLay->addRow(tr("Pixel Info: \n"), lbPixel_);
    tabs_->addTab(infoPage, tr("ImageInfo"));
}

void RightSidebar::imageInfoChanged(const QString& path, const QSize& size) {
    lbPath_->setText(path);
	lbSize_->setText(QString("%1 x %2").arg(size.width()).arg(size.height()));
}

void RightSidebar::updateImagePixelInfo(const QString& path) {
    cv::Mat img = cv::imread(path.toStdString());

    // 灰度图 + Mask
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::Mat mask = (gray > 128);

    // 均值
    cv::Scalar mean_bgr = cv::mean(img, mask);
    double mean_gray = 0.114 * mean_bgr[0] + 0.587 * mean_bgr[1] + 0.299 * mean_bgr[2];

    // 均值 + 标准差
    cv::Scalar mean, stddev;
    cv::meanStdDev(img, mean, stddev, mask);

    lbPixel_->setText(QString("RGBMean: BGR(%1,%2,%3)\nGray Mean: %4\nMeanStdDev: %5\n").arg(mean_bgr[0]).arg(mean_bgr[1]).arg(mean_bgr[2]).arg(mean_gray).arg(stddev[0]));
}

void RightSidebar::mouseMoved(const QPoint& pos, const QPointF& imgPos) {
    /**
        更新鼠标位置
    */
    lbMouse_->setText(QString("Window: (%1, %2)\nImage: (%3, %4)").arg(pos.x()).arg(pos.y()).arg(imgPos.x()).arg(imgPos.y()));
}

void RightSidebar::connectSignals() {}
