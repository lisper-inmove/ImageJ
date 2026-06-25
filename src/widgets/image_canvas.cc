#include "widgets/image_canvas.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QMenu>

#include <algorithm>
#include <cmath>

#include "core/image_document.h"
#include "core/image_document_adapter.h"

// ============================================================================
// 构造 / 析构
// ============================================================================

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent),
      document_(nullptr),
      zoom_factor_(1.0),
      view_offset_(0, 0),
      background_style_(BackgroundStyle::kSolidColor),
      background_color_(QColor(0x2D, 0x2D, 0x2D)),
      is_selecting_(false),
      selection_mode_(false),
      selection_center_(),
      selection_rect_(),
      is_moving_selection_(false),
      move_origin_(),
      move_press_point_() {
  setMinimumSize(100, 100);
  setMouseTracking(true);   // 需要在非按下时也接收鼠标移动事件（用于光标反馈）
  setFocusPolicy(Qt::StrongFocus);  // 接收键盘事件（Ctrl+1, Escape）
}

ImageCanvas::~ImageCanvas() = default;

// ============================================================================
// 文档管理
// ============================================================================

void ImageCanvas::set_document(ImageDocument *document) {
  document_ = document;
  selection_rect_ = QRect();  // 切换文档时清除选区
  view_offset_ = QPoint(0, 0); // 重置视图偏移
  emit document_changed(document_);
  update();
}

ImageDocument *ImageCanvas::document() const {
  return document_;
}

// ============================================================================
// 缩放控制
// ============================================================================

void ImageCanvas::set_zoom_factor(double factor) {
  zoom_factor_ = factor;
  clampViewOffset();  // 缩放后重新 clamp 偏移量，防止图像完全滚出视图
  emit view_changed();
  update();
}

double ImageCanvas::zoom_factor() const {
  return zoom_factor_;
}

void ImageCanvas::fit_to_window() {
  if (!document_ || !document_->is_valid()) {
    return;
  }
  const auto &image_data = document_->image_data();
  // 计算水平和垂直方向分别的缩放比例，取较小值确保完整显示
  double scale_x = static_cast<double>(width()) / image_data.width();
  double scale_y = static_cast<double>(height()) / image_data.height();
  set_zoom_factor(std::min(scale_x, scale_y));
}

void ImageCanvas::reset_zoom() {
  set_zoom_factor(1.0);
}

// ============================================================================
// 视图偏移（平移）
// ============================================================================

QPoint ImageCanvas::view_offset() const {
  return view_offset_;
}

void ImageCanvas::set_view_offset(const QPoint &offset) {
  view_offset_ = offset;
  clampViewOffset();
  emit view_changed();
  update();
}

QRect ImageCanvas::visible_image_rect() const {
  QPoint top_left = canvas_to_image(QPoint(0, 0));
  QPoint bottom_right = canvas_to_image(QPoint(width(), height()));
  return QRect(top_left, bottom_right);
}

// ============================================================================
// 背景样式
// ============================================================================

void ImageCanvas::set_background_style(BackgroundStyle style) {
  background_style_ = style;
  update();
}

ImageCanvas::BackgroundStyle ImageCanvas::background_style() const {
  return background_style_;
}

void ImageCanvas::set_background_color(const QColor &color) {
  background_color_ = color;
  update();
}

QColor ImageCanvas::background_color() const {
  return background_color_;
}

// ============================================================================
// 显示控制
// ============================================================================

void ImageCanvas::update_display() {
  update();  // Qt 异步重绘（合并多次调用）
}

void ImageCanvas::force_redraw() {
  repaint();  // Qt 同步重绘（立即执行）
}

// ============================================================================
// 绘制
// ============================================================================

void ImageCanvas::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  QRect rect = event->rect();

  // --- 1. 绘制背景 ---
  switch (background_style_) {
    case BackgroundStyle::kCheckerboard: {
      // 棋盘格背景：交替绘制亮/暗方块，用于可视化透明区域
      const int kSquareSize = 16;
      const QColor kLight(0xCC, 0xCC, 0xCC);
      const QColor kDark(0x99, 0x99, 0x99);

      int start_col = rect.left() / kSquareSize;
      int end_col = (rect.right() + kSquareSize - 1) / kSquareSize;
      int start_row = rect.top() / kSquareSize;
      int end_row = (rect.bottom() + kSquareSize - 1) / kSquareSize;

      for (int row = start_row; row <= end_row; ++row) {
        for (int col = start_col; col <= end_col; ++col) {
          QColor color = ((row + col) % 2 == 0) ? kLight : kDark;
          painter.fillRect(col * kSquareSize, row * kSquareSize,
                           kSquareSize, kSquareSize, color);
        }
      }
      break;
    }
    case BackgroundStyle::kSolidColor:
      painter.fillRect(rect, background_color_);
      break;
    case BackgroundStyle::kTransparent:
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.fillRect(rect, Qt::transparent);
      break;
  }

  // --- 2. 绘制文档图像 ---
  if (document_ && document_->is_valid()) {
    ImageDocumentAdapter adapter(document_);
    QImage image = adapter.to_qimage();
    if (!image.isNull()) {
      painter.save();
      painter.translate(view_offset_);  // 应用视图偏移
      painter.scale(zoom_factor_, zoom_factor_); // 应用缩放
      painter.drawImage(0, 0, image);
      painter.restore();
    }
  }

  // --- 3. 绘制选区（红色边框 + 中心十字线） ---
  if (selection_rect_.isValid()) {
    QRect canvas_rect = image_to_canvas(selection_rect_);
    if (!canvas_rect.isEmpty()) {
      painter.save();

      // 红色选区边框（2px 宽）
      QPen rect_pen(QColor(255, 0, 0));
      rect_pen.setWidth(2);
      painter.setPen(rect_pen);
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(canvas_rect.adjusted(0, 0, -1, -1));

      // 红色中心十字线（1px 宽）
      QPoint center = canvas_rect.center();
      QPen cross_pen(QColor(255, 0, 0));
      cross_pen.setWidth(1);
      painter.setPen(cross_pen);
      // 水平线
      painter.drawLine(QPoint(canvas_rect.left(), center.y()),
                       QPoint(canvas_rect.right() - 1, center.y()));
      // 垂直线
      painter.drawLine(QPoint(center.x(), canvas_rect.top()),
                       QPoint(center.x(), canvas_rect.bottom() - 1));
      painter.restore();
    }
  }
}

// ============================================================================
// 事件处理
// ============================================================================

void ImageCanvas::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
}

/**
 * 鼠标按下事件处理：
 *
 * 在选区模式下左键按下时：
 * - Shift 键按住 + 光标在已有选区内 → 进入选区移动模式
 *   - 设置 is_moving_selection_ = true
 *   - 记录 move_origin_（选区当前左上角）和 move_press_point_（鼠标按下点）
 *   - 光标变为 ClosedHandCursor（抓取中）
 * - 否则 → 进入选区创建模式
 *   - 从当前点开始以中心扩散方式创建选区（selection_center_ 固定）
 *
 * 非选区模式下任意鼠标按钮 → 发出 image_clicked 信号
 */
void ImageCanvas::mousePressEvent(QMouseEvent *event) {
  if (selection_mode_ && event->button() == Qt::LeftButton) {
    QPoint image_pos = canvas_to_image(event->pos());
    // Shift 按住 + 光标在选区内 → 移动选区
    if ((event->modifiers() & Qt::ShiftModifier) && is_over_selection(image_pos)) {
      is_moving_selection_ = true;
      move_origin_ = selection_rect_.topLeft(); // 记录起始位置
      move_press_point_ = image_pos;            // 记录按下点（计算 delta 用）
      setCursor(Qt::ClosedHandCursor);
    } else {
      // 正常选区创建（以按下的点为中心向外扩展）
      is_selecting_ = true;
      selection_center_ = image_pos;
      selection_rect_ = QRect(selection_center_, QSize(1, 1));
    }
  } else {
    QPoint image_pos = canvas_to_image(event->pos());
    emit image_clicked(image_pos, event->button());
  }
  QWidget::mousePressEvent(event);
}

/**
 * 鼠标移动事件处理（三种模式）：
 *
 * 1. 选区移动模式 (is_moving_selection_ = true)：
 *    - 计算鼠标 delta = current - move_press_point_
 *    - 选区新位置 = move_origin_ + delta
 *    - clamp 到图像边界（保证选区不越界）
 *    - 发出 selection_changed 信号（触发侧边栏微调框更新）
 *
 * 2. 选区创建模式 (is_selecting_ = true)：
 *    - 以 selection_center_ 为中心，计算当前点与中心的曼哈顿距离
 *    - 创建以中心对称的矩形（dx*2+1 × dy*2+1）
 *    - 发出 selection_changed 信号
 *
 * 3. 普通模式：
 *    - 发出 mouse_over_image 信号（状态栏像素信息更新）
 *    - 光标反馈：
 *      - Shift + 光标在选区内 → OpenHandCursor（可移动提示）
 *      - 选区模式 → CrossCursor（十字线）
 *      - 其他 → ArrowCursor
 */
void ImageCanvas::mouseMoveEvent(QMouseEvent *event) {
  if (is_moving_selection_) {
    // --- 选区移动 ---
    QPoint current = canvas_to_image(event->pos());
    QPoint delta = current - move_press_point_;
    QPoint new_top_left = move_origin_ + delta;

    // clamp 到图像边界，确保选区不会移出图像
    if (document_ && document_->is_valid()) {
      const auto& data = document_->image_data();
      int max_x = data.width() - selection_rect_.width();
      int max_y = data.height() - selection_rect_.height();
      new_top_left.setX(std::clamp(new_top_left.x(), 0, max_x));
      new_top_left.setY(std::clamp(new_top_left.y(), 0, max_y));
    }

    selection_rect_.moveTopLeft(new_top_left);
    selection_center_ = selection_rect_.center(); // 保持中心点同步
    emit selection_changed(selection_rect_);
    update();
  } else if (is_selecting_) {
    // --- 选区创建（中心扩散） ---
    QPoint current = canvas_to_image(event->pos());
    int dx = std::abs(current.x() - selection_center_.x());
    int dy = std::abs(current.y() - selection_center_.y());
    // Qt6 中 QRect 右/下边界是包含性的，所以宽高 = dx*2 + 1
    selection_rect_ = QRect(selection_center_.x() - dx,
                            selection_center_.y() - dy,
                            dx * 2 + 1, dy * 2 + 1);
    emit selection_changed(selection_rect_);
    update();
  } else {
    // --- 普通模式：光标反馈 ---
    QPoint image_pos = canvas_to_image(event->pos());
    emit mouse_over_image(image_pos);

    // 根据当前状态给出视觉提示
    if ((event->modifiers() & Qt::ShiftModifier) && is_over_selection(image_pos)) {
      setCursor(Qt::OpenHandCursor);   // Shift+框选上 → 可以拖拽移动
    } else if (selection_mode_) {
      setCursor(Qt::CrossCursor);      // 选区模式 → 十字线
    } else {
      setCursor(Qt::ArrowCursor);      // 默认 → 箭头
    }
  }
  QWidget::mouseMoveEvent(event);
}

/**
 * 鼠标释放事件处理：
 *
 * - 选区移动结束：重置移动状态和光标
 * - 选区创建结束：clamp 选区到图像边界，如有效则发出 selection_completed 信号
 *   （该信号触发侧边栏添加选区到选择历史）
 */
void ImageCanvas::mouseReleaseEvent(QMouseEvent *event) {
  if (is_moving_selection_) {
    is_moving_selection_ = false;
    move_origin_ = QPoint();
    move_press_point_ = QPoint();
    setCursor(selection_mode_ ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
  } else if (is_selecting_) {
    is_selecting_ = false;
    // clamp 选区到图像边界
    if (document_ && document_->is_valid()) {
      const auto &image_data = document_->image_data();
      QRect image_bounds(0, 0, image_data.width(), image_data.height());
      selection_rect_ = selection_rect_.intersected(image_bounds);
    }
    if (selection_rect_.isValid()) {
      emit selection_completed(selection_rect_);
    }
    update();
  }
  QWidget::mouseReleaseEvent(event);
}

/**
 * 键盘事件处理：
 *
 * - Ctrl+1：切换选区模式的开关
 * - Escape：
 *   1. 清除 is_moving_selection_ 状态
 *   2. 退出选区模式
 *   3. 如果正在选择或已有选区，清除选区并发出 selection_changed 信号
 *     （使侧边栏的微调框和选区信息标签恢复默认状态）
 */
void ImageCanvas::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_1 && event->modifiers() == Qt::ControlModifier) {
    selection_mode_ = !selection_mode_;
    setCursor(selection_mode_ ? Qt::CrossCursor : Qt::ArrowCursor);
    return;
  }
  if (event->key() == Qt::Key_Escape) {
    is_moving_selection_ = false; // 清除移动状态
    selection_mode_ = false;
    setCursor(Qt::ArrowCursor);
    if (is_selecting_ || selection_rect_.isValid()) {
      is_selecting_ = false;
      selection_rect_ = QRect();  // 无效矩形表示无选区
      emit selection_changed(selection_rect_); // 通知侧边栏更新
      update();
    }
  }
  QWidget::keyPressEvent(event);
}

/**
 * 鼠标滚轮事件处理（三种模式）：
 *
 * - Ctrl + 滚轮：缩放（以光标下的图像像素为锚点）
 *   使用浮点计算避免 canvas_to_image/image_to_canvas 来回转换中的截断误差累积。
 *   缩放范围：0.01x ~ 100x。
 *
 * - Shift + 滚轮：水平滚动
 *
 * - 无修饰键：垂直滚动
 */
void ImageCanvas::wheelEvent(QWheelEvent *event) {
  const QPoint delta = event->angleDelta();

  if (event->modifiers() & Qt::ControlModifier) {
    // --- 锚点缩放 ---
    // 缩放前后保持光标下的图像像素不变，使用浮点避免截断误差
    QPointF canvas_pos = event->position();
    double old_zoom = zoom_factor_;

    QPointF image_pos(
        (canvas_pos.x() - view_offset_.x()) / old_zoom,
        (canvas_pos.y() - view_offset_.y()) / old_zoom);

    double new_zoom = zoom_factor_;
    if (delta.y() > 0) {
      new_zoom = zoom_factor_ * 1.1;
    } else {
      new_zoom = zoom_factor_ / 1.1;
    }
    new_zoom = std::clamp(new_zoom, 0.01, 100.0);
    zoom_factor_ = new_zoom;

    // 调整 view_offset_ 使锚点像素保持在原位
    QPointF new_canvas(
        image_pos.x() * new_zoom + view_offset_.x(),
        image_pos.y() * new_zoom + view_offset_.y());

    view_offset_.rx() += static_cast<int>(
        std::floor(canvas_pos.x() - new_canvas.x()));
    view_offset_.ry() += static_cast<int>(
        std::floor(canvas_pos.y() - new_canvas.y()));
  } else if (event->modifiers() & Qt::ShiftModifier) {
    // --- 水平滚动 ---
    int dx = delta.x() != 0 ? delta.x() : delta.y();
    view_offset_.rx() -= dx;
    clampViewOffset();
  } else {
    // --- 垂直滚动 ---
    view_offset_.ry() += delta.y();
    clampViewOffset();
  }

  emit view_changed();
  update();
  QWidget::wheelEvent(event);
}

/**
 * 右键上下文菜单：
 * 仅当存在有效选区时显示，提供两个操作：
 * - "另存为"：发出 save_selection_requested 信号
 * - "剪切"：发出 cut_selection_requested 信号
 */
void ImageCanvas::contextMenuEvent(QContextMenuEvent* event) {
  if (!selection_rect_.isValid() || !document_ || !document_->is_valid()) {
    event->ignore();
    return;
  }

  QMenu menu(this);
  QAction* save_action = menu.addAction("另存为");
  QAction* cut_action = menu.addAction("剪切");

  QAction* chosen = menu.exec(event->globalPos());
  if (chosen == save_action) {
    emit save_selection_requested();
  } else if (chosen == cut_action) {
    emit cut_selection_requested();
  }

  event->accept();
}

// ============================================================================
// 坐标转换
// ============================================================================

QPoint ImageCanvas::image_to_canvas(const QPoint &image_point) const {
  return QPoint(
      static_cast<int>(image_point.x() * zoom_factor_) + view_offset_.x(),
      static_cast<int>(image_point.y() * zoom_factor_) + view_offset_.y());
}

QPoint ImageCanvas::canvas_to_image(const QPoint &canvas_point) const {
  return QPoint(
      static_cast<int>((canvas_point.x() - view_offset_.x()) / zoom_factor_),
      static_cast<int>((canvas_point.y() - view_offset_.y()) / zoom_factor_));
}

/**
 * 图像矩形 → 画布矩形：
 * 将左上角和右下角（x+width, y+height，即 exclusive 右下角）分别转换，
 * 得到画布坐标下的矩形。避免 width/height 直接乘以 zoom_factor_ 时
 * 因四舍五入导致的尺寸不一致问题。
 */
QRect ImageCanvas::image_to_canvas(const QRect &image_rect) const {
  QPoint top_left = image_to_canvas(image_rect.topLeft());
  QPoint bottom_right_exclusive = image_to_canvas(
      QPoint(image_rect.x() + image_rect.width(),
             image_rect.y() + image_rect.height()));
  return QRect(top_left.x(), top_left.y(),
               bottom_right_exclusive.x() - top_left.x(),
               bottom_right_exclusive.y() - top_left.y());
}

/**
 * 画布矩形 → 图像矩形：
 * 同上，将 exclusive 右下角分别转换后再计算宽高。
 */
QRect ImageCanvas::canvas_to_image(const QRect &canvas_rect) const {
  QPoint top_left = canvas_to_image(canvas_rect.topLeft());
  QPoint bottom_right_exclusive = canvas_to_image(
      QPoint(canvas_rect.x() + canvas_rect.width(),
             canvas_rect.y() + canvas_rect.height()));
  return QRect(top_left.x(), top_left.y(),
               bottom_right_exclusive.x() - top_left.x(),
               bottom_right_exclusive.y() - top_left.y());
}

// ============================================================================
// 选区管理
// ============================================================================

QRect ImageCanvas::selection() const {
  return selection_rect_;
}

void ImageCanvas::set_selection(const QRect &image_rect) {
  is_selecting_ = false; // 直接设置选区时取消任何进行中的选择
  selection_rect_ = image_rect;
  update();
}

void ImageCanvas::clear_selection() {
  selection_rect_ = QRect(); // 无效矩形表示无选区
  emit selection_changed(selection_rect_);
  update();
}

/**
 * 以选区中心为锚点调整选区大小。
 *
 * 关键细节：Qt6 中 QRect 使用包含性右/下边界，即
 * QRect(x, y, w, h).right() == x + w - 1。
 *
 * 因此 center() 在偶数宽高的矩形上会偏向"左上"：
 * QRect(45, 45, 10, 10).center() == (49, 49)
 *
 * 为确保 resize 后中心点不变，半尺寸使用 (width - 1) / 2：
 * new_rect = QRect(center.x - (w-1)/2, center.y - (h-1)/2, w, h)
 *
 * 这样 new_rect.center() 仍然等于原来的 center。
 */
void ImageCanvas::resize_selection(int new_width, int new_height) {
  if (!selection_rect_.isValid() || !document_ || !document_->is_valid()) return;
  if (new_width < 1 || new_height < 1) return;

  QPoint center = selection_rect_.center();
  // 使用 (n-1)/2 而非 n/2 来处理 Qt6 QRect 的包含性右/下边界
  int half_w = (new_width - 1) / 2;
  int half_h = (new_height - 1) / 2;

  QRect new_rect(center.x() - half_w, center.y() - half_h,
                 new_width, new_height);

  // clamp 到图像边界
  const auto& data = document_->image_data();
  QRect image_bounds(0, 0, data.width(), data.height());
  new_rect = new_rect.intersected(image_bounds);
  if (new_rect.isEmpty()) return; // 完全越界，忽略

  selection_rect_ = new_rect;
  selection_center_ = selection_rect_.center();
  emit selection_changed(selection_rect_);
  update();
}

/**
 * 判断图像坐标点是否在选区内。
 * 用于 Shift+拖动检测：鼠标悬停在选区上且按下 Shift 时，光标变为手型并可拖拽移动选区。
 */
bool ImageCanvas::is_over_selection(const QPoint& image_point) const {
  if (!selection_rect_.isValid()) return false;
  return selection_rect_.contains(image_point);
}

void ImageCanvas::on_document_modified() {
  update();
}

// ============================================================================
// 视图偏移 clamp
// ============================================================================

/**
 * 将视图偏移量 clamp 到有效范围。
 *
 * 逻辑：如果缩放后的图像比控件小，居中显示（offset 在负值和 0 之间 clamp）。
 * 如果比控件大，允许拖动但不能让图像完全滚出视图。
 */
void ImageCanvas::clampViewOffset() {
  if (!document_ || !document_->is_valid()) {
    return;
  }
  const auto &image_data = document_->image_data();
  int scaled_w = static_cast<int>(image_data.width() * zoom_factor_);
  int scaled_h = static_cast<int>(image_data.height() * zoom_factor_);

  int max_x_offset = -(scaled_w - width());
  int max_y_offset = -(scaled_h - height());

  // 图像缩放后比控件小 → 居中显示
  if (max_x_offset > 0) {
    view_offset_.rx() = std::clamp(view_offset_.x(), 0, max_x_offset);
  } else {
    view_offset_.rx() = std::clamp(view_offset_.x(), max_x_offset, 0);
  }
  if (max_y_offset > 0) {
    view_offset_.ry() = std::clamp(view_offset_.y(), 0, max_y_offset);
  } else {
    view_offset_.ry() = std::clamp(view_offset_.y(), max_y_offset, 0);
  }
}
