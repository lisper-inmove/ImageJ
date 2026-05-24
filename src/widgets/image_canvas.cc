#include "widgets/image_canvas.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

#include "core/image_document.h"
#include "core/image_document_adapter.h"

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent),
      document_(nullptr),
      zoom_factor_(1.0),
      view_offset_(0, 0),
      background_style_(BackgroundStyle::kSolidColor),
      background_color_(QColor(0x2D, 0x2D, 0x2D)),
      is_selecting_(false),
      selection_rect_() {
  setMinimumSize(100, 100);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
}

ImageCanvas::~ImageCanvas() = default;

void ImageCanvas::set_document(ImageDocument *document) {
  document_ = document;
  view_offset_ = QPoint(0, 0);
  emit document_changed(document_);
  update();
}

ImageDocument *ImageCanvas::document() const {
  return document_;
}

void ImageCanvas::set_zoom_factor(double factor) {
  zoom_factor_ = factor;
  clampViewOffset();
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
  double scale_x = static_cast<double>(width()) / image_data.width();
  double scale_y = static_cast<double>(height()) / image_data.height();
  set_zoom_factor(std::min(scale_x, scale_y));
}

void ImageCanvas::reset_zoom() {
  set_zoom_factor(1.0);
}

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

void ImageCanvas::update_display() {
  update();
}

void ImageCanvas::force_redraw() {
  repaint();
}

void ImageCanvas::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  QRect rect = event->rect();

  // Draw background
  switch (background_style_) {
    case BackgroundStyle::kCheckerboard: {
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

  // Draw document image
  if (document_ && document_->is_valid()) {
    ImageDocumentAdapter adapter(document_);
    QImage image = adapter.to_qimage();
    if (!image.isNull()) {
      painter.save();
      painter.translate(view_offset_);
      painter.scale(zoom_factor_, zoom_factor_);
      painter.drawImage(0, 0, image);
      painter.restore();
    }
  }

  if (selection_rect_.isValid()) {
    QRect canvas_rect = image_to_canvas(selection_rect_);
    if (!canvas_rect.isEmpty()) {
      painter.save();
      QPen rect_pen(QColor(255, 0, 0));
      rect_pen.setWidth(2);
      painter.setPen(rect_pen);
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(canvas_rect.adjusted(0, 0, -1, -1));

      QPoint center = canvas_rect.center();
      QPen cross_pen(QColor(255, 0, 0));
      cross_pen.setWidth(1);
      painter.setPen(cross_pen);
      painter.drawLine(QPoint(canvas_rect.left(), center.y()),
                       QPoint(canvas_rect.right() - 1, center.y()));
      painter.drawLine(QPoint(center.x(), canvas_rect.top()),
                       QPoint(center.x(), canvas_rect.bottom() - 1));
      painter.restore();
    }
  }
}

void ImageCanvas::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
}

void ImageCanvas::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::RightButton) {
    is_selecting_ = true;
    QPoint image_pos = canvas_to_image(event->pos());
    selection_rect_ = QRect(image_pos, image_pos);
  } else {
    QPoint image_pos = canvas_to_image(event->pos());
    emit image_clicked(image_pos, event->button());
  }
  QWidget::mousePressEvent(event);
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event) {
  if (is_selecting_) {
    QPoint current_image = canvas_to_image(event->pos());
    selection_rect_.setBottomRight(current_image);
    emit selection_changed(selection_rect_.normalized());
    update();
  } else {
    QPoint image_pos = canvas_to_image(event->pos());
    emit mouse_over_image(image_pos);
  }
  QWidget::mouseMoveEvent(event);
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::RightButton && is_selecting_) {
    is_selecting_ = false;
    selection_rect_ = selection_rect_.normalized();
    // Clamp to image bounds
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

void ImageCanvas::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    if (is_selecting_ || selection_rect_.isValid()) {
      is_selecting_ = false;
      selection_rect_ = QRect();
      emit selection_changed(selection_rect_);
      update();
    }
  }
  QWidget::keyPressEvent(event);
}

void ImageCanvas::wheelEvent(QWheelEvent *event) {
  const QPoint delta = event->angleDelta();

  if (event->modifiers() & Qt::ControlModifier) {
    // Zoom anchored to cursor pixel — use floating-point to avoid
    // truncation loss in the intermediate canvas_to_image/image_to_canvas round-trip
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

    QPointF new_canvas(
        image_pos.x() * new_zoom + view_offset_.x(),
        image_pos.y() * new_zoom + view_offset_.y());

    view_offset_.rx() += static_cast<int>(
        std::floor(canvas_pos.x() - new_canvas.x()));
    view_offset_.ry() += static_cast<int>(
        std::floor(canvas_pos.y() - new_canvas.y()));
  } else if (event->modifiers() & Qt::ShiftModifier) {
    // Horizontal scroll
    int dx = delta.x() != 0 ? delta.x() : delta.y();
    view_offset_.rx() -= dx;
    clampViewOffset();
  } else {
    // Vertical scroll
    view_offset_.ry() += delta.y();
    clampViewOffset();
  }

  emit view_changed();
  update();
  QWidget::wheelEvent(event);
}

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

QRect ImageCanvas::image_to_canvas(const QRect &image_rect) const {
  QPoint top_left = image_to_canvas(image_rect.topLeft());
  QPoint bottom_right_exclusive = image_to_canvas(
      QPoint(image_rect.x() + image_rect.width(),
             image_rect.y() + image_rect.height()));
  return QRect(top_left.x(), top_left.y(),
               bottom_right_exclusive.x() - top_left.x(),
               bottom_right_exclusive.y() - top_left.y());
}

QRect ImageCanvas::canvas_to_image(const QRect &canvas_rect) const {
  QPoint top_left = canvas_to_image(canvas_rect.topLeft());
  QPoint bottom_right_exclusive = canvas_to_image(
      QPoint(canvas_rect.x() + canvas_rect.width(),
             canvas_rect.y() + canvas_rect.height()));
  return QRect(top_left.x(), top_left.y(),
               bottom_right_exclusive.x() - top_left.x(),
               bottom_right_exclusive.y() - top_left.y());
}

QRect ImageCanvas::selection() const {
  return selection_rect_;
}

void ImageCanvas::set_selection(const QRect &image_rect) {
  is_selecting_ = false;
  selection_rect_ = image_rect;
  update();
}

void ImageCanvas::clear_selection() {
  selection_rect_ = QRect();
  emit selection_changed(selection_rect_);
  update();
}

void ImageCanvas::on_document_modified() {
  update();
}

void ImageCanvas::clampViewOffset() {
  if (!document_ || !document_->is_valid()) {
    return;
  }
  const auto &image_data = document_->image_data();
  int scaled_w = static_cast<int>(image_data.width() * zoom_factor_);
  int scaled_h = static_cast<int>(image_data.height() * zoom_factor_);

  int max_x_offset = -(scaled_w - width());
  int max_y_offset = -(scaled_h - height());

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
