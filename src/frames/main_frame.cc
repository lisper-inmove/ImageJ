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
    // 创建分割器
    splitter_ = new QSplitter(Qt::Horizontal, this);

    // 创建左侧图像画布
    image_canvas_ = new ImageCanvas(splitter_);

    // 创建右侧边栏
    right_sidebar_ = new RightSidebar(splitter_);

    // 添加到分割器
    splitter_->addWidget(image_canvas_);
    splitter_->addWidget(right_sidebar_);

    // 设置初始分割比例 (80% : 20%)
    QList<int> sizes;
    sizes << 800 << 200;  // 基于默认1024宽度计算
    splitter_->setSizes(sizes);

    // 设置主布局
    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->addWidget(splitter_);
    setLayout(main_layout);

    // 设置窗口标题和默认大小
    setWindowTitle("ImageJ");
    resize(1024, 768);
}

void MainFrame::connectSignals() {
}
