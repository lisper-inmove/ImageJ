#ifndef IMAGEJ_CORE_IMAGE_DOCUMENT_ADAPTER_H_
#define IMAGEJ_CORE_IMAGE_DOCUMENT_ADAPTER_H_

#include <QImage>
#include <QPixmap>
#include <QSize>

#include "image_data.h"

class ImageDocument;

class ImageDocumentAdapter {
 public:
  explicit ImageDocumentAdapter(ImageDocument* document = nullptr);
  ~ImageDocumentAdapter();

  void set_document(ImageDocument* document);
  ImageDocument* document() const;

  bool can_convert_to_qimage() const;
  QImage to_qimage() const;
  QPixmap to_qpixmap() const;

  bool is_valid() const;
  QSize qsize() const;

  bool update_from_qimage(const QImage& qimage);
  bool update_from_qpixmap(const QPixmap& qpixmap);

  static QImage::Format to_qimage_format(ImageData::PixelFormat format);
  static ImageData::PixelFormat from_qimage_format(QImage::Format format);

 private:
  ImageDocument* document_ = nullptr;
};

#endif  // IMAGEJ_CORE_IMAGE_DOCUMENT_ADAPTER_H_