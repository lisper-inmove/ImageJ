#include "frames/right_sidebar.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include <opencv2/imgproc.hpp>

#include "core/image_data.h"
#include "core/image_document.h"
#include "core/image_document_adapter.h"
#include "dialogs/histogram_dialog.h"

// ============================================================================
// 构造 / 析构
// ============================================================================

RightSidebar::RightSidebar(QWidget *parent)
    : QWidget(parent), tabs_(nullptr), info_tab_(nullptr), size_label_(nullptr),
      type_label_(nullptr), zoom_label_(nullptr), rotation_label_(nullptr),
      selection_info_label_(nullptr), tools_tab_(nullptr),
      histogram_btn_(nullptr), equalize_hist_btn_(nullptr), clahe_btn_(nullptr),
      colorspace_combo_(nullptr), channel_sliders_widget_(nullptr),
      channel_sliders_layout_(nullptr), selection_list_(nullptr),
      selection_width_spin_(nullptr), selection_height_spin_(nullptr),
      selection_size_widget_(nullptr), cut_history_list_(nullptr),
      restore_original_btn_(nullptr), blur_group_(nullptr),
      blur_ksize_spin_(nullptr), blur_anchor_x_spin_(nullptr),
      blur_anchor_y_spin_(nullptr), blur_border_combo_(nullptr),
      blur_apply_btn_(nullptr), document_(nullptr), zoom_factor_(1.0),
      rotation_(0.0), main_layout_(nullptr) {
  buildUi();
}

// ============================================================================
// UI 构建
// ============================================================================

/**
 * 构建四个标签页的完整 UI：
 *
 * Tab 1 "图片信息"：使用 QFormLayout 显示图像元数据
 * Tab 2 "工具"：色彩空间下拉框 → 通道增益滑块 → 选区大小微调框 → 直方图按钮
 * Tab 3 "选择历史"：QListWidget 显示保存的选区缩略图和坐标
 * Tab 4 "剪切历史"："还原原始"按钮 + QListWidget 显示剪切历史
 */
void RightSidebar::buildUi() {
  main_layout_ = new QVBoxLayout(this);
  main_layout_->setContentsMargins(4, 4, 4, 4);

  // --- 标签页容器 ---
  tabs_ = new QTabWidget(this);

  // ========================================================================
  // Tab 1: 图片信息
  // ========================================================================
  info_tab_ = new QWidget();
  QFormLayout *info_layout = new QFormLayout(info_tab_);
  info_layout->setContentsMargins(8, 8, 8, 8);

  size_label_ = new QLabel("-", info_tab_);
  size_label_->setWordWrap(true);
  type_label_ = new QLabel("-", info_tab_);
  type_label_->setWordWrap(true);
  zoom_label_ = new QLabel("100%", info_tab_);
  rotation_label_ = new QLabel("0°", info_tab_);
  selection_info_label_ = new QLabel("-", info_tab_);
  selection_info_label_->setWordWrap(true);

  info_layout->addRow("图片大小:", size_label_);
  info_layout->addRow("类型:", type_label_);
  info_layout->addRow("缩放比例:", zoom_label_);
  info_layout->addRow("旋转角度:", rotation_label_);
  info_layout->addRow("选中区域:", selection_info_label_);

  tabs_->addTab(info_tab_, "图片信息");

  // ========================================================================
  // Tab 2: 工具
  // ========================================================================
  tools_tab_ = new QWidget();
  QVBoxLayout *tools_layout = new QVBoxLayout(tools_tab_);

  // --- 色彩空间下拉框 ---
  colorspace_combo_ = new QComboBox(tools_tab_);
  colorspace_combo_->addItem("RGB");
  colorspace_combo_->addItem("HSV");
  colorspace_combo_->addItem("LAB");
  colorspace_combo_->addItem("Gray");
  colorspace_combo_->addItem("Binary");
  colorspace_combo_->setEnabled(false); // 无文档时禁用

  tools_layout->addWidget(colorspace_combo_);

  // --- 通道增益滑块容器 ---
  // 滑块由 updateChannelSliders() 动态创建和销毁
  channel_sliders_widget_ = new QWidget(tools_tab_);
  channel_sliders_layout_ = new QVBoxLayout(channel_sliders_widget_);
  channel_sliders_layout_->setContentsMargins(0, 4, 0, 0);
  tools_layout->addWidget(channel_sliders_widget_);

  // --- 选区大小微调框 ---
  // 默认禁用，有选区时由 update_selection_size_spinboxes() 启用
  selection_size_widget_ = new QWidget(tools_tab_);
  QHBoxLayout *sel_size_layout = new QHBoxLayout(selection_size_widget_);
  sel_size_layout->setContentsMargins(0, 4, 0, 4);

  QLabel *sel_label = new QLabel("选择大小:", selection_size_widget_);
  selection_width_spin_ = new QSpinBox(selection_size_widget_);
  selection_width_spin_->setRange(1, 99999);
  selection_width_spin_->setEnabled(false);
  selection_width_spin_->setToolTip("宽度");

  selection_height_spin_ = new QSpinBox(selection_size_widget_);
  selection_height_spin_->setRange(1, 99999);
  selection_height_spin_->setEnabled(false);
  selection_height_spin_->setToolTip("高度");

  sel_size_layout->addWidget(sel_label);
  sel_size_layout->addWidget(selection_width_spin_);
  sel_size_layout->addWidget(
      new QLabel("\u00d7", selection_size_widget_)); // × 号分隔符
  sel_size_layout->addWidget(selection_height_spin_);

  tools_layout->addWidget(selection_size_widget_);

  /**
   * 微调框信号连接（带 blockSignals 反馈循环防护）：
   *
   * 数据流回路：
   * 用户修改微调框 → selection_size_changed → MainFrame →
   * ImageCanvas::resize_selection() → emit selection_changed →
   * RightSidebar::update_selection_size_spinboxes() → spinbox->setValue()（通过
   * blockSignals 阻止再次发出 valueChanged）
   *
   * 如果不用 blockSignals 包裹 setValue()，就会形成无限循环。
   */
  connect(selection_width_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &RightSidebar::onSelectionWidthChanged);
  connect(selection_height_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &RightSidebar::onSelectionHeightChanged);

  // --- 直方图/均衡化按钮 ---
  histogram_btn_ = new QPushButton("灰度直方图", tools_tab_);
  equalize_hist_btn_ = new QPushButton("直方图均衡化", tools_tab_);
  clahe_btn_ = new QPushButton("局部自适应直方图均衡化", tools_tab_);
  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->addWidget(histogram_btn_, 1);
  btn_layout->addWidget(equalize_hist_btn_, 1);
  btn_layout->addWidget(clahe_btn_, 1);
  tools_layout->addLayout(btn_layout);

  // --- 卷积模糊 ---
  blur_group_ = new QGroupBox("卷积模糊", tools_tab_);
  QFormLayout *blur_layout = new QFormLayout(blur_group_);

  // ksize 微调框（只允许奇数 1/3/5/.../31）
  blur_ksize_spin_ = new QSpinBox(blur_group_);
  blur_ksize_spin_->setRange(1, 31);
  blur_ksize_spin_->setSingleStep(2); // 步长2确保只能选择奇数
  blur_ksize_spin_->setValue(3);
  blur_ksize_spin_->setToolTip("卷积核大小 (ksize)，必须为奇数");

  // anchor X 微调框（默认 -1 = 核中心）
  blur_anchor_x_spin_ = new QSpinBox(blur_group_);
  blur_anchor_x_spin_->setRange(-1, 31);
  blur_anchor_x_spin_->setValue(-1);
  blur_anchor_x_spin_->setToolTip("锚点 X 坐标，-1 表示核中心");

  // anchor Y 微调框（默认 -1 = 核中心）
  blur_anchor_y_spin_ = new QSpinBox(blur_group_);
  blur_anchor_y_spin_->setRange(-1, 31);
  blur_anchor_y_spin_->setValue(-1);
  blur_anchor_y_spin_->setToolTip("锚点 Y 坐标，-1 表示核中心");

  // borderType 下拉框
  blur_border_combo_ = new QComboBox(blur_group_);
  blur_border_combo_->addItem("BORDER_DEFAULT", cv::BORDER_DEFAULT);
  blur_border_combo_->addItem("BORDER_CONSTANT", cv::BORDER_CONSTANT);
  blur_border_combo_->addItem("BORDER_REPLICATE", cv::BORDER_REPLICATE);
  blur_border_combo_->addItem("BORDER_REFLECT", cv::BORDER_REFLECT);
  blur_border_combo_->addItem("BORDER_WRAP", cv::BORDER_WRAP);
  blur_border_combo_->addItem("BORDER_REFLECT_101", cv::BORDER_REFLECT_101);
  blur_border_combo_->setCurrentIndex(0); // BORDER_DEFAULT
  blur_border_combo_->setToolTip("边界填充类型");

  // "应用"按钮
  blur_apply_btn_ = new QPushButton("应用", blur_group_);

  blur_layout->addRow("ksize:", blur_ksize_spin_);
  blur_layout->addRow("anchor X:", blur_anchor_x_spin_);
  blur_layout->addRow("anchor Y:", blur_anchor_y_spin_);
  blur_layout->addRow("borderType:", blur_border_combo_);
  blur_layout->addRow(blur_apply_btn_);

  tools_layout->addWidget(blur_group_);

  tools_layout->addStretch();
  tabs_->addTab(tools_tab_, "工具");

  // 工具标签页信号连接
  connect(histogram_btn_, &QPushButton::clicked, this,
          &RightSidebar::onHistogramButtonClicked);
  connect(equalize_hist_btn_, &QPushButton::clicked, this,
          &RightSidebar::onEqualizeHistClicked);
  connect(clahe_btn_, &QPushButton::clicked, this,
          &RightSidebar::onCLAHEHistClicked);
  connect(blur_apply_btn_, &QPushButton::clicked, this,
          &RightSidebar::onBlurClicked);
  // 色彩空间下拉框变化 → 发出信号 + 重建通道滑块
  connect(colorspace_combo_,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &RightSidebar::color_space_changed);
  connect(colorspace_combo_,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &RightSidebar::updateChannelSliders);

  // ========================================================================
  // Tab 3: 选择历史
  // ========================================================================
  selection_list_ = new QListWidget(this);
  selection_list_->setViewMode(QListView::ListMode);
  selection_list_->setIconSize(QSize(32, 32));
  selection_list_->setSelectionMode(QAbstractItemView::SingleSelection);
  tabs_->addTab(selection_list_, "选择历史");

  connect(selection_list_, &QListWidget::itemClicked, this,
          &RightSidebar::onSelectionItemClicked);

  // ========================================================================
  // Tab 4: 剪切历史
  // ========================================================================
  // 使用 QWidget 包裹控件以统一管理布局
  QWidget *cut_history_tab = new QWidget(this);
  QVBoxLayout *cut_history_layout = new QVBoxLayout(cut_history_tab);
  cut_history_layout->setContentsMargins(4, 4, 4, 4);

  // "还原原始"按钮：点击发出 history_item_selected(-1)
  restore_original_btn_ = new QPushButton("还原原始", cut_history_tab);
  cut_history_layout->addWidget(restore_original_btn_);

  // 剪切历史列表（单选模式）
  cut_history_list_ = new QListWidget(cut_history_tab);
  cut_history_list_->setSelectionMode(QAbstractItemView::SingleSelection);
  cut_history_layout->addWidget(cut_history_list_);

  tabs_->addTab(cut_history_tab, "剪切历史");

  // 列表项点击 → 计算行索引 → 发出 history_item_selected
  connect(cut_history_list_, &QListWidget::itemClicked, this,
          &RightSidebar::onCutHistoryItemClicked);
  // "还原原始"按钮 → 发出 history_item_selected(-1)
  connect(restore_original_btn_, &QPushButton::clicked, this,
          [this]() { emit history_item_selected(-1); });

  main_layout_->addWidget(tabs_);
  setLayout(main_layout_);
}

// ============================================================================
// 公开方法
// ============================================================================

void RightSidebar::set_document(ImageDocument *doc) {
  document_ = doc;
  updateImageInfoTab();     // 刷新图片信息标签页
  selection_list_->clear(); // 清除选择历史
  selection_info_label_->setText("-");
}

void RightSidebar::set_zoom_factor(double factor) {
  zoom_factor_ = factor;
  int percent = static_cast<int>(zoom_factor_ * 100.0);
  zoom_label_->setText(QString("%1%").arg(percent));
}

void RightSidebar::update_selection_info(const QRect &image_rect) {
  current_selection_ = image_rect;
  if (image_rect.isValid()) {
    // 计算几何中心显示坐标（width/2 使用整数除法，配合 Qt6 包含性右边界）
    int cx = image_rect.x() + image_rect.width() / 2;
    int cy = image_rect.y() + image_rect.height() / 2;
    selection_info_label_->setText(QString("中心(%1,%2)  %3×%4")
                                       .arg(cx)
                                       .arg(cy)
                                       .arg(image_rect.width())
                                       .arg(image_rect.height()));
  } else {
    selection_info_label_->setText("-");
  }
}

void RightSidebar::add_selection(const QRect &image_rect) {
  if (!document_ || !document_->is_valid()) {
    return;
  }

  // 从文档图像提取缩略图
  ImageDocumentAdapter adapter(document_);
  QImage full_image = adapter.to_qimage();
  if (full_image.isNull()) {
    return;
  }

  // clamp 选区到图像边界
  QRect clamped = image_rect.intersected(
      QRect(0, 0, full_image.width(), full_image.height()));
  if (clamped.isEmpty()) {
    return;
  }

  // 创建 64x64 缩略图（保持宽高比）
  QImage thumb = full_image.copy(clamped).scaled(64, 64, Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);

  // 显示格式："(x,y) W×H"
  QString label = QString("(%1,%2) %3×%4")
                      .arg(image_rect.x())
                      .arg(image_rect.y())
                      .arg(image_rect.width())
                      .arg(image_rect.height());

  QListWidgetItem *item =
      new QListWidgetItem(QIcon(QPixmap::fromImage(thumb)), label);
  // 存储选区矩形到 UserRole，点击时可恢复
  item->setData(Qt::UserRole, image_rect);
  // 新条目插入到列表顶部
  selection_list_->insertItem(0, item);
}

/**
 * 更新选区大小微调框。
 *
 * 关键：使用 blockSignals(true/false) 包裹 setValue() 调用。
 * 如果不阻止信号，setValue() 会触发 valueChanged → onSelectionWidthChanged/
 * onSelectionHeightChanged → selection_size_changed → resize_selection()
 * → selection_changed → 再次调用更新微调框 → 形成无限反馈循环。
 */
void RightSidebar::update_selection_size_spinboxes(const QRect &image_rect) {
  if (!selection_width_spin_ || !selection_height_spin_)
    return;

  bool has_selection = image_rect.isValid();
  selection_width_spin_->setEnabled(has_selection);
  selection_height_spin_->setEnabled(has_selection);

  if (has_selection) {
    // blockSignals 防止 setValue → valueChanged → 递归更新
    selection_width_spin_->blockSignals(true);
    selection_height_spin_->blockSignals(true);
    selection_width_spin_->setValue(image_rect.width());
    selection_height_spin_->setValue(image_rect.height());
    selection_width_spin_->blockSignals(false);
    selection_height_spin_->blockSignals(false);
  }
}

/**
 * 宽度微调框变化处理。
 * 同时发送当前高度值，让接收方得到完整的 (width, height) 对。
 */
void RightSidebar::onSelectionWidthChanged(int value) {
  if (!selection_height_spin_)
    return;
  emit selection_size_changed(value, selection_height_spin_->value());
}

/**
 * 高度微调框变化处理。
 * 同时发送当前宽度值，让接收方得到完整的 (width, height) 对。
 */
void RightSidebar::onSelectionHeightChanged(int value) {
  if (!selection_width_spin_)
    return;
  emit selection_size_changed(selection_width_spin_->value(), value);
}

// ============================================================================
// 剪切历史
// ============================================================================

void RightSidebar::add_cut_history_entry(const QString &label) {
  if (!cut_history_list_)
    return;
  QListWidgetItem *item = new QListWidgetItem(label, cut_history_list_);
  // 存储列表项索引供点击时使用（注意：存储的是 addItem 之前的 count）
  item->setData(Qt::UserRole, cut_history_list_->count());
  cut_history_list_->addItem(item);
}

void RightSidebar::clear_cut_history() {
  if (cut_history_list_) {
    cut_history_list_->clear();
  }
}

/**
 * 剪切历史列表项点击处理。
 * 直接使用列表行索引（row）作为 history_item_selected 信号的参数，
 * MainFrame 通过该索引在 cut_history_ 向量中查找对应的快照。
 */
void RightSidebar::onCutHistoryItemClicked(QListWidgetItem *item) {
  if (!item || !cut_history_list_)
    return;
  int index = cut_history_list_->row(item);
  emit history_item_selected(index);
}

// ============================================================================
// 选择历史
// ============================================================================

/**
 * 选择历史列表项点击处理。
 * 从 QListWidgetItem 的 Qt::UserRole 中取出存储的 QRect，
 * 发出 selection_restore_requested 信号让 MainFrame 恢复选区。
 */
void RightSidebar::onSelectionItemClicked(QListWidgetItem *item) {
  if (!item) {
    return;
  }
  QVariant data = item->data(Qt::UserRole);
  if (data.isValid()) {
    emit selection_restore_requested(data.toRect());
  }
}

// ============================================================================
// 图片信息标签页
// ============================================================================

/**
 * 根据关联文档刷新"图片信息"标签页的内容。
 * 无文档或无效文档时，所有标签显示 "-"。
 */
void RightSidebar::updateImageInfoTab() {
  if (!document_ || !document_->is_valid()) {
    size_label_->setText("-");
    type_label_->setText("-");
    return;
  }

  const auto &data = document_->image_data();
  size_label_->setText(
      QString("%1 × %2 像素").arg(data.width()).arg(data.height()));

  // 像素格式转可读字符串
  QString format_str;
  switch (data.format()) {
  case ImageData::PixelFormat::kGray8:
    format_str = "8-bit 灰度";
    break;
  case ImageData::PixelFormat::kRGB24:
    format_str = "24-bit RGB";
    break;
  case ImageData::PixelFormat::kRGBA32:
    format_str = "32-bit RGBA";
    break;
  default:
    format_str = "未知";
    break;
  }
  // 如果有文件路径，追加文件扩展名
  if (document_->has_file_path()) {
    format_str += QString(" (%1)").arg(
        QString::fromStdString(document_->metadata().file_format));
  }
  type_label_->setText(format_str);
}

// ============================================================================
// 直方图 / 均衡化
// ============================================================================

void RightSidebar::onHistogramButtonClicked() {
  HistogramDialog dialog(document_, current_selection_, this);
  dialog.exec();
}

// ============================================================================
// QImage ↔ cv::Mat 转换辅助函数（匿名命名空间，文件内部可见）
// ============================================================================

namespace {

/**
 * @brief QImage → cv::Mat 转换。
 *
 * - 灰度图直接包装为 CV_8UC1
 * - RGB 图转换为 BGR（OpenCV 默认色彩顺序）后包装为 CV_8UC3
 *
 * 返回的 cv::Mat 是深拷贝（.clone()），不共享 QImage 的内存。
 */
cv::Mat qimageToMat(const QImage &img) {
  if (img.format() == QImage::Format_Grayscale8) {
    return cv::Mat(img.height(), img.width(), CV_8UC1,
                   const_cast<uchar *>(img.bits()),
                   static_cast<size_t>(img.bytesPerLine()))
        .clone();
  }
  // RGB888 → BGR（OpenCV 标准色彩顺序）
  QImage rgb = img.convertToFormat(QImage::Format_RGB888);
  cv::Mat mat(rgb.height(), rgb.width(), CV_8UC3,
              const_cast<uchar *>(rgb.bits()),
              static_cast<size_t>(rgb.bytesPerLine()));
  cv::Mat bgr;
  cv::cvtColor(mat.clone(), bgr, cv::COLOR_RGB2BGR);
  return bgr;
}

/**
 * @brief cv::Mat → QImage 转换。
 *
 * - 单通道 → QImage::Format_Grayscale8
 * - 多通道 → BGR → RGB → QImage::Format_RGB888
 *
 * 返回的 QImage 是深拷贝（.copy()），不共享 cv::Mat 的内存。
 */
QImage matToQImage(const cv::Mat &mat) {
  if (mat.channels() == 1) {
    return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step),
                  QImage::Format_Grayscale8)
        .copy();
  }
  // BGR → RGB
  cv::Mat rgb;
  cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
  return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                QImage::Format_RGB888)
      .copy();
}

/**
 * @brief 在可滚动的对话框中显示图像。
 *
 * 对话框大小限制为屏幕的 80%，图像通过 QScrollArea 显示。
 * 使用 Qt::WA_DeleteOnClose 属性确保关闭时自动清理内存。
 */
void showImageDialog(const QImage &image, const QString &title,
                     QWidget *parent) {
  QDialog *dialog = new QDialog(parent);
  dialog->setWindowTitle(title);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout *layout = new QVBoxLayout(dialog);

  QScrollArea *scroll = new QScrollArea(dialog);
  QLabel *label = new QLabel(scroll);
  label->setPixmap(QPixmap::fromImage(image));
  label->setAlignment(Qt::AlignCenter);
  scroll->setWidget(label);
  scroll->setWidgetResizable(false);

  layout->addWidget(scroll);

  // 对话框尺寸取图像大小和屏幕 80% 的较小值，最小 300×200
  QScreen *screen = QGuiApplication::primaryScreen();
  QSize screen_size =
      screen ? screen->availableGeometry().size() : QSize(1920, 1080);
  int max_w = screen_size.width() * 4 / 5;
  int max_h = screen_size.height() * 4 / 5;
  QSize dlg_size = image.size().boundedTo(QSize(max_w, max_h));
  dialog->resize(dlg_size.expandedTo(QSize(300, 200)));

  dialog->show();
}

} // namespace

// ============================================================================
// 直方图均衡化
// ============================================================================

/**
 * 直方图均衡化：
 * - 灰度图：直接对单通道做 equalizeHist
 * - 彩色图：转换到 HSV 色彩空间，对 V（明度）通道做 equalizeHist，
 *   再转换回 BGR。仅调整亮度，保持色调不变。
 *
 * 如果存在选区，仅对选区内图像做均衡化。
 */
void RightSidebar::onEqualizeHistClicked() {
  if (!document_ || !document_->is_valid())
    return;

  ImageDocumentAdapter adapter(document_);
  QImage src = adapter.to_qimage();
  if (src.isNull())
    return;

  // 如果有选区，仅对选区内图像做处理
  if (current_selection_.isValid()) {
    QRect clamped = current_selection_.intersected(src.rect());
    if (!clamped.isEmpty()) {
      src = src.copy(clamped);
    }
  }

  cv::Mat src_mat = qimageToMat(src);

  if (src_mat.channels() == 1) {
    cv::Mat dst;
    cv::equalizeHist(src_mat, dst);
    showImageDialog(matToQImage(dst), "直方图均衡化", this);
  } else {
    // 彩色图：HSV → 均衡化 V → 合并 → BGR
    cv::Mat hsv;
    cv::cvtColor(src_mat, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::equalizeHist(channels[2], channels[2]); // 仅均衡化 V（明度）通道
    cv::merge(channels, hsv);
    cv::Mat result;
    cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);
    showImageDialog(matToQImage(result), "直方图均衡化", this);
  }
}

// ============================================================================
// CLAHE（局部自适应直方图均衡化）
// ============================================================================

/**
 * CLAHE（对比度受限的自适应直方图均衡化）：
 *
 * 先弹出参数配置对话框：
 * - Clip Limit：对比度限制阈值（0.1 ~ 40.0，默认 3.0）
 * - Tile Grid Size：网格大小（2 ~ 64，默认 8）
 *
 * 处理逻辑同直方图均衡化：
 * - 灰度图直接做 CLAHE
 * - 彩色图在 HSV 的 V 通道上做 CLAHE
 */
void RightSidebar::onCLAHEHistClicked() {
  if (!document_ || !document_->is_valid())
    return;

  // --- CLAHE 参数配置对话框 ---
  QDialog param_dialog(this);
  param_dialog.setWindowTitle("CLAHE 参数");
  param_dialog.setMinimumWidth(280);

  QFormLayout *form = new QFormLayout(&param_dialog);

  QDoubleSpinBox *clip_spin = new QDoubleSpinBox(&param_dialog);
  clip_spin->setRange(0.1, 40.0);
  clip_spin->setValue(3.0);
  clip_spin->setDecimals(1);
  clip_spin->setSingleStep(0.5);

  QSpinBox *tile_spin = new QSpinBox(&param_dialog);
  tile_spin->setRange(2, 64);
  tile_spin->setValue(8);

  form->addRow("Clip Limit:", clip_spin);
  form->addRow("Tile Grid Size:", tile_spin);

  QDialogButtonBox *buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &param_dialog);
  connect(buttons, &QDialogButtonBox::accepted, &param_dialog,
          &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &param_dialog,
          &QDialog::reject);
  form->addRow(buttons);

  if (param_dialog.exec() != QDialog::Accepted)
    return;

  double clip_limit = clip_spin->value();
  int tile_size = tile_spin->value();

  ImageDocumentAdapter adapter(document_);
  QImage src = adapter.to_qimage();
  if (src.isNull())
    return;

  // 如果有选区，仅对选区内图像做处理
  if (current_selection_.isValid()) {
    QRect clamped = current_selection_.intersected(src.rect());
    if (!clamped.isEmpty()) {
      src = src.copy(clamped);
    }
  }

  cv::Mat src_mat = qimageToMat(src);

  // 创建 CLAHE 对象（使用配置的参数）
  auto clahe = cv::createCLAHE(clip_limit, cv::Size(tile_size, tile_size));

  if (src_mat.channels() == 1) {
    cv::Mat dst;
    clahe->apply(src_mat, dst);
    showImageDialog(matToQImage(dst), "CLAHE", this);
  } else {
    // 彩色图：HSV → CLAHE V → 合并 → BGR
    cv::Mat hsv;
    cv::cvtColor(src_mat, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    clahe->apply(channels[2], channels[2]); // 仅处理 V 通道
    cv::merge(channels, hsv);
    cv::Mat result;
    cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);
    showImageDialog(matToQImage(result), "CLAHE", this);
  }
}

// ============================================================================
// 卷积模糊（OpenCV blur）
// ============================================================================

/**
 * @brief 执行卷积模糊处理。
 *
 * 从 blur 控件组读取参数：
 * - ksize = blur_ksize_spin_->value()
 * - anchor = (blur_anchor_x_spin_->value(), blur_anchor_y_spin_->value())
 *   -1 表示使用核中心
 * - borderType = blur_border_combo_->currentData().toInt()
 *
 * 如果存在有效选区，仅对选区内图像做模糊处理。
 * 结果通过 showImageDialog() 在弹出的对话框中显示。
 *
 * @note OpenCV blur() 调用代码预留在此处，由用户自行补充。
 */
void RightSidebar::onBlurClicked() {
  if (!document_ || !document_->is_valid())
    return;

  int ksize = blur_ksize_spin_->value();
  int anchor_x = blur_anchor_x_spin_->value();
  int anchor_y = blur_anchor_y_spin_->value();
  int border_type = blur_border_combo_->currentData().toInt();

  ImageDocumentAdapter adapter(document_);
  QImage src = adapter.to_qimage();
  if (src.isNull())
    return;

  // 如果有选区，仅对选区内图像做处理
  if (current_selection_.isValid()) {
    QRect clamped = current_selection_.intersected(src.rect());
    if (!clamped.isEmpty()) {
      src = src.copy(clamped);
    }
  }

  cv::Mat src_mat = qimageToMat(src);

  // ========================================================================
  // TODO: 在此补充 OpenCV blur 代码
  //
  cv::Mat dst;
  cv::blur(src_mat, dst, cv::Size(ksize, ksize), cv::Point(anchor_x, anchor_y),
           border_type);
  showImageDialog(matToQImage(dst), "卷积模糊", this);
  // ========================================================================
}

// ============================================================================
// 通道增益滑块
// ============================================================================

/**
 * 根据色彩空间索引动态创建对应的通道增益滑块。
 *
 * 各色彩空间的通道定义：
 * - RGB (0)：R(红色) / G(绿色) / B(蓝色)，范围 0-255，默认 255
 * - HSV (1)：H(色调, 0-180) / S(饱和度, 0-255) / V(明度, 0-255)
 * - LAB (2)：L(亮度) / A(绿-红) / B(蓝-黄)，范围 0-255
 * - Gray (3)：强度，范围 0-255，默认 255
 * - Binary (4)：阈值，范围 0-255，默认 128
 *
 * 每个通道包括：
 * - QLabel：显示通道名称和当前值
 * - QSlider：水平滑块
 * - QSpinBox：微调框
 * 滑块和微调框双向同步，任一变化都会发出 channel_gains_changed 信号。
 */
void RightSidebar::updateChannelSliders(int colorSpaceIndex) {
  if (!channel_sliders_layout_) {
    return;
  }

  // 清除现有滑块（删除 widget 的同时子控件会被 Qt 自动清理）
  for (auto &cs : channel_sliders_) {
    QWidget *row = cs.label->parentWidget();
    channel_sliders_layout_->removeWidget(row);
    delete row;
  }
  channel_sliders_.clear();

  // 各色彩空间对应的通道定义：{名称, 最小值, 最大值, 默认值}
  struct ChannelDef {
    QString name;
    int min_val;
    int max_val;
    int default_val;
  };
  QVector<ChannelDef> channels;

  switch (colorSpaceIndex) {
  case 0: // RGB
    channels = {{"R (红色)", 0, 255, 255},
                {"G (绿色)", 0, 255, 255},
                {"B (蓝色)", 0, 255, 255}};
    break;
  case 1: // HSV
    channels = {{"H (色调)", 0, 180, 180},
                {"S (饱和度)", 0, 255, 255},
                {"V (明度)", 0, 255, 255}};
    break;
  case 2: // LAB
    channels = {{"L (亮度)", 0, 255, 255},
                {"A (绿-红)", 0, 255, 255},
                {"B (蓝-黄)", 0, 255, 255}};
    break;
  case 3: // Gray
    channels = {{"强度", 0, 255, 255}};
    break;
  case 4: // Binary
    channels = {{"阈值", 0, 255, 128}};
    break;
  default:
    return;
  }

  // 为每个通道创建滑块行
  for (const auto &ch : channels) {
    QWidget *row = new QWidget(channel_sliders_widget_);
    QHBoxLayout *row_layout = new QHBoxLayout(row);
    row_layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(row);
    QSlider *slider = new QSlider(Qt::Horizontal, row);
    slider->setRange(ch.min_val, ch.max_val);
    slider->setValue(ch.default_val);

    QSpinBox *spinbox = new QSpinBox(row);
    spinbox->setRange(ch.min_val, ch.max_val);
    spinbox->setValue(ch.default_val);
    spinbox->setFixedWidth(60);

    QString base_name = ch.name;
    label->setText(QString("%1: %2").arg(base_name).arg(ch.default_val));

    row_layout->addWidget(label);
    row_layout->addWidget(slider, 1);
    row_layout->addWidget(spinbox);
    channel_sliders_layout_->addWidget(row);

    // 滑块和微调框双向同步
    connect(slider, &QSlider::valueChanged, spinbox, &QSpinBox::setValue);
    connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), slider,
            &QSlider::setValue);

    // 任一控件值变化 → 收集所有值并发出 channel_gains_changed
    connect(slider, &QSlider::valueChanged, this,
            &RightSidebar::emitChannelGains);

    channel_sliders_.append({label, slider, spinbox, base_name});
  }

  // 初始化时发出默认增益值
  emitChannelGains();
}

/**
 * 收集所有通道滑块的当前值，更新标签显示，发出 channel_gains_changed 信号。
 */
void RightSidebar::emitChannelGains() {
  QVector<int> gains;
  for (auto &cs : channel_sliders_) {
    int val = cs.slider->value();
    gains.append(val);
    // 同步更新标签文本（如 "R (红色): 200"）
    cs.label->setText(QString("%1: %2").arg(cs.base_name).arg(val));
  }
  emit channel_gains_changed(gains);
}

// ============================================================================
// 色彩空间下拉框控制
// ============================================================================

void RightSidebar::enable_color_space_combo(bool enabled) {
  if (colorspace_combo_) {
    colorspace_combo_->setEnabled(enabled);
  }
}

/**
 * 静默重置色彩空间下拉框到 RGB（index = 0）。
 *
 * 使用 blockSignals(true) 防止 currentIndexChanged 信号被触发，
 * 避免不必要的色彩空间转换和通道滑块重建。
 * 手动调用 updateChannelSliders(0) 来重建默认的 RGB 通道滑块。
 */
void RightSidebar::reset_color_space_combo() {
  if (colorspace_combo_) {
    colorspace_combo_->blockSignals(true);
    colorspace_combo_->setCurrentIndex(0); // RGB
    colorspace_combo_->blockSignals(false);
    updateChannelSliders(0);
  }
}
