#ifndef IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
#define IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_

#include <QWidget>

class QLabel;
class QTabWidget;
class QListWidget;
class QVBoxLayout;
class ImageDocument;

class RightSidebar : public QWidget {
  Q_OBJECT
 public:
  explicit RightSidebar(QWidget* parent = nullptr);

  void set_document(ImageDocument* doc);
  void set_zoom_factor(double factor);
  void add_selection(const QRect& image_rect);

 private:
  void buildUi();
  void updateImageInfoTab();

  // Tabs
  QTabWidget* tabs_;

  // Tab 1 — Image Info
  QWidget* info_tab_;
  QLabel* size_label_;
  QLabel* type_label_;
  QLabel* zoom_label_;
  QLabel* rotation_label_;

  // Tab 2 — Tools
  QWidget* tools_tab_;

  // Tab 3 — Selection History
  QListWidget* selection_list_;

  // State
  ImageDocument* document_;
  double zoom_factor_;
  double rotation_;

  QVBoxLayout* main_layout_;
};

#endif  // IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
