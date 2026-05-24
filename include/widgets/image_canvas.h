#ifndef IMAGEJ_WIDGETS_IMAGE_CANVAS_H_
#define IMAGEJ_WIDGETS_IMAGE_CANVAS_H_

#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QColor>

class ImageDocument;

class ImageCanvas : public QWidget {
  Q_OBJECT

 public:
  enum class BackgroundStyle {
    kCheckerboard,
    kSolidColor,
    kTransparent
  };

  explicit ImageCanvas(QWidget* parent = nullptr);
  ~ImageCanvas() override;

  void set_document(ImageDocument* document);
  ImageDocument* document() const;

  void set_zoom_factor(double factor);
  double zoom_factor() const;
  void fit_to_window();
  void reset_zoom();

  QPoint view_offset() const;
  void set_view_offset(const QPoint& offset);
  QRect visible_image_rect() const;

  void set_background_style(BackgroundStyle style);
  BackgroundStyle background_style() const;
  void set_background_color(const QColor& color);
  QColor background_color() const;

  QRect selection() const;
  void clear_selection();

  void update_display();
  void force_redraw();

  // Event overrides
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

  // Coordinate conversion
  QPoint image_to_canvas(const QPoint& image_point) const;
  QPoint canvas_to_image(const QPoint& canvas_point) const;
  QRect image_to_canvas(const QRect& image_rect) const;
  QRect canvas_to_image(const QRect& canvas_rect) const;

 signals:
  void document_changed(ImageDocument* new_document);
  void view_changed();
  void mouse_over_image(const QPoint& image_position);
  void image_clicked(const QPoint& image_position, Qt::MouseButton button);
  void selection_changed(const QRect& image_rect);

 public slots:
  void on_document_modified();

 private:
  ImageDocument* document_;
  double zoom_factor_;
  QPoint view_offset_;
  BackgroundStyle background_style_;
  QColor background_color_;
  bool is_selecting_;
  QRect selection_rect_;  // in image coordinates

  void clampViewOffset();
};

#endif  // IMAGEJ_WIDGETS_IMAGE_CANVAS_H_