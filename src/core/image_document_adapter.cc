#include "core/image_document_adapter.h"
#include "core/image_document.h"
#include <cstring>

ImageDocumentAdapter::ImageDocumentAdapter(ImageDocument *document)
    : document_(document) {}

ImageDocumentAdapter::~ImageDocumentAdapter() = default;

void ImageDocumentAdapter::set_document(ImageDocument *document) {
  document_ = document;
}

ImageDocument *ImageDocumentAdapter::document() const { return document_; }

bool ImageDocumentAdapter::can_convert_to_qimage() const {
  return document_ != nullptr && document_->is_valid();
}

QImage ImageDocumentAdapter::to_qimage() const {
  if (!can_convert_to_qimage()) {
    return QImage();
  }

  const ImageData &image_data = document_->image_data();
  QImage::Format qfmt = to_qimage_format(image_data.format());
  if (qfmt == QImage::Format_Invalid) {
    return QImage();
  }

  // Expand grayscale to RGB for display to avoid Format_Grayscale8
  // rendering quirks with externally-owned buffers
  if (qfmt == QImage::Format_Grayscale8) {
    QImage result(image_data.width(), image_data.height(),
                  QImage::Format_RGB888);
    for (int y = 0; y < image_data.height(); ++y) {
      const uint8_t* src = image_data.pixel(0, y);
      uint8_t* dst = result.scanLine(y);
      for (int x = 0; x < image_data.width(); ++x) {
        dst[x * 3] = src[x];
        dst[x * 3 + 1] = src[x];
        dst[x * 3 + 2] = src[x];
      }
    }
    return result;
  }

  int data_stride = calculate_bytes_per_line(image_data);
  int depth = image_data.channels() * 8;

  // QImage requires 4-byte aligned scanlines
  int qt_bytes_per_line = ((image_data.width() * depth + 31) / 32) * 4;

  if (data_stride == qt_bytes_per_line) {
    // Fast path: stride matches, wrap buffer directly
    return QImage(const_cast<uint8_t *>(image_data.data()),
                  image_data.width(), image_data.height(),
                  data_stride, qfmt);
  }

  // Slow path: stride mismatch, need to copy to aligned QImage
  QImage result(image_data.width(), image_data.height(), qfmt);
  for (int y = 0; y < image_data.height(); ++y) {
    std::memcpy(result.scanLine(y), image_data.pixel(0, y), data_stride);
  }
  return result;
}

QPixmap ImageDocumentAdapter::to_qpixmap() const {
  QImage img = to_qimage();
  if (img.isNull()) {
    return QPixmap();
  }
  return QPixmap::fromImage(img);
}

bool ImageDocumentAdapter::is_valid() const {
  return document_ != nullptr && document_->is_valid();
}

QSize ImageDocumentAdapter::qsize() const {
  if (!is_valid()) {
    return QSize();
  }
  return QSize(document_->image_data().width(),
               document_->image_data().height());
}

bool ImageDocumentAdapter::update_from_qimage(const QImage &qimage) {
  if (qimage.isNull() || !document_) {
    return false;
  }

  ImageData::PixelFormat pixel_format = from_qimage_format(qimage.format());
  if (pixel_format == ImageData::PixelFormat::kUnknown) {
    return false;
  }

  QImage converted = qimage;
  QImage::Format target_fmt = to_qimage_format(pixel_format);
  if (qimage.format() != target_fmt) {
    converted = qimage.convertToFormat(target_fmt);
    if (converted.isNull()) {
      return false;
    }
  }

  ImageData &image_data = document_->image_data();
  if (!image_data.create(converted.width(), converted.height(), pixel_format)) {
    return false;
  }

  // Copy row-by-row: QImage may have padded scanlines (4-byte alignment),
  // while ImageData stores pixels tightly packed
  const uint8_t *src_bits = converted.constBits();
  int src_bytes_per_line = converted.bytesPerLine();
  int dst_stride = ImageData::calculate_stride(converted.width(), pixel_format);
  uint8_t *dst = image_data.data();
  for (int y = 0; y < converted.height(); y++) {
    std::memcpy(dst + y * dst_stride,
                src_bits + y * src_bytes_per_line,
                dst_stride);
  }

  document_->notify_changed();
  return true;
}

bool ImageDocumentAdapter::update_from_qpixmap(const QPixmap &qpixmap) {
  if (qpixmap.isNull()) {
    return false;
  }
  return update_from_qimage(qpixmap.toImage());
}

QImage::Format
ImageDocumentAdapter::to_qimage_format(ImageData::PixelFormat format) {
  switch (format) {
    case ImageData::PixelFormat::kGray8:
      return QImage::Format_Grayscale8;
    case ImageData::PixelFormat::kRGB24:
      return QImage::Format_RGB888;
    case ImageData::PixelFormat::kRGBA32:
      return QImage::Format_RGBA8888;
    default:
      return QImage::Format_Invalid;
  }
}

ImageData::PixelFormat
ImageDocumentAdapter::from_qimage_format(QImage::Format format) {
  switch (format) {
    case QImage::Format_Grayscale8:
      return ImageData::PixelFormat::kGray8;
    case QImage::Format_RGB888:
      return ImageData::PixelFormat::kRGB24;
    case QImage::Format_RGBA8888:
      return ImageData::PixelFormat::kRGBA32;
    default:
      return ImageData::PixelFormat::kUnknown;
  }
}

int ImageDocumentAdapter::calculate_bytes_per_line(const ImageData &data) {
  return ImageData::calculate_stride(data.width(), data.format());
}
