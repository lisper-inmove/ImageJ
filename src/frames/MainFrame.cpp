#include "frames/MainFrame.h"
#include "image_canvas/ImageCanvas.h"
#include "frames/RightSidebar.h"
#include "widgets/histogram//HistogramDialog.h"
#include "widgets/preview/ImagePreviewDialog.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QDialog>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QtMath>
#include <QLabel>
#include <QVBoxLayout>

MainFrame::MainFrame(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    connectSignals();
}

int MainFrame::add(int a, int b) {
    return a + b;
}

void MainFrame::buildUi() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    canvas_  = new ImageCanvas(splitter);
    sidebar_ = new RightSidebar(splitter);

    sidebar_->setObjectName("RightSidebar");
    canvas_->setObjectName("ImageCanvas");

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->addWidget(splitter);

    QList<int> sizes; sizes << 800 << 300;
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
}

void MainFrame::connectSignals() {
    connect(canvas_, &ImageCanvas::imageInfoChanged, this,
            [this](const QString& path, const QSize& size){ sidebar_->setInfo(path, size); });
    connect(canvas_, &ImageCanvas::viewChanged, this,
            [this](double /*zoom*/, const QPointF& offset, double scale){ sidebar_->setView(offset, scale); });

    connect(canvas_, &ImageCanvas::mousePositionChanged, this, &MainFrame::onMouseChange);
    connect(sidebar_, &RightSidebar::openImageRequested,         canvas_, &ImageCanvas::openImage);
    connect(sidebar_, &RightSidebar::fitToWindowRequested,       canvas_, &ImageCanvas::fitToWindow);
    connect(sidebar_, &RightSidebar::resetViewRequested,         canvas_, &ImageCanvas::resetView);
    connect(sidebar_, &RightSidebar::zoomActualPixelsRequested,  canvas_, &ImageCanvas::zoomActualPixels);
    connect(sidebar_, &RightSidebar::histogramRequested,         this,    &MainFrame::showHistogram);
    connect(canvas_, &ImageCanvas::selectionCreated, this, [this](const QImage& img){
        // 方式A：弹出预览对话框
        ImagePreviewDialog dlg(img, this);
        dlg.exec();

        // 方式B（可选）：如果你想把历史放到右侧标签，可以把 img 传给 sidebar_ 去显示/记录
    });
}

void MainFrame::onMouseChange(const QPoint& wPos, const QPointF& iPos, bool inside) {
    sidebar_->setMouse(wPos, iPos, inside);
}

void MainFrame::showHistogram() {
    const QImage img = canvas_->currentImage();
    HistogramDialog::showForImage(img, this);
}
