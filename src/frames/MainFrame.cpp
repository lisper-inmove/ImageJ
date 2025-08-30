#include <QHBoxLayout>
#include <QSplitter>
#include <QHBoxLayout>
#include <QList>
#include <QShortcut>

#include "frames/MainFrame.h"
#include "image_canvas/ImageCanvas.h"
#include "frames/RightSidebar.h"
#include "logger/logger.h"

MainFrame::MainFrame(QMainWindow* parent): QMainWindow(parent) {

    auto* esc = new QShortcut(QKeySequence::Cancel, this);
    esc->setContext(Qt::ApplicationShortcut);
    connect(esc, &QShortcut::activated, this, &MainFrame::onEsc);

    build();
    connectSignals();
}

void MainFrame::build() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    buildImageCanvas(splitter);
    buildRightSidebar(splitter);
    buildMenu();

    setCentralWidget(splitter);

    QList<int> sizes;
    sizes << 800 << 300;
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
}

void MainFrame::buildRightSidebar(QSplitter* splitter) {
    rightSidebar_ = new RightSidebar(splitter);
    rightSidebar_->setObjectName("RightSidebar");
}

void MainFrame::buildImageCanvas(QSplitter* splitter) {
    canvas_ = new ImageCanvas(splitter);
    canvas_->setObjectName("Canvas");
}

void MainFrame::buildMenu() {
    menu_ = new Menu(this);
    menu_->build();
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
    connect(menu_, &Menu::openRequested, canvas_, &ImageCanvas::openImage);
    connect(menu_, &Menu::histogramRequested, canvas_, &ImageCanvas::displayHistogram);
}


void MainFrame::onEsc() {
    LOG_INFO("on Esc");
    canvas_->cancel();
}
