#include "widgets/image_canvas.h"
#include "core/image_document.h"

ImageCanvas::ImageCanvas(QWidget* parent) : QWidget(parent) {}

ImageCanvas::~ImageCanvas() = default;

void ImageCanvas::set_document(ImageDocument* document) {}
ImageDocument* ImageCanvas::document() const { return nullptr; }

void ImageCanvas::set_zoom_factor(double factor) {}
double ImageCanvas::zoom_factor() const { return 1.0; }
void ImageCanvas::fit_to_window() {}
void ImageCanvas::reset_zoom() {}

QPoint ImageCanvas::view_offset() const { return QPoint(); }
void ImageCanvas::set_view_offset(const QPoint& offset) {}
QRect ImageCanvas::visible_image_rect() const { return QRect(); }

void ImageCanvas::set_background_style(BackgroundStyle style) {}
ImageCanvas::BackgroundStyle ImageCanvas::background_style() const { return BackgroundStyle::kCheckerboard; }
void ImageCanvas::set_background_color(const QColor& color) {}
QColor ImageCanvas::background_color() const { return QColor(); }

void ImageCanvas::update_display() {}
void ImageCanvas::force_redraw() {}

void ImageCanvas::paintEvent(QPaintEvent* event) {}
void ImageCanvas::resizeEvent(QResizeEvent* event) {}
void ImageCanvas::mousePressEvent(QMouseEvent* event) {}
void ImageCanvas::mouseMoveEvent(QMouseEvent* event) {}
void ImageCanvas::mouseReleaseEvent(QMouseEvent* event) {}
void ImageCanvas::wheelEvent(QWheelEvent* event) {}

QPoint ImageCanvas::image_to_canvas(const QPoint& image_point) const { return QPoint(); }
QPoint ImageCanvas::canvas_to_image(const QPoint& canvas_point) const { return QPoint(); }
QRect ImageCanvas::image_to_canvas(const QRect& image_rect) const { return QRect(); }
QRect ImageCanvas::canvas_to_image(const QRect& canvas_rect) const { return QRect(); }

void ImageCanvas::on_document_modified() {}