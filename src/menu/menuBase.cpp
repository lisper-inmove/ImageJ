#include "menu/menu.h"

Menu::Menu(QMainWindow* main): QObject(main), mw_(main) {}

void Menu::build() {
    buildMFile();
    buildMOperate();
}

void Menu::buildMFile() {
    mFile_ = mw_->menuBar()->addMenu(tr("File"));
    actOpen_ = new QAction(tr("Open"), this);
    mFile_->addAction(actOpen_);
    connect(actOpen_, &QAction::triggered, this, [&](){
        emit openRequested();
    });
}

void Menu::buildMOperate() {
    mOperate_ = mw_->menuBar()->addMenu(tr("Operate"));
    actHistogram_ = new QAction(tr("Histogram"), this);
    mOperate_->addAction(actHistogram_);
    connect(actHistogram_, &QAction::triggered, this, [&]() {
        emit histogramRequested();
    });
}
