#include "frames/MainFrame.h"

#include <QMenuBar>
#include <QList>
#include <QSplitter>

MainFrame::MainFrame(QWidget* parent)
    : QMainWindow(parent)
{

}

void MainFrame::build(QString path) {
    buildUi(path);
    connectSignals();
}

void MainFrame::buildUi(QString path) {

    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // 用样式表修改分隔线
    splitter->setStyleSheet(R"(
    QSplitter::handle {
        background: #a0a0a0;      /* 灰色 */
    }
    QSplitter::handle:horizontal {
        width: 1px;              /* 分隔线宽度 */
    }
    QSplitter::handle:vertical {
        height: 1px;             /* 如果是上下分割，控制粗细 */
    }
    )");

    QList<int> sizes;
    sizes.append(rightWidth_);
    sizes.append(leftWidth_);

    buildMenubar();
    buildBody(splitter, path);
    buildRightside(splitter);

    setCentralWidget(splitter);
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
}

void MainFrame::buildMenubar() {
    QMenu* fMenu = menuBar()->addMenu("File");
    fMenu->setObjectName("File");
    actOpen_ = new QAction("Open", fMenu);
    actOpen_->setObjectName("act_open_");

    QMenu* oMenu = menuBar()->addMenu("Operation");
    oMenu->setObjectName("Operation");
    actHist_ = new QAction("Histogram", oMenu);
    actHist_->setObjectName("act_hist_");

    fMenu->addAction(actOpen_);
    oMenu->addAction(actHist_);

    qDebug() << "File menu actions count: " << fMenu->actions().count();
    qDebug() << "Operation menu actions count: " << oMenu->actions().count();
}

void MainFrame::buildBody(QSplitter* splitter, QString path) {
    body_ = new JBody(splitter);
    body_->setWidth(leftWidth_);
    body_->setHeight(height_);
    body_->build(path);
}

void MainFrame::buildRightside(QSplitter* splitter) {
    rightside_ = new JRightside(splitter);
    rightside_->setWidth(rightWidth_);
    rightside_->setHeight(height_);
    rightside_->build();
}

void MainFrame::connectSignals() {
    // 选择图片
    connect(actOpen_, &QAction::triggered, body_, &JBody::open);
}
