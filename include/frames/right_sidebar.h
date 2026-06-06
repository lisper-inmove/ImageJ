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

class RightSidebar : public QWidget {
  Q_OBJECT
 public:
  explicit RightSidebar(QWidget* parent = nullptr);

  void set_document(ImageDocument* doc);
  void set_zoom_factor(double factor);
  void update_selection_info(const QRect& image_rect);
  void add_selection(const QRect& image_rect);
  void enable_color_space_combo(bool enabled);
  void reset_color_space_combo();

 private slots:
  void onHistogramButtonClicked();
  void onEqualizeHistClicked();
  void onCLAHEHistClicked();
  void updateChannelSliders(int colorSpaceIndex);

 signals:
  void selection_restore_requested(const QRect& image_rect);
  void color_space_changed(int index);
  void channel_gains_changed(const QVector<int>& gains);

 private:
  void buildUi();
  void updateImageInfoTab();
  void onSelectionItemClicked(QListWidgetItem* item);
  void emitChannelGains();

  struct ChannelSlider {
    QLabel* label;
    QSlider* slider;
    QSpinBox* spinbox;
    QString base_name;
  };

  // Tabs
  QTabWidget* tabs_;

  // Tab 1 — Image Info
  QWidget* info_tab_;
  QLabel* size_label_;
  QLabel* type_label_;
  QLabel* zoom_label_;
  QLabel* rotation_label_;
  QLabel* selection_info_label_;

  // Tab 2 — Tools
  QWidget* tools_tab_;
  QPushButton* histogram_btn_;
  QPushButton* equalize_hist_btn_;
  QPushButton* clahe_btn_;
  QComboBox* colorspace_combo_;
  QWidget* channel_sliders_widget_;
  QVBoxLayout* channel_sliders_layout_;
  QVector<ChannelSlider> channel_sliders_;

  // Tab 3 — Selection History
  QListWidget* selection_list_;

  // State
  ImageDocument* document_;
  double zoom_factor_;
  double rotation_;
  QRect current_selection_;

  QVBoxLayout* main_layout_;
};

#endif  // IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
