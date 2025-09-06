#include <QHBoxLayout>
#include <QSplitter>
#include <QHBoxLayout>
#include <QList>

#include "frames/MainFrame.h"
#include "image_canvas/ImageCanvas.h"
#include "frames/RightSidebar.h"
#include "logger/logger.h"

MainFrame::MainFrame(QWidget* parent): QWidget(parent) {
    build();
    connectSignals();
}

void MainFrame::build() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    buildImageCanvas(splitter);
    buildRightSidebar(splitter);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(splitter);

    QList<int> sizes;
    sizes << 800 << 300;
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    canvas_->show();
    rightSidebar_->show();
}

void MainFrame::buildRightSidebar(QSplitter* splitter) {
    rightSidebar_ = new RightSidebar(splitter);
    rightSidebar_->setObjectName("RightSidebar");
}

void MainFrame::buildImageCanvas(QSplitter* splitter) {
    canvas_ = new ImageCanvas(splitter);
    canvas_->setObjectName("Canvas");
}

void MainFrame::connectSignals() {
    connect(canvas_, &ImageCanvas::imageInfoChanged, this, [this](const QString& path, const QSize& size) {
        LOG_DEBUG("ImageInfoChanged");
        rightSidebar_->imageInfoChanged(path, size);
    });
    connect(canvas_, &ImageCanvas::mouseMoved, this, [this](const QPoint& pos, const QPointF& imgPos) {
        LOG_DEBUG("Mouse moved");
        rightSidebar_->mouseMoved(pos, imgPos);
    });
}
