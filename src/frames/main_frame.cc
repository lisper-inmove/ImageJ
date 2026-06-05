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
#include <QCursor>
#include <QFileDialog>
#include <QFileInfo>

#include "core/colorspace_converter.h"
#include "core/image_document.h"
#include "frames/right_sidebar.h"
#include "widgets/image_canvas.h"

MainFrame::MainFrame(QWidget *parent)
    : QWidget(parent), splitter_(nullptr), image_canvas_(nullptr),
      right_sidebar_(nullptr), settings_(nullptr), menu_bar_(nullptr),
      recent_menu_(nullptr),
      status_bar_(nullptr), pixel_info_label_(nullptr),
      current_colorspace_(0) {
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

  // Recent files submenu
  file_menu->addSeparator();
  recent_menu_ = file_menu->addMenu("最近文件");
  updateRecentFileMenu();

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

  // Pixel info label: "就绪 (x,y) R: xxx G: xxx B: xxx"
  pixel_info_label_ = new QLabel("就绪", status_bar_);
  status_bar_->addWidget(pixel_info_label_);
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

  // Connect ImageCanvas signals to RightSidebar
  connect(image_canvas_, &ImageCanvas::document_changed,
          right_sidebar_, &RightSidebar::set_document);
  connect(image_canvas_, &ImageCanvas::mouse_over_image,
          this, &MainFrame::updateStatusBarPixelInfo);
  connect(image_canvas_, &ImageCanvas::selection_changed,
          right_sidebar_, &RightSidebar::update_selection_info);
  connect(image_canvas_, &ImageCanvas::selection_completed,
          right_sidebar_, &RightSidebar::add_selection);
  connect(right_sidebar_, &RightSidebar::selection_restore_requested, this,
          [this](const QRect &rect) {
            image_canvas_->set_selection(rect);
            right_sidebar_->update_selection_info(rect);
          });
  connect(image_canvas_, &ImageCanvas::view_changed, this, [this]() {
    right_sidebar_->set_zoom_factor(image_canvas_->zoom_factor());
  });

  // Color space combo (in sidebar tools tab)
  connect(right_sidebar_, &RightSidebar::color_space_changed,
          this, &MainFrame::onColorSpaceChanged);
  connect(right_sidebar_, &RightSidebar::channel_gains_changed,
          this, &MainFrame::onChannelGainsChanged);
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

  // Update pixel info immediately (cursor may already be over canvas)
  QPoint canvas_pos = image_canvas_->mapFromGlobal(QCursor::pos());
  if (image_canvas_->rect().contains(canvas_pos)) {
    updateStatusBarPixelInfo(
        image_canvas_->canvas_to_image(canvas_pos));
  }

  // Save original data for color space switching
  original_data_.copy_from(doc->image_data());

  // Enable and reset color space combo in sidebar
  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }

  // Save to recent files
  addRecentFilePath(file_path);

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

void MainFrame::updateStatusBarPixelInfo(const QPoint &image_pos) {
  if (!pixel_info_label_) {
    return;
  }

  ImageDocument *doc = image_canvas_->document();
  if (!doc || !doc->is_valid()) {
    pixel_info_label_->setText("就绪");
    return;
  }

  const auto &data = doc->image_data();
  int x = image_pos.x();
  int y = image_pos.y();

  QString text = QString("就绪 (%1,%2)").arg(x).arg(y);

  if (x >= 0 && x < data.width() && y >= 0 && y < data.height()) {
    const uint8_t *p = data.pixel(x, y);
    switch (data.format()) {
      case ImageData::PixelFormat::kGray8:
        text += QString(" %1").arg(p[0]);
        break;
      case ImageData::PixelFormat::kRGB24:
        text += QString(" R:%1 G:%2 B:%3").arg(p[0]).arg(p[1]).arg(p[2]);
        break;
      case ImageData::PixelFormat::kRGBA32:
        text += QString(" R:%1 G:%2 B:%3 A:%4")
                    .arg(p[0]).arg(p[1]).arg(p[2]).arg(p[3]);
        break;
      default:
        break;
    }
  }

  pixel_info_label_->setText(text);
}

void MainFrame::onColorSpaceChanged(int index) {
  ImageDocument *doc = image_canvas_->document();
  if (!doc || !original_data_.is_valid()) {
    return;
  }

  current_colorspace_ = index;

  if (index == 0) {
    // RGB: copy original, apply gains, set
    ImageData rgb_copy;
    rgb_copy.copy_from(original_data_);
    applyChannelGains(rgb_copy);
    doc->set_image_data(rgb_copy);
    image_canvas_->update();
    return;
  }

  ColorSpace target;
  switch (index) {
    case 1: target = ColorSpace::kHSV;    break;
    case 2: target = ColorSpace::kLAB;    break;
    case 3: target = ColorSpace::kGray;   break;
    case 4: target = ColorSpace::kBinary; break;
    default: return;
  }

  ImageData converted = convertColorSpace(original_data_, target);
  if (converted.is_valid()) {
    applyChannelGains(converted);
    doc->set_image_data(converted);
    image_canvas_->update();
  }
}

void MainFrame::addRecentFilePath(const QString& path) {
  // Read current list from settings
  QStringList paths;
  int count = settings_->beginReadArray("RecentFiles");
  for (int i = 0; i < count; ++i) {
    settings_->setArrayIndex(i);
    QString p = settings_->value("path").toString();
    if (!p.isEmpty()) {
      paths.append(p);
    }
  }
  settings_->endArray();

  // Deduplicate: remove existing entry with same path
  paths.removeAll(path);

  // Prepend new path, keep max 5
  paths.prepend(path);
  while (paths.size() > 5) {
    paths.removeLast();
  }

  // Write back to settings
  settings_->beginWriteArray("RecentFiles");
  for (int i = 0; i < paths.size(); ++i) {
    settings_->setArrayIndex(i);
    settings_->setValue("path", paths[i]);
  }
  settings_->endArray();

  updateRecentFileMenu();
}

void MainFrame::updateRecentFileMenu() {
  if (!recent_menu_) {
    return;
  }
  recent_menu_->clear();

  int count = settings_->beginReadArray("RecentFiles");
  if (count == 0) {
    recent_menu_->setEnabled(false);
    QAction* empty = recent_menu_->addAction("(空)");
    empty->setEnabled(false);
    settings_->endArray();
    return;
  }

  recent_menu_->setEnabled(true);
  for (int i = 0; i < count; ++i) {
    settings_->setArrayIndex(i);
    QString path = settings_->value("path").toString();
    if (path.isEmpty()) {
      continue;
    }

    QAction* action = recent_menu_->addAction(QFileInfo(path).fileName());
    action->setToolTip(path);
    action->setData(path);
    connect(action, &QAction::triggered, this, &MainFrame::openRecentFile);
  }
  settings_->endArray();
}

void MainFrame::openRecentFile() {
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action) {
    return;
  }

  QString file_path = action->data().toString();
  if (file_path.isEmpty() || !QFileInfo::exists(file_path)) {
    if (status_bar_) {
      status_bar_->showMessage("文件不存在: " + file_path, 5000);
    }
    return;
  }

  ImageDocument* doc = new ImageDocument();
  if (!doc->load_from_file(file_path.toStdString())) {
    delete doc;
    if (status_bar_) {
      status_bar_->showMessage("加载失败: " + file_path, 5000);
    }
    return;
  }

  // Pass ownership to image canvas
  ImageDocument* old_doc = image_canvas_->document();
  image_canvas_->set_document(doc);
  delete old_doc;

  // Update window title
  setWindowTitle(QString::fromStdString(doc->file_name()) + " - ImageJ");

  // Update pixel info immediately
  QPoint canvas_pos = image_canvas_->mapFromGlobal(QCursor::pos());
  if (image_canvas_->rect().contains(canvas_pos)) {
    updateStatusBarPixelInfo(image_canvas_->canvas_to_image(canvas_pos));
  }

  // Save original data for color space switching
  original_data_.copy_from(doc->image_data());

  // Enable and reset color space combo in sidebar
  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }

  // Update recent files (moves this path to top)
  addRecentFilePath(file_path);

  // Update status bar
  if (status_bar_) {
    const auto& data = doc->image_data();
    QString info = QString("已加载: %1x%2 %3")
                       .arg(data.width())
                       .arg(data.height())
                       .arg(QString::fromStdString(doc->metadata().file_format));
    status_bar_->showMessage(info, 5000);
  }
}

void MainFrame::onChannelGainsChanged(const QVector<int>& gains) {
  channel_gains_ = gains;
  // Re-trigger conversion with new gains
  onColorSpaceChanged(current_colorspace_);
}

void MainFrame::applyChannelGains(ImageData& data) {
  if (channel_gains_.isEmpty()) {
    return;
  }

  int channels = data.channels();
  int w = data.width();
  int h = data.height();

  // Binary mode: gains[0] is threshold value
  if (current_colorspace_ == 4 && data.format() == ImageData::PixelFormat::kGray8) {
    int threshold = channel_gains_[0];
    for (int y = 0; y < h; ++y) {
      uint8_t* row = data.pixel(0, y);
      for (int x = 0; x < w; ++x) {
        row[x] = (row[x] >= threshold) ? 255 : 0;
      }
    }
    return;
  }

  // Per-channel gain (multiplier)
  for (int y = 0; y < h; ++y) {
    uint8_t* row = data.pixel(0, y);
    for (int x = 0; x < w; ++x) {
      uint8_t* p = row + x * channels;
      for (int ch = 0; ch < channels && ch < channel_gains_.size(); ++ch) {
        int gain = channel_gains_[ch];
        // Default gain is max (e.g. 255 → identity); scale accordingly
        int max_gain = 255;
        if (current_colorspace_ == 1 && ch == 0) {
          max_gain = 180;  // H channel
        }
        int val = (static_cast<int>(p[ch]) * gain) / max_gain;
        p[ch] = static_cast<uint8_t>(val < 0 ? 0 : (val > 255 ? 255 : val));
      }
    }
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
