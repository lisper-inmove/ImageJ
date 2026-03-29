#include "frames/main_frame.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QSettings>
#include <QCloseEvent>

#include "widgets/image_canvas.h"
#include "frames/right_sidebar.h"

MainFrame::MainFrame(QWidget* parent)
    : QWidget(parent),
      splitter_(nullptr),
      image_canvas_(nullptr),
      right_sidebar_(nullptr),
      settings_(nullptr) {
    // 创建配置对象
    settings_ = new QSettings("ImageJ", "ImageJ", this);

    loadWindowSettings();
    buildUi();
    connectSignals();
}

MainFrame::~MainFrame() {
    // QSettings和子控件由Qt父子关系自动清理
}

void MainFrame::loadWindowSettings() {
    // 暂时为空实现，后续任务中完善
}

void MainFrame::saveWindowSettings() {
    // 暂时为空实现，后续任务中完善
}

void MainFrame::closeEvent(QCloseEvent* event) {
    saveWindowSettings();
    event->accept();
}

void MainFrame::buildUi() {
}

void MainFrame::connectSignals() {
}
