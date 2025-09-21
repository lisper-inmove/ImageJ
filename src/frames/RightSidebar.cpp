#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QFormLayout>
#include <QLabel>
#include <opencv2/opencv.hpp>

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
    cv::Scalar m_bgr = cv::mean(img);
    // lbPixel_->setText(QString("Mean: %1\n").arg(m_bgr.val()));
}

void RightSidebar::mouseMoved(const QPoint& pos, const QPointF& imgPos) {
    /**
        更新鼠标位置
    */
    lbMouse_->setText(QString("Window: (%1, %2)\nImage: (%3, %4)").arg(pos.x()).arg(pos.y()).arg(imgPos.x()).arg(imgPos.y()));
}

void RightSidebar::connectSignals() {}
