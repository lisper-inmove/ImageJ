#include "frames/right_sidebar.h"

#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

#include "core/image_data.h"
#include "core/image_document.h"
#include "core/image_document_adapter.h"

RightSidebar::RightSidebar(QWidget *parent)
    : QWidget(parent),
      pixel_coord_label_(nullptr),
      pixel_value_label_(nullptr),
      tabs_(nullptr),
      info_tab_(nullptr),
      size_label_(nullptr),
      type_label_(nullptr),
      zoom_label_(nullptr),
      rotation_label_(nullptr),
      tools_tab_(nullptr),
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

  // --- Pixel info bar (always visible) ---
  pixel_coord_label_ = new QLabel("像素坐标: -", this);
  pixel_coord_label_->setStyleSheet("font-weight: bold; padding: 2px;");

  pixel_value_label_ = new QLabel("像素值: -", this);
  pixel_value_label_->setStyleSheet("padding: 2px;");

  main_layout_->addWidget(pixel_coord_label_);
  main_layout_->addWidget(pixel_value_label_);

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

  info_layout->addRow("图片大小:", size_label_);
  info_layout->addRow("类型:", type_label_);
  info_layout->addRow("缩放比例:", zoom_label_);
  info_layout->addRow("旋转角度:", rotation_label_);

  tabs_->addTab(info_tab_, "图片信息");

  // Tab 2: Tools (placeholder)
  tools_tab_ = new QWidget();
  QVBoxLayout *tools_layout = new QVBoxLayout(tools_tab_);
  QLabel *tools_placeholder = new QLabel("工具列表\n\n待实现...", tools_tab_);
  tools_placeholder->setAlignment(Qt::AlignCenter);
  tools_layout->addWidget(tools_placeholder);
  tabs_->addTab(tools_tab_, "工具");

  // Tab 3: Selection History
  selection_list_ = new QListWidget(this);
  selection_list_->setViewMode(QListView::IconMode);
  selection_list_->setIconSize(QSize(64, 64));
  selection_list_->setResizeMode(QListView::Adjust);
  selection_list_->setMovement(QListView::Static);
  selection_list_->setSelectionMode(QAbstractItemView::SingleSelection);
  tabs_->addTab(selection_list_, "选择历史");

  main_layout_->addWidget(tabs_);

  setLayout(main_layout_);
}

void RightSidebar::set_document(ImageDocument *doc) {
  document_ = doc;
  updateImageInfoTab();
  selection_list_->clear();
}

void RightSidebar::set_zoom_factor(double factor) {
  zoom_factor_ = factor;
  int percent = static_cast<int>(zoom_factor_ * 100.0);
  zoom_label_->setText(QString("%1%").arg(percent));
}

void RightSidebar::update_pixel_info(const QPoint &image_pos) {
  pixel_coord_label_->setText(
      QString("像素坐标: (%1, %2)").arg(image_pos.x()).arg(image_pos.y()));

  if (!document_ || !document_->is_valid()) {
    pixel_value_label_->setText("像素值: -");
    return;
  }

  const auto &data = document_->image_data();
  int x = image_pos.x();
  int y = image_pos.y();

  if (x < 0 || x >= data.width() || y < 0 || y >= data.height()) {
    pixel_value_label_->setText("像素值: (图像外)");
    return;
  }

  const uint8_t *p = data.pixel(x, y);
  switch (data.format()) {
    case ImageData::PixelFormat::kGray8:
      pixel_value_label_->setText(QString("像素值: %1").arg(p[0]));
      break;
    case ImageData::PixelFormat::kRGB24:
      pixel_value_label_->setText(
          QString("R: %1  G: %2  B: %3").arg(p[0]).arg(p[1]).arg(p[2]));
      break;
    case ImageData::PixelFormat::kRGBA32:
      pixel_value_label_->setText(
          QString("R: %1  G: %2  B: %3  A: %4")
              .arg(p[0]).arg(p[1]).arg(p[2]).arg(p[3]));
      break;
    default:
      pixel_value_label_->setText("像素值: (未知格式)");
      break;
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
  selection_list_->insertItem(0, item);
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
