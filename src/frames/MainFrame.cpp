#include "frames/MainFrame.h"

#include <QMenuBar>
#include <QList>
#include <QSplitter>

MainFrame::MainFrame(QWidget* parent)
    : QMainWindow(parent)
{
    buildUi();
    connectSignals();
}

void MainFrame::buildUi() {

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
    sizes.append(right_width_);
    sizes.append(left_width_);

    buildMenubar();
    buildBody(splitter);
    buildRightside(splitter);

    setCentralWidget(splitter);
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
}

void MainFrame::buildMenubar() {
    QMenu* fMenu = menuBar()->addMenu("File");
    fMenu->setObjectName("File");
    act_open_ = new QAction("Open", fMenu);
    act_open_->setObjectName("act_open_");

    QMenu* oMenu = menuBar()->addMenu("Operation");
    oMenu->setObjectName("Operation");
    act_hist_ = new QAction("Histogram", oMenu);
    act_hist_->setObjectName("act_hist_");

    fMenu->addAction(act_open_);
    oMenu->addAction(act_hist_);

    qDebug() << "File menu actions count: " << fMenu->actions().count();
    qDebug() << "Operation menu actions count: " << oMenu->actions().count();
}

void MainFrame::buildBody(QSplitter* splitter) {
    body_ = new JBody(splitter);
    body_->setWidth(left_width_);
    body_->setHeight(height_);
    body_->build();
}

void MainFrame::buildRightside(QSplitter* splitter) {
    rightside_ = new JRightside(splitter);
    rightside_->setWidth(right_width_);
    rightside_->setHeight(height_);
    rightside_->build();
}

void MainFrame::connectSignals() {
    // 选择图片
    connect(act_open_, &QAction::triggered, body_, &JBody::open);
}
