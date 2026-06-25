#ifndef IMAGEJ_WIDGETS_IMAGE_CANVAS_H_
#define IMAGEJ_WIDGETS_IMAGE_CANVAS_H_

#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QColor>

class ImageDocument;

/**
 * @brief 图像显示画布控件，负责渲染图像、处理交互和选区管理。
 *
 * ImageCanvas 是整个应用的核心显示组件，提供：
 * - 图像渲染，支持棋盘格/纯色/透明背景
 * - 缩放（鼠标滚轮）和平移（滚轮滚动）
 * - 以中心点为基准的矩形选区：Ctrl+1 进入选择模式，拖动创建选区
 * - Shift+拖动移动已有选区（保持选区大小不变）
 * - 右键上下文菜单（另存为/剪切选中区域）
 *
 * 坐标系统：
 * - 图像坐标（image coordinates）：以图像左上角为原点的像素坐标
 * - 画布坐标（canvas coordinates）：控件窗口坐标，受 view_offset_ 和 zoom_factor_ 影响
 * - image_to_canvas() / canvas_to_image() 负责两者转换
 *
 * 注意：Qt6 中 QRect 的 right()/bottom() 是包含性边界（x2 = x1 + width - 1），
 * 因此 center() 在偶数宽高的矩形上会偏向"左上"方向。resize_selection()
 * 使用 `(new_width - 1) / 2` 的半尺寸计算来确保中心点保持不变。
 */
class ImageCanvas : public QWidget {
  Q_OBJECT

 public:
  /// 背景样式枚举
  enum class BackgroundStyle {
    kCheckerboard,  ///< 棋盘格背景（常用于透明图像）
    kSolidColor,     ///< 纯色背景
    kTransparent    ///< 透明背景
  };

  explicit ImageCanvas(QWidget* parent = nullptr);
  ~ImageCanvas() override;

  // --- 文档管理 ---

  /// 设置要显示的图像文档（不获取所有权，由调用者管理生命周期）
  void set_document(ImageDocument* document);
  /// 返回当前显示的图像文档指针，无文档时返回 nullptr
  ImageDocument* document() const;

  // --- 缩放控制 ---

  /// 设置缩放因子（1.0 = 100%），自动 clamp 到有效范围
  void set_zoom_factor(double factor);
  /// 返回当前缩放因子
  double zoom_factor() const;
  /// 缩放图像以适应窗口大小
  void fit_to_window();
  /// 重置缩放因子到 100%
  void reset_zoom();

  // --- 视图偏移（平移） ---

  /// 返回当前视图偏移量（图像在画布上的像素偏移）
  QPoint view_offset() const;
  /// 设置视图偏移量并 clamp 到有效范围
  void set_view_offset(const QPoint& offset);
  /// 返回当前可见区域的图像坐标范围
  QRect visible_image_rect() const;

  // --- 背景样式 ---

  void set_background_style(BackgroundStyle style);
  BackgroundStyle background_style() const;
  void set_background_color(const QColor& color);
  QColor background_color() const;

  // --- 选区管理 ---

  /// 返回当前选区（图像坐标），无选区时返回无效 QRect
  QRect selection() const;
  /// 直接设置选区矩形（图像坐标），结束任何进行中的选择操作
  void set_selection(const QRect& image_rect);
  /// 清除选区并通过 selection_changed 信号通知外围组件
  void clear_selection();

  // --- 显示控制 ---

  /// 请求重绘（异步，合并多次调用）
  void update_display();
  /// 立即强制重绘（同步）
  void force_redraw();

  // --- Qt 事件重写 ---

  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

  // --- 坐标转换 ---

  /// 图像坐标 → 画布坐标（单点）
  QPoint image_to_canvas(const QPoint& image_point) const;
  /// 画布坐标 → 图像坐标（单点）
  QPoint canvas_to_image(const QPoint& canvas_point) const;
  /// 图像坐标 → 画布坐标（矩形）
  QRect image_to_canvas(const QRect& image_rect) const;
  /// 画布坐标 → 图像坐标（矩形）
  QRect canvas_to_image(const QRect& canvas_rect) const;

  /**
   * @brief 判断图像坐标点是否在选区内。
   * @param image_point 图像坐标点
   * @return true 当存在有效选区且该点在选区内
   *
   * 用于 Shift+拖动检测：当鼠标悬停在已有选区上且按下 Shift 键时，
   * 光标变为 OpenHandCursor 并可以拖拽移动选区。
   */
  bool is_over_selection(const QPoint& image_point) const;

 signals:
  /// 文档切换时发出，new_document 可能为 nullptr
  void document_changed(ImageDocument* new_document);
  /// 视图参数（缩放/偏移）变化时发出
  void view_changed();
  /// 鼠标在图像上移动时发出，传递图像坐标位置
  void mouse_over_image(const QPoint& image_position);
  /// 鼠标点击图像时发出，传递图像坐标和按下的按钮
  void image_clicked(const QPoint& image_position, Qt::MouseButton button);
  /// 选区变化时发出（拖动创建、移动、缩放时持续发出）
  void selection_changed(const QRect& image_rect);
  /// 选区创建完成时发出（鼠标释放）
  void selection_completed(const QRect& image_rect);
  /// 右键菜单"另存为"被点击
  void save_selection_requested();
  /// 右键菜单"剪切"被点击
  void cut_selection_requested();

 public slots:
  /// 文档内容发生变化时更新显示
  void on_document_modified();

  /**
   * @brief 以当前选区的中心点为基准调整选区大小。
   * @param new_width 新的选区宽度（像素，>= 1）
   * @param new_height 新的选区高度（像素，>= 1）
   *
   * 保持选区中心点不变，调整宽高后 clamp 到图像边界。
   * 如果调整后选区变为空（完全越界），操作被忽略。
   * 使用 `(new_width - 1) / 2` 计算半尺寸以正确处理 Qt 偶数宽高 QRect 的中心偏差。
   */
  void resize_selection(int new_width, int new_height);

 private:
  ImageDocument* document_;       ///< 当前显示的图像文档（不拥有所有权）
  double zoom_factor_;            ///< 缩放因子（1.0 = 100%）
  QPoint view_offset_;            ///< 视图偏移（图像在画布上的像素偏移）
  BackgroundStyle background_style_;  ///< 当前背景样式
  QColor background_color_;       ///< 纯色背景的颜色
  bool is_selecting_;             ///< 正在拖动创建选区
  bool selection_mode_;           ///< 选择模式激活（Ctrl+1 切换）
  QPoint selection_center_;       ///< 选区中心点（图像坐标），拖动时固定
  QRect selection_rect_;          ///< 当前选区矩形（图像坐标），无效时表示无选区
  bool is_moving_selection_;      ///< 正在 Shift+拖动移动选区
  QPoint move_origin_;            ///< 移动开始时选区的原始左上角（图像坐标）
  QPoint move_press_point_;       ///< 移动开始时鼠标按下的图像坐标点

  /// 将视图偏移量 clamp 到有效范围，防止图像完全滚出视图
  void clampViewOffset();
};

#endif  // IMAGEJ_WIDGETS_IMAGE_CANVAS_H_
