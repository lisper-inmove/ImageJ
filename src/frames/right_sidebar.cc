#include "frames/right_sidebar.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
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

RightSidebar::RightSidebar(QWidget *parent)
    : QWidget(parent),
      tabs_(nullptr),
      info_tab_(nullptr),
      size_label_(nullptr),
      type_label_(nullptr),
      zoom_label_(nullptr),
      rotation_label_(nullptr),
      selection_info_label_(nullptr),
      tools_tab_(nullptr),
      histogram_btn_(nullptr),
      equalize_hist_btn_(nullptr),
      clahe_btn_(nullptr),
      colorspace_combo_(nullptr),
      channel_sliders_widget_(nullptr),
      channel_sliders_layout_(nullptr),
      selection_list_(nullptr),
      document_(nullptr),
      zoom_factor_(1.0),
      rotation_(0.0),
      main_layout_(nullptr) {
  buildUi();
}

void RightSidebar::buildUi() {
  main_layout_ = new QVBoxLayout(this);
  main_layout_->setContentsMargins(4, 4, 4, 4);

  // --- Tab widget ---
  tabs_ = new QTabWidget(this);

  // Tab 1: Image Info
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

  // Tab 2: Tools
  tools_tab_ = new QWidget();
  QVBoxLayout *tools_layout = new QVBoxLayout(tools_tab_);

  colorspace_combo_ = new QComboBox(tools_tab_);
  colorspace_combo_->addItem("RGB");
  colorspace_combo_->addItem("HSV");
  colorspace_combo_->addItem("LAB");
  colorspace_combo_->addItem("Gray");
  colorspace_combo_->addItem("Binary");
  colorspace_combo_->setEnabled(false);

  tools_layout->addWidget(colorspace_combo_);

  channel_sliders_widget_ = new QWidget(tools_tab_);
  channel_sliders_layout_ = new QVBoxLayout(channel_sliders_widget_);
  channel_sliders_layout_->setContentsMargins(0, 4, 0, 0);
  tools_layout->addWidget(channel_sliders_widget_);

  // Histogram / equalization buttons, below color space section
  histogram_btn_ = new QPushButton("灰度直方图", tools_tab_);
  equalize_hist_btn_ = new QPushButton("直方图均衡化", tools_tab_);
  clahe_btn_ = new QPushButton("局部自适应直方图均衡化", tools_tab_);
  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->addWidget(histogram_btn_, 1);
  btn_layout->addWidget(equalize_hist_btn_, 1);
  btn_layout->addWidget(clahe_btn_, 1);
  tools_layout->addLayout(btn_layout);

  tools_layout->addStretch();
  tabs_->addTab(tools_tab_, "工具");

  connect(histogram_btn_, &QPushButton::clicked,
          this, &RightSidebar::onHistogramButtonClicked);
  connect(equalize_hist_btn_, &QPushButton::clicked,
          this, &RightSidebar::onEqualizeHistClicked);
  connect(clahe_btn_, &QPushButton::clicked,
          this, &RightSidebar::onCLAHEHistClicked);
  connect(colorspace_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &RightSidebar::color_space_changed);
  connect(colorspace_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &RightSidebar::updateChannelSliders);

  // Tab 3: Selection History
  selection_list_ = new QListWidget(this);
  selection_list_->setViewMode(QListView::ListMode);
  selection_list_->setIconSize(QSize(32, 32));
  selection_list_->setSelectionMode(QAbstractItemView::SingleSelection);
  tabs_->addTab(selection_list_, "选择历史");

  connect(selection_list_, &QListWidget::itemClicked,
          this, &RightSidebar::onSelectionItemClicked);

  main_layout_->addWidget(tabs_);

  setLayout(main_layout_);
}

void RightSidebar::set_document(ImageDocument *doc) {
  document_ = doc;
  updateImageInfoTab();
  selection_list_->clear();
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
    selection_info_label_->setText(
        QString("(%1,%2) %3×%4")
            .arg(image_rect.x())
            .arg(image_rect.y())
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

  // Extract thumbnail from document image
  ImageDocumentAdapter adapter(document_);
  QImage full_image = adapter.to_qimage();
  if (full_image.isNull()) {
    return;
  }

  QRect clamped = image_rect.intersected(
      QRect(0, 0, full_image.width(), full_image.height()));
  if (clamped.isEmpty()) {
    return;
  }

  QImage thumb = full_image.copy(clamped).scaled(
      64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  QString label = QString("(%1,%2) %3×%4")
                      .arg(image_rect.x())
                      .arg(image_rect.y())
                      .arg(image_rect.width())
                      .arg(image_rect.height());

  QListWidgetItem *item = new QListWidgetItem(QIcon(QPixmap::fromImage(thumb)),
                                              label);
  item->setData(Qt::UserRole, image_rect);
  selection_list_->insertItem(0, item);
}

void RightSidebar::onSelectionItemClicked(QListWidgetItem *item) {
  if (!item) {
    return;
  }
  QVariant data = item->data(Qt::UserRole);
  if (data.isValid()) {
    emit selection_restore_requested(data.toRect());
  }
}

void RightSidebar::updateImageInfoTab() {
  if (!document_ || !document_->is_valid()) {
    size_label_->setText("-");
    type_label_->setText("-");
    return;
  }

  const auto &data = document_->image_data();
  size_label_->setText(
      QString("%1 × %2 像素").arg(data.width()).arg(data.height()));

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
  if (document_->has_file_path()) {
    format_str +=
        QString(" (%1)").arg(QString::fromStdString(document_->metadata().file_format));
  }
  type_label_->setText(format_str);
}

void RightSidebar::onHistogramButtonClicked() {
  HistogramDialog dialog(document_, current_selection_, this);
  dialog.exec();
}

namespace {

cv::Mat qimageToMat(const QImage& img) {
  if (img.format() == QImage::Format_Grayscale8) {
    return cv::Mat(img.height(), img.width(), CV_8UC1,
                   const_cast<uchar*>(img.bits()),
                   static_cast<size_t>(img.bytesPerLine())).clone();
  }
  // RGB888 → BGR for OpenCV
  QImage rgb = img.convertToFormat(QImage::Format_RGB888);
  cv::Mat mat(rgb.height(), rgb.width(), CV_8UC3,
              const_cast<uchar*>(rgb.bits()),
              static_cast<size_t>(rgb.bytesPerLine()));
  cv::Mat bgr;
  cv::cvtColor(mat.clone(), bgr, cv::COLOR_RGB2BGR);
  return bgr;
}

QImage matToQImage(const cv::Mat& mat) {
  if (mat.channels() == 1) {
    return QImage(mat.data, mat.cols, mat.rows,
                  static_cast<int>(mat.step),
                  QImage::Format_Grayscale8).copy();
  }
  // BGR → RGB
  cv::Mat rgb;
  cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
  return QImage(rgb.data, rgb.cols, rgb.rows,
                static_cast<int>(rgb.step),
                QImage::Format_RGB888).copy();
}

void showImageDialog(const QImage& image, const QString& title,
                     QWidget* parent) {
  QDialog* dialog = new QDialog(parent);
  dialog->setWindowTitle(title);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout* layout = new QVBoxLayout(dialog);

  QScrollArea* scroll = new QScrollArea(dialog);
  QLabel* label = new QLabel(scroll);
  label->setPixmap(QPixmap::fromImage(image));
  label->setAlignment(Qt::AlignCenter);
  scroll->setWidget(label);
  scroll->setWidgetResizable(false);

  layout->addWidget(scroll);

  // Size dialog to fit image (up to 80% of screen)
  QScreen* screen = QGuiApplication::primaryScreen();
  QSize screen_size = screen ? screen->availableGeometry().size() : QSize(1920, 1080);
  int max_w = screen_size.width() * 4 / 5;
  int max_h = screen_size.height() * 4 / 5;
  QSize dlg_size = image.size().boundedTo(QSize(max_w, max_h));
  dialog->resize(dlg_size.expandedTo(QSize(300, 200)));

  dialog->show();
}

}  // namespace

void RightSidebar::onEqualizeHistClicked() {
  if (!document_ || !document_->is_valid()) return;

  ImageDocumentAdapter adapter(document_);
  QImage src = adapter.to_qimage();
  if (src.isNull()) return;

  // Crop to selection if active
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
    // Color: convert to HSV, equalize V, merge back
    cv::Mat hsv;
    cv::cvtColor(src_mat, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::equalizeHist(channels[2], channels[2]);
    cv::merge(channels, hsv);
    cv::Mat result;
    cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);
    showImageDialog(matToQImage(result), "直方图均衡化", this);
  }
}

void RightSidebar::onCLAHEHistClicked() {
  if (!document_ || !document_->is_valid()) return;

  ImageDocumentAdapter adapter(document_);
  QImage src = adapter.to_qimage();
  if (src.isNull()) return;

  // Crop to selection if active
  if (current_selection_.isValid()) {
    QRect clamped = current_selection_.intersected(src.rect());
    if (!clamped.isEmpty()) {
      src = src.copy(clamped);
    }
  }

  cv::Mat src_mat = qimageToMat(src);

  auto clahe = cv::createCLAHE(3.0, cv::Size(8, 8));

  if (src_mat.channels() == 1) {
    cv::Mat dst;
    clahe->apply(src_mat, dst);
    showImageDialog(matToQImage(dst), "局部自适应直方图均衡化 (CLAHE)", this);
  } else {
    // Color: convert to HSV, equalize V, merge back
    cv::Mat hsv;
    cv::cvtColor(src_mat, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    clahe->apply(channels[2], channels[2]);
    cv::merge(channels, hsv);
    cv::Mat result;
    cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);
    showImageDialog(matToQImage(result), "局部自适应直方图均衡化 (CLAHE)", this);
  }
}

void RightSidebar::updateChannelSliders(int colorSpaceIndex) {
  if (!channel_sliders_layout_) {
    return;
  }

  // Clear existing sliders
  for (auto& cs : channel_sliders_) {
    // Parent widget (row) owns label/slider/spinbox, deleting it cleans up all
    QWidget* row = cs.label->parentWidget();
    channel_sliders_layout_->removeWidget(row);
    delete row;
  }
  channel_sliders_.clear();

  // Define channels for each color space: {name, min, max, default}
  struct ChannelDef {
    QString name;
    int min_val;
    int max_val;
    int default_val;
  };
  QVector<ChannelDef> channels;

  switch (colorSpaceIndex) {
    case 0:  // RGB
      channels = {{"R (红色)", 0, 255, 255},
                  {"G (绿色)", 0, 255, 255},
                  {"B (蓝色)", 0, 255, 255}};
      break;
    case 1:  // HSV
      channels = {{"H (色调)", 0, 180, 180},
                  {"S (饱和度)", 0, 255, 255},
                  {"V (明度)", 0, 255, 255}};
      break;
    case 2:  // LAB
      channels = {{"L (亮度)", 0, 255, 255},
                  {"A (绿-红)", 0, 255, 255},
                  {"B (蓝-黄)", 0, 255, 255}};
      break;
    case 3:  // Gray
      channels = {{"强度", 0, 255, 255}};
      break;
    case 4:  // Binary
      channels = {{"阈值", 0, 255, 128}};
      break;
    default:
      return;
  }

  for (const auto& ch : channels) {
    QWidget* row = new QWidget(channel_sliders_widget_);
    QHBoxLayout* row_layout = new QHBoxLayout(row);
    row_layout->setContentsMargins(0, 0, 0, 0);

    QLabel* label = new QLabel(row);
    QSlider* slider = new QSlider(Qt::Horizontal, row);
    slider->setRange(ch.min_val, ch.max_val);
    slider->setValue(ch.default_val);

    QSpinBox* spinbox = new QSpinBox(row);
    spinbox->setRange(ch.min_val, ch.max_val);
    spinbox->setValue(ch.default_val);
    spinbox->setFixedWidth(60);

    QString base_name = ch.name;
    label->setText(QString("%1: %2").arg(base_name).arg(ch.default_val));

    row_layout->addWidget(label);
    row_layout->addWidget(slider, 1);
    row_layout->addWidget(spinbox);
    channel_sliders_layout_->addWidget(row);

    // Bidirectional sync between slider and spinbox
    connect(slider, &QSlider::valueChanged, spinbox, &QSpinBox::setValue);
    connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
            slider, &QSlider::setValue);

    // Emit gains when either changes
    connect(slider, &QSlider::valueChanged,
            this, &RightSidebar::emitChannelGains);

    channel_sliders_.append({label, slider, spinbox, base_name});
  }

  emitChannelGains();
}

void RightSidebar::emitChannelGains() {
  QVector<int> gains;
  for (auto& cs : channel_sliders_) {
    int val = cs.slider->value();
    gains.append(val);
    cs.label->setText(QString("%1: %2").arg(cs.base_name).arg(val));
  }
  emit channel_gains_changed(gains);
}

void RightSidebar::enable_color_space_combo(bool enabled) {
  if (colorspace_combo_) {
    colorspace_combo_->setEnabled(enabled);
  }
}

void RightSidebar::reset_color_space_combo() {
  if (colorspace_combo_) {
    colorspace_combo_->blockSignals(true);
    colorspace_combo_->setCurrentIndex(0);  // RGB
    colorspace_combo_->blockSignals(false);
    updateChannelSliders(0);
  }
}