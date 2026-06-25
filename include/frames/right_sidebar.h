#ifndef IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
#define IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_

#include <QWidget>
#include <QVector>

class QLabel;
class QListWidgetItem;
class QPushButton;
class QComboBox;
class QSlider;
class QSpinBox;
class QTabWidget;
class QListWidget;
class QVBoxLayout;
class ImageDocument;

/**
 * @brief 右侧边栏控件，提供图像信息、工具、选择和剪切历史四个标签页。
 *
 * 标签页布局：
 * - Tab 1 "图片信息"：显示图像尺寸、像素格式、缩放比例、旋转角度、当前选区信息
 * - Tab 2 "工具"：色彩空间转换、通道增益滑块、选区大小微调框、直方图/均衡化按钮
 * - Tab 3 "选择历史"：保存过的选区缩略图列表，点击可恢复选区
 * - Tab 4 "剪切历史"：剪切操作历史记录列表，点击可恢复到对应快照；"还原原始"按钮可恢复初始图像
 *
 * 信号流：
 * - selection_size_changed → MainFrame → ImageCanvas::resize_selection()
 * - history_item_selected → MainFrame → 从剪切历史快照恢复图像
 * - color_space_changed / channel_gains_changed → MainFrame → 色彩空间转换
 * - selection_restore_requested → MainFrame → 恢复保存的选区
 */
class RightSidebar : public QWidget {
  Q_OBJECT
 public:
  explicit RightSidebar(QWidget* parent = nullptr);

  /// 设置关联的图像文档，更新信息标签页并清除选择历史
  void set_document(ImageDocument* doc);
  /// 更新缩放因子显示（百分比）
  void set_zoom_factor(double factor);
  /// 更新选区信息标签，显示中心坐标和尺寸（格式："中心(x,y) W×H"）
  void update_selection_info(const QRect& image_rect);

  /**
   * @brief 根据选区矩形更新微调框的值，不触发 selection_size_changed 信号。
   *
   * 使用 blockSignals(true/false) 包裹 setValue() 调用，防止微调框变化
   * → 触发 resize_selection → canvas 发出 selection_changed → 更新微调框
   * 的无限反馈循环。
   *
   * @param image_rect 选区矩形（图像坐标），无效时禁用微调框
   */
  void update_selection_size_spinboxes(const QRect& image_rect);

  /// 添加选区缩略图到选择历史列表
  void add_selection(const QRect& image_rect);

  /**
   * @brief 向剪切历史列表添加一个条目。
   * @param label 显示文本，格式如 "1. 1920×1080 → 320×240"
   *
   * 条目携带 Qt::UserRole 数据（当前列表项索引），点击时通过
   * history_item_selected(int) 信号传递给 MainFrame。
   */
  void add_cut_history_entry(const QString& label);

  /// 清空剪切历史列表（加载新图像时调用）
  void clear_cut_history();

  /// 启用/禁用色彩空间下拉框
  void enable_color_space_combo(bool enabled);
  /// 静默重置色彩空间下拉框到 RGB 并重建对应的通道滑块
  void reset_color_space_combo();

 private slots:
  void onHistogramButtonClicked();
  void onEqualizeHistClicked();
  void onCLAHEHistClicked();
  /// 根据色彩空间索引重建通道增益滑块（RGB/HSV/LAB/Gray/Binary）
  void updateChannelSliders(int colorSpaceIndex);

 signals:
  /// 用户点击选择历史列表项，请求恢复选区
  void selection_restore_requested(const QRect& image_rect);
  /// 用户修改微调框后发出的选区大小变更（width, height）
  void selection_size_changed(int width, int height);
  /// 色彩空间下拉框选择变化（0=RGB, 1=HSV, 2=LAB, 3=Gray, 4=Binary）
  void color_space_changed(int index);
  /// 通道增益滑块变化，值含义取决于当前色彩空间
  void channel_gains_changed(const QVector<int>& gains);
  /**
   * @brief 用户点击剪切历史列表项或"还原原始"按钮。
   * @param index 列表行索引（0-based），-1 表示请求恢复原始图像
   */
  void history_item_selected(int index);

 private:
  void buildUi();
  void updateImageInfoTab();
  void onSelectionItemClicked(QListWidgetItem* item);
  void onSelectionWidthChanged(int value);
  void onSelectionHeightChanged(int value);
  void onCutHistoryItemClicked(QListWidgetItem* item);
  /// 收集所有通道滑块的当前值并通过 channel_gains_changed 信号发出
  void emitChannelGains();

  /// 单个通道滑块组合（标签 + 滑块 + 微调框）
  struct ChannelSlider {
    QLabel* label;       ///< 显示通道名称和当前值
    QSlider* slider;     ///< 滑块控件
    QSpinBox* spinbox;   ///< 微调框控件
    QString base_name;   ///< 通道基础名称（如 "R (红色)"）
  };

  // --- 标签页容器 ---
  QTabWidget* tabs_;

  // --- Tab 1: 图片信息 ---
  QWidget* info_tab_;
  QLabel* size_label_;              ///< "1920 × 1080 像素"
  QLabel* type_label_;              ///< "8-bit 灰度 (png)"
  QLabel* zoom_label_;              ///< "100%"
  QLabel* rotation_label_;          ///< "0°"
  QLabel* selection_info_label_;    ///< "中心(x,y) W×H" 或 "-"

  // --- Tab 2: 工具 ---
  QWidget* tools_tab_;
  QPushButton* histogram_btn_;      ///< "灰度直方图"
  QPushButton* equalize_hist_btn_;  ///< "直方图均衡化"
  QPushButton* clahe_btn_;           ///< "局部自适应直方图均衡化"
  QComboBox* colorspace_combo_;     ///< 色彩空间选择（RGB/HSV/LAB/Gray/Binary）
  QWidget* channel_sliders_widget_; ///< 通道增益滑块的容器 widget
  QVBoxLayout* channel_sliders_layout_; ///< 通道滑块容器的布局
  QVector<ChannelSlider> channel_sliders_; ///< 当前显示的通道滑块列表

  /**
   * @brief 选区大小微调框，位于工具标签页。
   *
   * 微调框默认禁用，selection_changed 信号触发 update_selection_size_spinboxes()
   * 后才启用和设值。通过 blockSignals() 防止反馈循环。
   */
  QSpinBox* selection_width_spin_;
  QSpinBox* selection_height_spin_;
  QWidget* selection_size_widget_;  ///< 微调框行的容器 widget

  // --- Tab 3: 选择历史 ---
  QListWidget* selection_list_;     ///< 保存的选区缩略图列表

  // --- Tab 4: 剪切历史 ---
  QListWidget* cut_history_list_;   ///< 剪切操作记录列表
  QPushButton* restore_original_btn_; ///< "还原原始"按钮，点击发出 history_item_selected(-1)

  // --- 状态 ---
  ImageDocument* document_;         ///< 关联的图像文档（不拥有所有权）
  double zoom_factor_;              ///< 当前缩放因子
  double rotation_;                 ///< 当前旋转角度（预留）
  QRect current_selection_;         ///< 当前选区矩形缓存

  QVBoxLayout* main_layout_;        ///< 主布局（包含标签页控件）
};

#endif  // IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
