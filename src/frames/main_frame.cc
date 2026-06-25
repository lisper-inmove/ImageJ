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
#include <cstring>
#include <stdexcept>

#include <QAction>
#include <QComboBox>
#include <QCursor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QImageReader>
#include <QMessageBox>
#include <QMimeData>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>

#include "core/colorspace_converter.h"
#include "core/image_document.h"
#include "frames/right_sidebar.h"
#include "widgets/image_canvas.h"

// ============================================================================
// 构造 / 析构
// ============================================================================

MainFrame::MainFrame(QWidget *parent)
    : QWidget(parent), splitter_(nullptr), image_canvas_(nullptr),
      right_sidebar_(nullptr), settings_(nullptr), menu_bar_(nullptr),
      status_bar_(nullptr), pixel_info_label_(nullptr), current_colorspace_(0) {
  // 创建 QSettings 对象用于窗口状态持久化
  settings_ = new QSettings("ImageJ", "ImageJ", this);

  validateSettings();
  setAcceptDrops(true); // 启用拖放加载图像

  loadWindowSettings();
  buildUi();
  connectSignals();
}

MainFrame::~MainFrame() {
  // QSettings 和子控件由 Qt 父子关系自动清理，无需手动释放
}

// ============================================================================
// 窗口设置持久化
// ============================================================================

void MainFrame::loadWindowSettings() {
  try {
    // 从 QSettings 恢复窗口几何信息（位置和大小）
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

    // 始终以最大化状态启动
    showMaximized();
  } catch (const std::exception &e) {
    qCritical() << "Error loading window settings:" << e.what();
    setDefaultGeometry();
  }
}

void MainFrame::saveWindowSettings() {
  // 保存窗口几何信息
  settings_->setValue("Window/geometry", saveGeometry());
  settings_->sync(); // 立即写入磁盘
}

void MainFrame::closeEvent(QCloseEvent *event) {
  saveWindowSettings(); // 关闭前保存窗口状态
  event->accept();
}

// ============================================================================
// 拖放事件
// ============================================================================

void MainFrame::dragEnterEvent(QDragEnterEvent *event) {
  // 接受拖放的 URL（文件路径）
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void MainFrame::dragMoveEvent(QDragMoveEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void MainFrame::dropEvent(QDropEvent *event) {
  const QMimeData *mime = event->mimeData();
  if (!mime->hasUrls())
    return;

  QList<QUrl> urls = mime->urls();
  if (urls.isEmpty())
    return;

  // 仅处理拖放的第一个文件
  QString file_path = urls.first().toLocalFile();
  if (file_path.isEmpty())
    return;

  loadDroppedFile(file_path);
}

// ============================================================================
// 文件加载
// ============================================================================

/**
 * 根据文件类型加载图像。
 *
 * RAW 文件 (.raw/.bin/.dat)：
 * - 弹出配置对话框让用户输入宽度、高度和位深（8-bit / Unsigned 16-bit）
 * - 如果文件大小与输入尺寸不匹配，显示警告
 *
 * 标准格式（PNG/JPEG/BMP 等）：
 * - 使用 QImageReader 自动检测尺寸
 * - 弹出只读信息对话框供用户确认
 *
 * 加载成功后调用 setupLoadedDocument() 完成 UI 初始化。
 */
void MainFrame::loadDroppedFile(const QString &file_path) {
  QFileInfo fi(file_path);
  QString ext = fi.suffix().toLower();
  bool is_raw = (ext == "raw" || ext == "bin" || ext == "dat");

  ImageDocument *doc = new ImageDocument();
  bool ok = false;
  bool is_16bit = false;

  if (is_raw) {
    // ================================================================
    // RAW 文件加载流程
    // ================================================================
    QDialog config_dialog(this);
    config_dialog.setWindowTitle("拖拽加载设置");
    config_dialog.setMinimumWidth(320);

    QFormLayout *form = new QFormLayout(&config_dialog);

    QComboBox *type_combo = new QComboBox(&config_dialog);
    type_combo->addItem("8-bit");
    type_combo->addItem("Unsigned 16bit");

    QSpinBox *width_spin = new QSpinBox(&config_dialog);
    width_spin->setRange(1, 65536);
    width_spin->setValue(10240); // 默认 10240×2560（常见 RAW 传感器尺寸）

    QSpinBox *height_spin = new QSpinBox(&config_dialog);
    height_spin->setRange(1, 65536);
    height_spin->setValue(2560);

    form->addRow("Image Type:", type_combo);
    form->addRow("Width (pixels):", width_spin);
    form->addRow("Height (pixels):", height_spin);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &config_dialog);
    connect(buttons, &QDialogButtonBox::accepted, &config_dialog,
            &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &config_dialog,
            &QDialog::reject);
    form->addRow(buttons);

    if (config_dialog.exec() != QDialog::Accepted) {
      delete doc;
      return;
    }

    int width = width_spin->value();
    int height = height_spin->value();
    is_16bit = (type_combo->currentIndex() == 1);

    if (is_16bit) {
      ok = doc->load_raw_from_file(file_path.toStdString(), width, height);
      if (!ok) {
        int64_t file_size = fi.size();
        int64_t expected = static_cast<int64_t>(width) * height * 2;
        if (file_size != expected) {
          // 尺寸不匹配时给出详细提示
          QMessageBox::warning(
              this, "尺寸不匹配",
              QString("文件大小 (%1 bytes) 与输入的尺寸不匹配。\n"
                      "期望: %2 × %3 × 2 = %4 bytes")
                  .arg(file_size)
                  .arg(width)
                  .arg(height)
                  .arg(expected));
        } else {
          QMessageBox::warning(this, "加载失败", "无法加载文件: " + file_path);
        }
      }
    } else {
      // 8-bit → 灰度加载
      ok =
          doc->load_raw_from_file(file_path.toStdString(), width, height, true);
    }
  } else {
    // ================================================================
    // 标准格式加载流程
    // ================================================================
    QImageReader reader(file_path);
    if (reader.canRead() && reader.size().isValid()) {
      QSize size = reader.size();

      // 显示检测到的信息（只读）
      QDialog info_dialog(this);
      info_dialog.setWindowTitle("拖拽加载设置");
      info_dialog.setMinimumWidth(300);

      QFormLayout *form = new QFormLayout(&info_dialog);

      QLabel *type_label = new QLabel("Auto-detected (8-bit)", &info_dialog);
      QLabel *width_label =
          new QLabel(QString::number(size.width()), &info_dialog);
      QLabel *height_label =
          new QLabel(QString::number(size.height()), &info_dialog);

      form->addRow("Image Type:", type_label);
      form->addRow("Width (pixels):", width_label);
      form->addRow("Height (pixels):", height_label);

      QDialogButtonBox *buttons = new QDialogButtonBox(
          QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &info_dialog);
      connect(buttons, &QDialogButtonBox::accepted, &info_dialog,
              &QDialog::accept);
      connect(buttons, &QDialogButtonBox::rejected, &info_dialog,
              &QDialog::reject);
      form->addRow(buttons);

      if (info_dialog.exec() != QDialog::Accepted) {
        delete doc;
        return;
      }

      ok = doc->load_from_file(file_path.toStdString());
    } else {
      QMessageBox::warning(this, "不支持的文件格式",
                           "无法读取文件: " + file_path);
      delete doc;
      return;
    }
  }

  if (!ok) {
    delete doc;
    if (status_bar_) {
      status_bar_->showMessage("加载失败: " + file_path, 5000);
    }
    return;
  }

  setupLoadedDocument(doc, file_path);

  // 状态栏显示加载完成信息
  if (status_bar_) {
    const auto &data = doc->image_data();
    if (is_16bit) {
      QString info = QString("已加载: %1x%2 raw (16bit→8bit)")
                         .arg(data.width())
                         .arg(data.height());
      status_bar_->showMessage(info, 5000);
    } else {
      QString info =
          QString("已加载: %1x%2 %3")
              .arg(data.width())
              .arg(data.height())
              .arg(QString::fromStdString(doc->metadata().file_format));
      status_bar_->showMessage(info, 5000);
    }
  }
}

// ============================================================================
// UI 构建
// ============================================================================

void MainFrame::buildUi() {
  try {
    setupMenuBar();
    setupStatusBar();

    // 创建水平分割器：左侧图像画布（95%），右侧边栏（5%）
    splitter_ = new QSplitter(Qt::Horizontal, this);
    if (!splitter_) {
      throw std::runtime_error("Failed to create QSplitter");
    }

    image_canvas_ = new ImageCanvas(splitter_);
    if (!image_canvas_) {
      throw std::runtime_error("Failed to create ImageCanvas");
    }

    right_sidebar_ = new RightSidebar(splitter_);
    if (!right_sidebar_) {
      throw std::runtime_error("Failed to create RightSidebar");
    }

    // stretch 比例 19:1（95%:5%）
    splitter_->addWidget(image_canvas_);
    splitter_->addWidget(right_sidebar_);
    splitter_->setStretchFactor(0, 19);
    splitter_->setStretchFactor(1, 1);
    splitter_->setSizes({980, 20});

    // 主布局：菜单栏 → 分割器 → 状态栏
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

    setWindowTitle("ImageJ");
  } catch (const std::exception &e) {
    qCritical() << "Failed to build UI:" << e.what();
    // 创建最简后备布局，防止空窗口
    QLabel *error_label =
        new QLabel("Failed to initialize application UI", this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(error_label);
    setLayout(layout);
  }
}

void MainFrame::setupMenuBar() {
  menu_bar_ = new QMenuBar(this);

  // === 文件菜单 ===
  QMenu *file_menu = menu_bar_->addMenu("文件");

  QAction *new_action = file_menu->addAction("新建");
  new_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

  file_menu->addSeparator();

  QAction *save_action = file_menu->addAction("保存");
  save_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

  QAction *save_as_action = file_menu->addAction("另存为");
  save_as_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

  file_menu->addSeparator();

  QAction *exit_action = file_menu->addAction("退出");
  exit_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));

  // === 编辑菜单 ===
  QMenu *edit_menu = menu_bar_->addMenu("编辑");

  // 撤销/重做已替换为剪切历史列表，保留菜单项但默认禁用（占位）
  QAction *undo_action = edit_menu->addAction("撤销");
  undo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
  undo_action->setEnabled(false);

  QAction *redo_action = edit_menu->addAction("重做");
  redo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
  redo_action->setEnabled(false);

  // === 视图菜单 ===
  QMenu *view_menu = menu_bar_->addMenu("视图");

  QAction *fit_action = view_menu->addAction("适应窗口");
  fit_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));

  QAction *actual_size_action = view_menu->addAction("实际大小");

  view_menu->addSeparator();

  QAction *zoom_in_action = view_menu->addAction("放大");
  zoom_in_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));

  QAction *zoom_out_action = view_menu->addAction("缩小");
  zoom_out_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));

  // === 帮助菜单 ===
  QMenu *help_menu = menu_bar_->addMenu("帮助");

  QAction *about_action = help_menu->addAction("关于");
}

void MainFrame::setupStatusBar() {
  status_bar_ = new QStatusBar(this);
  status_bar_->setSizeGripEnabled(true); // 右下角拖拽调整大小

  // 像素信息标签：格式 "就绪 (x,y) R: xxx G: xxx B: xxx"
  pixel_info_label_ = new QLabel("就绪", status_bar_);
  status_bar_->addWidget(pixel_info_label_);
}

// ============================================================================
// 信号连接
// ============================================================================

/**
 * 建立 MainFrame 作为中枢的信号连接网络：
 *
 * ImageCanvas → RightSidebar（直接连接）：
 * - document_changed → 更新图片信息标签页
 * - selection_changed → 更新选区信息 + 微调框值
 * - selection_completed → 添加到选择历史
 * - view_changed → 更新缩放比例显示
 *
 * ImageCanvas → MainFrame（需要业务逻辑中转）：
 * - mouse_over_image → 更新状态栏像素信息
 * - save_selection_requested → 弹出保存对话框
 * - cut_selection_requested → 执行剪切并记录历史
 *
 * RightSidebar → MainFrame（需要操作 ImageCanvas）：
 * - color_space_changed → 色彩空间转换
 * - channel_gains_changed → 应用通道增益
 * - selection_size_changed → 调整选区大小
 * - history_item_selected → 恢复剪切历史快照
 * - selection_restore_requested → 恢复保存的选区
 */
void MainFrame::connectSignals() {
  // --- ImageCanvas → RightSidebar ---
  connect(image_canvas_, &ImageCanvas::document_changed, right_sidebar_,
          &RightSidebar::set_document);
  connect(image_canvas_, &ImageCanvas::selection_changed, right_sidebar_,
          &RightSidebar::update_selection_info);
  connect(image_canvas_, &ImageCanvas::selection_completed, right_sidebar_,
          &RightSidebar::add_selection);
  connect(image_canvas_, &ImageCanvas::view_changed, this, [this]() {
    right_sidebar_->set_zoom_factor(image_canvas_->zoom_factor());
  });

  // --- ImageCanvas → MainFrame ---
  connect(image_canvas_, &ImageCanvas::mouse_over_image, this,
          &MainFrame::updateStatusBarPixelInfo);

  // 右键菜单操作
  connect(image_canvas_, &ImageCanvas::save_selection_requested,
          this, &MainFrame::onSaveSelection);
  connect(image_canvas_, &ImageCanvas::cut_selection_requested,
          this, &MainFrame::onCutSelection);

  // --- RightSidebar → MainFrame ---
  // 色彩空间
  connect(right_sidebar_, &RightSidebar::color_space_changed,
          this, &MainFrame::onColorSpaceChanged);
  connect(right_sidebar_, &RightSidebar::channel_gains_changed,
          this, &MainFrame::onChannelGainsChanged);

  // 保存的选区恢复
  connect(right_sidebar_, &RightSidebar::selection_restore_requested, this,
          [this](const QRect &rect) {
            image_canvas_->set_selection(rect);
            right_sidebar_->update_selection_info(rect);
          });

  // 剪切历史
  connect(right_sidebar_, &RightSidebar::history_item_selected,
          this, &MainFrame::onHistoryItemSelected);

  // 选区大小微调框 → 画布
  connect(right_sidebar_, &RightSidebar::selection_size_changed,
          this, &MainFrame::onSelectionSizeChanged);

  // 选区变化（拖动/移动/缩放） → 更新微调框
  connect(image_canvas_, &ImageCanvas::selection_changed,
          right_sidebar_, &RightSidebar::update_selection_size_spinboxes);
}

// ============================================================================
// 文档加载后初始化
// ============================================================================

/**
 * 新文档加载完成后的初始化流程：
 *
 * 1. 替换 ImageCanvas 的文档（释放旧文档）
 * 2. 图像适应窗口大小
 * 3. 更新窗口标题
 * 4. 同步更新状态栏像素信息（如果鼠标已在画布上）
 * 5. 保存原始图像数据：original_data_（色彩空间基准）、
 *    original_image_data_（还原原始用，永不覆盖）、original_size_
 * 6. 重置色彩空间下拉框到 RGB 并启用
 * 7. 清空剪切历史（新文档=新历史）
 */
void MainFrame::setupLoadedDocument(ImageDocument *doc,
                                    const QString &file_path) {
  // 替换文档（释放旧文档）
  ImageDocument *old_doc = image_canvas_->document();
  image_canvas_->set_document(doc);
  delete old_doc;

  image_canvas_->fit_to_window();
  setWindowTitle(QString::fromStdString(doc->file_name()) + " - ImageJ");

  // 同步更新状态栏（鼠标可能已在画布上）
  QPoint canvas_pos = image_canvas_->mapFromGlobal(QCursor::pos());
  if (image_canvas_->rect().contains(canvas_pos)) {
    updateStatusBarPixelInfo(image_canvas_->canvas_to_image(canvas_pos));
  }

  // 保存三种"原始"数据
  original_data_.copy_from(doc->image_data());
  original_image_data_.copy_from(doc->image_data()); // 永不覆盖的原始数据
  original_size_ = QSize(doc->image_data().width(), doc->image_data().height());

  // 重置色彩空间
  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }

  // 清空剪切历史
  cut_history_.clear();
  if (right_sidebar_) {
    right_sidebar_->clear_cut_history();
  }
}

// ============================================================================
// 状态栏像素信息
// ============================================================================

/**
 * 更新状态栏像素信息标签。
 *
 * 显示格式：
 * - 无文档或无像素："就绪"
 * - 灰度："就绪 (x,y) value"
 * - RGB："就绪 (x,y) R:xxx G:xxx B:xxx"
 * - RGBA："就绪 (x,y) R:xxx G:xxx B:xxx A:xxx"
 */
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

  // 仅在坐标在图像范围内时读取像素值
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
                  .arg(p[0])
                  .arg(p[1])
                  .arg(p[2])
                  .arg(p[3]);
      break;
    default:
      break;
    }
  }

  pixel_info_label_->setText(text);
}

// ============================================================================
// 色彩空间和通道增益
// ============================================================================

/**
 * 色彩空间转换处理。
 *
 * index = 0 (RGB)：复制 original_data_，直接应用通道增益
 * index = 1-4：先转换色彩空间，再应用通道增益
 *
 * 转换结果通过 doc->set_image_data() 直接更新显示。
 */
void MainFrame::onColorSpaceChanged(int index) {
  ImageDocument *doc = image_canvas_->document();
  if (!doc || !original_data_.is_valid()) {
    return;
  }

  current_colorspace_ = index;

  if (index == 0) {
    // RGB：直接使用原始数据 + 通道增益
    ImageData rgb_copy;
    rgb_copy.copy_from(original_data_);
    applyChannelGains(rgb_copy);
    doc->set_image_data(rgb_copy);
    image_canvas_->update();
    return;
  }

  // 映射索引到 ColorSpace 枚举
  ColorSpace target;
  switch (index) {
  case 1:
    target = ColorSpace::kHSV;
    break;
  case 2:
    target = ColorSpace::kLAB;
    break;
  case 3:
    target = ColorSpace::kGray;
    break;
  case 4:
    target = ColorSpace::kBinary;
    break;
  default:
    return;
  }

  // 先转换色彩空间，再应用增益
  ImageData converted = convertColorSpace(original_data_, target);
  if (converted.is_valid()) {
    applyChannelGains(converted);
    doc->set_image_data(converted);
    image_canvas_->update();
  }
}

void MainFrame::onChannelGainsChanged(const QVector<int> &gains) {
  channel_gains_ = gains;
  // 用新的增益值重新触发色彩空间转换
  onColorSpaceChanged(current_colorspace_);
}

/**
 * 应用通道增益到图像数据。
 *
 * Binary 模式（colorspace == 4）：
 * - gains[0] 是阈值，>= 阈值的像素 → 255，< 阈值 → 0
 *
 * 其他模式：
 * - 每个通道独立乘以增益系数（gain / max_gain）
 * - 对于 HSV，H 通道的 max_gain = 180（OpenCV 使用 0-180 表示色相）
 * - 对于其他通道，max_gain = 255
 * - 结果 clamp 到 [0, 255]
 */
void MainFrame::applyChannelGains(ImageData &data) {
  if (channel_gains_.isEmpty()) {
    return;
  }

  int channels = data.channels();
  int w = data.width();
  int h = data.height();

  // Binary 模式：阈值处理
  if (current_colorspace_ == 4 &&
      data.format() == ImageData::PixelFormat::kGray8) {
    int threshold = channel_gains_[0];
    for (int y = 0; y < h; ++y) {
      uint8_t *row = data.pixel(0, y);
      for (int x = 0; x < w; ++x) {
        row[x] = (row[x] >= threshold) ? 255 : 0;
      }
    }
    return;
  }

  // 逐通道增益（乘法器）
  for (int y = 0; y < h; ++y) {
    uint8_t *row = data.pixel(0, y);
    for (int x = 0; x < w; ++x) {
      uint8_t *p = row + x * channels;
      for (int ch = 0; ch < channels && ch < channel_gains_.size(); ++ch) {
        int gain = channel_gains_[ch];
        int max_gain = 255;
        // H 通道范围是 0-180（OpenCV）
        if (current_colorspace_ == 1 && ch == 0) {
          max_gain = 180;
        }
        int val = (static_cast<int>(p[ch]) * gain) / max_gain;
        p[ch] = static_cast<uint8_t>(val < 0 ? 0 : (val > 255 ? 255 : val));
      }
    }
  }
}

// ============================================================================
// extractSelection() — 提取选区像素（自由函数）
// ============================================================================

/**
 * 从源图像中提取矩形区域的像素到新的 ImageData。
 *
 * 逐行使用 memcpy 复制，每个像素复制 channels 个字节。
 * 调用者应确保 rect 在 src 边界内（本函数不做边界检查）。
 *
 * @param src 源图像数据
 * @param rect 要提取的区域
 * @return 尺寸为 rect.size()、格式与 src 相同的新 ImageData
 */
ImageData extractSelection(const ImageData& src, const QRect& rect) {
  ImageData dst;
  dst.create(rect.width(), rect.height(), src.format());
  int channels = src.channels();
  for (int y = 0; y < rect.height(); ++y) {
    const uint8_t* src_row = src.pixel(rect.x(), rect.y() + y);
    uint8_t* dst_row = dst.pixel(0, y);
    std::memcpy(dst_row, src_row, rect.width() * channels);
  }
  return dst;
}

// ============================================================================
// 选区操作
// ============================================================================

/**
 * "另存为"操作：
 * 提取当前选区像素 → 弹出保存文件对话框 → 保存为图像文件。
 *
 * 默认文件扩展名从文档元数据中获取（如 "png"），
 * 也可以通过对话框选择其他格式。
 */
void MainFrame::onSaveSelection() {
  ImageDocument* doc = image_canvas_->document();
  if (!doc || !doc->is_valid()) return;

  QRect sel = image_canvas_->selection();
  if (!sel.isValid()) return;

  ImageData extracted = extractSelection(doc->image_data(), sel);

  // 从文档元数据获取默认扩展名
  QString default_ext;
  const auto& meta = doc->metadata();
  if (!meta.file_format.empty()) {
    default_ext = QString::fromStdString(meta.file_format);
  } else {
    default_ext = "png";
  }

  QString save_path = QFileDialog::getSaveFileName(
      this, "保存选中区域", QString(),
      "Images (*." + default_ext + ");;All Files (*)");

  if (save_path.isEmpty()) return;

  // 使用临时 ImageDocument 完成保存
  ImageDocument temp_doc;
  temp_doc.set_image_data(extracted);
  if (temp_doc.save_as(save_path.toStdString())) {
    if (status_bar_) {
      status_bar_->showMessage("已保存: " + save_path, 5000);
    }
  } else {
    QMessageBox::warning(this, "保存失败", "无法保存文件: " + save_path);
  }
}

/**
 * "剪切"操作（核心业务逻辑）：
 *
 * 流程：
 * 1. 记录剪切前尺寸（before_size，用于历史列表显示）
 * 2. 调用 extractSelection() 提取选区像素
 * 3. 将提取的图像设置回文档（替换原图）
 * 4. 创建 CutHistoryEntry 快照：
 *    - index = cut_history_.size() + 1（1-based 序号）
 *    - image_data = 剪切后的完整图像副本
 *    - before_size / after_size 记录尺寸变化
 * 5. 清除选区
 * 6. 重置色彩空间状态（回到 RGB）
 * 7. 通知侧边栏添加历史条目并重置色彩空间下拉框
 *
 * 注意：original_image_data_ 不在此处修改——它保存的是文件加载时的原始图像，
 * 仅用于"还原原始"操作。
 */
void MainFrame::onCutSelection() {
  ImageDocument* doc = image_canvas_->document();
  if (!doc || !doc->is_valid()) return;

  QRect sel = image_canvas_->selection();
  if (!sel.isValid()) return;

  // 记录剪切前的图像尺寸
  QSize before_size(doc->image_data().width(), doc->image_data().height());

  // 提取选区并替换文档图像
  ImageData extracted = extractSelection(doc->image_data(), sel);
  doc->set_image_data(extracted);

  // 推送快照到剪切历史
  CutHistoryEntry entry;
  entry.index = cut_history_.size() + 1; // 1-based 序号
  entry.image_data.copy_from(doc->image_data()); // 保存剪切后图像的副本
  entry.before_size = before_size;
  entry.after_size = QSize(extracted.width(), extracted.height());
  cut_history_.append(entry);

  image_canvas_->clear_selection();

  // 重置状态（等同于加载了新图像）
  current_colorspace_ = 0;
  image_canvas_->fit_to_window();
  original_data_.copy_from(doc->image_data()); // 更新色彩空间基准

  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
    // 构造历史列表显示文本："1. 1920×1080 → 320×240"
    QString label = QString("%1. %2×%3 → %4×%5")
                        .arg(entry.index)
                        .arg(entry.before_size.width())
                        .arg(entry.before_size.height())
                        .arg(entry.after_size.width())
                        .arg(entry.after_size.height());
    right_sidebar_->add_cut_history_entry(label);
  }

  image_canvas_->update();

  if (status_bar_) {
    const auto& data = doc->image_data();
    status_bar_->showMessage(
        QString("已裁剪至: %1x%2").arg(data.width()).arg(data.height()), 5000);
  }
}

// ============================================================================
// 选区大小微调框处理
// ============================================================================

/**
 * 选区大小微调框变化 → 转发给 ImageCanvas 调整选区。
 * 由 selection_size_changed 信号触发，信号已将 width/height 打包，
 * 这里只是简单转发。
 */
void MainFrame::onSelectionSizeChanged(int width, int height) {
  if (image_canvas_) {
    image_canvas_->resize_selection(width, height);
  }
}

// ============================================================================
// 剪切历史恢复
// ============================================================================

/**
 * 剪切历史项选择处理。
 *
 * @param index -1 表示恢复原始图像（original_image_data_），
 *               >=0 表示恢复 cut_history_[index] 的快照。
 *
 * 恢复流程：
 * 1. 设置文档图像为快照数据
 * 2. 更新 original_data_（后续色彩空间转换以此为基准）
 * 3. 清除选区、适应窗口、重绘
 * 4. 重置色彩空间到 RGB
 *
 * 如果 index 越界（< -1 或 >= cut_history_.size()），不做任何操作。
 */
void MainFrame::onHistoryItemSelected(int index) {
  ImageDocument* doc = image_canvas_->document();
  if (!doc) return;

  if (index == -1) {
    // "还原原始"按钮 → 恢复第一次加载时的原始图像
    doc->set_image_data(original_image_data_);
    original_data_.copy_from(original_image_data_);
  } else if (index >= 0 && index < cut_history_.size()) {
    // 点击历史列表项 → 恢复对应快照
    doc->set_image_data(cut_history_[index].image_data);
    original_data_.copy_from(cut_history_[index].image_data);
  } else {
    return;
  }

  image_canvas_->clear_selection();
  image_canvas_->fit_to_window();
  image_canvas_->update();

  current_colorspace_ = 0;
  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }
}

// ============================================================================
// 窗口默认值
// ============================================================================

void MainFrame::setDefaultGeometry() {
  resize(1280, 720);
  showMaximized();
}

bool MainFrame::validateSettings() {
  // 检查是否有保存的配置，没有则使用默认值
  if (!settings_->contains("Window/geometry") &&
      !settings_->contains("Layout/splitterSizes")) {
    qDebug() << "No saved settings found, using defaults";
    return false;
  }
  return true;
}
