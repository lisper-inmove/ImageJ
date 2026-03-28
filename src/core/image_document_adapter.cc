#include "core/image_document_adapter.h"
#include "core/image_document.h"

ImageDocumentAdapter::ImageDocumentAdapter(ImageDocument* document) : document_(document) {}

ImageDocumentAdapter::~ImageDocumentAdapter() = default;

void ImageDocumentAdapter::set_document(ImageDocument* document) { document_ = document; }
ImageDocument* ImageDocumentAdapter::document() const { return document_; }

bool ImageDocumentAdapter::can_convert_to_qimage() const { return false; }
QImage ImageDocumentAdapter::to_qimage() const { return QImage(); }
QPixmap ImageDocumentAdapter::to_qpixmap() const { return QPixmap(); }

bool ImageDocumentAdapter::is_valid() const { return false; }
QSize ImageDocumentAdapter::qsize() const { return QSize(); }

bool ImageDocumentAdapter::update_from_qimage(const QImage& qimage) { return false; }
bool ImageDocumentAdapter::update_from_qpixmap(const QPixmap& qpixmap) { return false; }

QImage::Format ImageDocumentAdapter::to_qimage_format(ImageData::PixelFormat format) { return QImage::Format_Invalid; }
ImageData::PixelFormat ImageDocumentAdapter::from_qimage_format(QImage::Format format) { return ImageData::PixelFormat::kUnknown; }