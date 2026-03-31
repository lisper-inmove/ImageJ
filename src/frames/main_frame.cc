#include "frames/main_frame.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QSettings>
#include <QCloseEvent>
#include <QDebug>
#include <QLabel>
#include <stdexcept>

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

    // 验证配置
    validateSettings();

    loadWindowSettings();
    buildUi();
    connectSignals();
}

MainFrame::~MainFrame() {
    // QSettings和子控件由Qt父子关系自动清理
}

void MainFrame::loadWindowSettings() {
    try {
        // 加载窗口几何信息
        QByteArray geometry_data = settings_->value("Window/geometry").toByteArray();
        if (!geometry_data.isEmpty()) {
            if (!restoreGeometry(geometry_data)) {
                qWarning() << "Failed to restore window geometry, using defaults";
                setDefaultGeometry();
            }
        } else {
            setDefaultGeometry();
        }

        // 加载窗口状态（最大化/正常） - 使用Qt::WindowState枚举值
        int window_state = settings_->value("Window/windowState", Qt::WindowNoState).toInt();
        setWindowState(static_cast<Qt::WindowState>(window_state));
    } catch (const std::exception& e) {
        qCritical() << "Error loading window settings:" << e.what();
        setDefaultGeometry();
    }

    // splitter状态在buildUi之后加载
}

void MainFrame::saveWindowSettings() {
    // 保存窗口几何信息
    settings_->setValue("Window/geometry", saveGeometry());

    // 保存窗口状态 - 使用Qt::WindowState枚举值
    settings_->setValue("Window/windowState", static_cast<int>(windowState()));

    // 保存splitter状态
    if (splitter_) {
        settings_->setValue("Layout/splitterSizes", splitter_->saveState());
    }

    // 确保立即写入磁盘
    settings_->sync();
}

void MainFrame::closeEvent(QCloseEvent* event) {
    saveWindowSettings();
    event->accept();
}

void MainFrame::buildUi() {
    try {
        // 创建分割器
        splitter_ = new QSplitter(Qt::Horizontal, this);
        if (!splitter_) {
            throw std::runtime_error("Failed to create QSplitter");
        }

        // 创建左侧图像画布
        image_canvas_ = new ImageCanvas(splitter_);
        if (!image_canvas_) {
            throw std::runtime_error("Failed to create ImageCanvas");
        }

        // 创建右侧边栏
        right_sidebar_ = new RightSidebar(splitter_);
        if (!right_sidebar_) {
            throw std::runtime_error("Failed to create RightSidebar");
        }

        // 添加到分割器
        splitter_->addWidget(image_canvas_);
        splitter_->addWidget(right_sidebar_);

        // 设置初始分割比例 (80% : 20%)
        QList<int> sizes;
        sizes << 800 << 200;  // 基于默认1024宽度计算
        splitter_->setSizes(sizes);

        // 尝试加载保存的splitter状态
        QByteArray splitter_state = settings_->value("Layout/splitterSizes").toByteArray();
        if (!splitter_state.isEmpty()) {
            splitter_->restoreState(splitter_state);
        }

        // 设置主布局
        QHBoxLayout* main_layout = new QHBoxLayout(this);
        main_layout->addWidget(splitter_);
        setLayout(main_layout);

        // 设置窗口标题和默认大小
        setWindowTitle("ImageJ");
        resize(1024, 768);
    } catch (const std::exception& e) {
        qCritical() << "Failed to build UI:" << e.what();
        // 创建最简单的后备布局
        QLabel* error_label = new QLabel("Failed to initialize application UI", this);
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(error_label);
        setLayout(layout);
    }
}

void MainFrame::connectSignals() {
}

void MainFrame::setDefaultGeometry() {
    resize(1024, 768);
    move(100, 100);
}

bool MainFrame::validateSettings() {
    // 检查必要的配置键是否存在且有效
    if (!settings_->contains("Window/geometry") &&
        !settings_->contains("Layout/splitterSizes")) {
        qDebug() << "No saved settings found, using defaults";
        return false;
    }
    return true;
}
