#include "widgets/image_canvas.h"

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
      background_color_(QColor(0x2D, 0x2D, 0x2D)) {
  setMinimumSize(100, 100);
  setMouseTracking(true);
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
  zoom_factor_ = std::min(scale_x, scale_y);
  update();
}

void ImageCanvas::reset_zoom() {
  zoom_factor_ = 1.0;
  update();
}

QPoint ImageCanvas::view_offset() const {
  return view_offset_;
}

void ImageCanvas::set_view_offset(const QPoint &offset) {
  view_offset_ = offset;
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
}

void ImageCanvas::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
}

void ImageCanvas::mousePressEvent(QMouseEvent *event) {
  QPoint image_pos = canvas_to_image(event->pos());
  emit image_clicked(image_pos, event->button());
  QWidget::mousePressEvent(event);
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event) {
  QPoint image_pos = canvas_to_image(event->pos());
  emit mouse_over_image(image_pos);
  QWidget::mouseMoveEvent(event);
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event) {
  QWidget::mouseReleaseEvent(event);
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
  } else {
    // Vertical scroll
    view_offset_.ry() -= delta.y();
  }

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

void ImageCanvas::on_document_modified() {
  update();
}
