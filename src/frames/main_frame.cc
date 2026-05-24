#include "frames/main_frame.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <stdexcept>

#include <QAction>
#include <QFileDialog>

#include "core/image_document.h"
#include "frames/right_sidebar.h"
#include "widgets/image_canvas.h"

MainFrame::MainFrame(QWidget *parent)
    : QWidget(parent), splitter_(nullptr), image_canvas_(nullptr),
      right_sidebar_(nullptr), settings_(nullptr), menu_bar_(nullptr),
      status_bar_(nullptr) {
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
    QByteArray geometry_data =
        settings_->value("Window/geometry").toByteArray();
    if (!geometry_data.isEmpty()) {
      if (!restoreGeometry(geometry_data)) {
        qWarning() << "Failed to restore window geometry, using defaults";
        setDefaultGeometry();
      }
    } else {
      setDefaultGeometry();
    }

    // 加载窗口状态（最大化/正常） - 使用Qt::WindowState枚举值
    int window_state =
        settings_->value("Window/windowState", Qt::WindowNoState).toInt();
    setWindowState(static_cast<Qt::WindowState>(window_state));
  } catch (const std::exception &e) {
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

void MainFrame::closeEvent(QCloseEvent *event) {
  saveWindowSettings();
  event->accept();
}

void MainFrame::buildUi() {
  try {
    // 创建菜单栏
    setupMenuBar();

    // 创建状态栏
    setupStatusBar();

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
    sizes << 800 << 200; // 基于默认1024宽度计算
    splitter_->setSizes(sizes);

    // 尝试加载保存的splitter状态
    QByteArray splitter_state =
        settings_->value("Layout/splitterSizes").toByteArray();
    if (!splitter_state.isEmpty()) {
      splitter_->restoreState(splitter_state);
    }

    // 设置主布局（菜单栏→分割器→状态栏）
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    if (menu_bar_) {
      main_layout->addWidget(menu_bar_, 0);
    }
    main_layout->addWidget(splitter_, 1);
    if (status_bar_) {
      main_layout->addWidget(status_bar_, 0);
    }
    setLayout(main_layout);

    // 设置窗口标题和默认大小
    setWindowTitle("ImageJ");
    resize(1024, 768);
  } catch (const std::exception &e) {
    qCritical() << "Failed to build UI:" << e.what();
    // 创建最简单的后备布局
    QLabel *error_label =
        new QLabel("Failed to initialize application UI", this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(error_label);
    setLayout(layout);
  }
}

void MainFrame::setupMenuBar() {
  menu_bar_ = new QMenuBar(this);

  // === File menu ===
  QMenu *file_menu = menu_bar_->addMenu("文件");

  QAction *new_action = file_menu->addAction("新建");
  new_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

  QAction *open_action = file_menu->addAction("打开");
  open_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));

  file_menu->addSeparator();

  QAction *save_action = file_menu->addAction("保存");
  save_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

  QAction *save_as_action = file_menu->addAction("另存为");
  save_as_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

  file_menu->addSeparator();

  QAction *exit_action = file_menu->addAction("退出");
  exit_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));

  // === Edit menu ===
  QMenu *edit_menu = menu_bar_->addMenu("编辑");

  QAction *undo_action = edit_menu->addAction("撤销");
  undo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
  undo_action->setEnabled(false);

  QAction *redo_action = edit_menu->addAction("重做");
  redo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
  redo_action->setEnabled(false);

  // === View menu ===
  QMenu *view_menu = menu_bar_->addMenu("视图");

  QAction *fit_action = view_menu->addAction("适应窗口");
  fit_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));

  QAction *actual_size_action = view_menu->addAction("实际大小");
  actual_size_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));

  view_menu->addSeparator();

  QAction *zoom_in_action = view_menu->addAction("放大");
  zoom_in_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));

  QAction *zoom_out_action = view_menu->addAction("缩小");
  zoom_out_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));

  // === Help menu ===
  QMenu *help_menu = menu_bar_->addMenu("帮助");

  QAction *about_action = help_menu->addAction("关于");
}

void MainFrame::setupStatusBar() {
  status_bar_ = new QStatusBar(this);
  status_bar_->setSizeGripEnabled(true);

  // 左侧默认消息（使用普通 widget，不受 showMessage/clearMessage 影响）
  status_bar_->addWidget(new QLabel("就绪", status_bar_));

  // 右侧永久标签
  status_bar_->addPermanentWidget(new QLabel("图像信息", status_bar_));
}

void MainFrame::connectSignals() {
  // Connect all "打开" actions to openImage()
  QList<QAction *> actions = findChildren<QAction *>();
  for (QAction *action : actions) {
    if (action->text() == "打开") {
      QObject::connect(action, &QAction::triggered,
                       this, &MainFrame::openImage);
    }
  }
}

void MainFrame::openImage() {
  QString file_path = QFileDialog::getOpenFileName(
      this, "打开图像", QString(),
      "Images (*.png *.jpg *.jpeg *.bmp *.tiff *.webp);;All Files (*)");

  if (file_path.isEmpty()) {
    return;
  }

  ImageDocument *doc = new ImageDocument();
  if (!doc->load_from_file(file_path.toStdString())) {
    delete doc;
    if (status_bar_) {
      status_bar_->showMessage("加载失败: " + file_path, 5000);
    }
    return;
  }

  // Pass ownership to image canvas (old document will be deleted if owned)
  ImageDocument *old_doc = image_canvas_->document();
  image_canvas_->set_document(doc);
  delete old_doc;

  // Update window title
  setWindowTitle(QString::fromStdString(doc->file_name()) +
                 " - ImageJ");

  // Update status bar
  if (status_bar_) {
    const auto &data = doc->image_data();
    QString info = QString("已加载: %1x%2 %3")
                       .arg(data.width())
                       .arg(data.height())
                       .arg(QString::fromStdString(doc->metadata().file_format));
    status_bar_->showMessage(info, 5000);
  }
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
