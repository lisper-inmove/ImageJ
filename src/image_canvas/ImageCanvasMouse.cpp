#include "image_canvas/ImageCanvas.h"
#include <QMouseEvent>
#include <QPointF>

void ImageCanvas::mouseMoveEvent(QMouseEvent* event) {
	if (img_.isNull()) {
		event->ignore();
		return;
	}
	QPointF pos = event->position();
	if (dragging_) {
		QPoint delta = pos.toPoint() - lastPos_;
		const QPointF wanted = delta + offset_;
		offset_ = clampOffsetForImage(wanted);
		lastPos_ = pos.toPoint();
		update();
	}
	QWidget::mouseMoveEvent(event);
	emit mouseMoved(pos.toPoint(), toImgCoord(pos.toPoint()));
}

QPointF ImageCanvas::toImgCoord(const QPoint& widgetCoord) const {
	if (img_.isNull()) {
		return QPointF(-1, -1);
	}
	const double scale = effectiveScale();
	const QPointF iOrigin = imageOrigin();
	const QPointF origin = iOrigin + offset_;
	const QPointF imgCoord = (widgetCoord - origin) / scale;
	if (imgCoord.x() < 0 || imgCoord.x() >= img_.width() ||
		imgCoord.y() < 0 || imgCoord.y() >= img_.height()) {
		return QPointF(-1, -1);
	}
	return imgCoord;
}

void ImageCanvas::mousePressEvent(QMouseEvent* event) {
	if (img_.isNull()) {
		chooseImage();
	}
	else {
		setCursor(Qt::ClosedHandCursor);
		dragging_ = true;
		lastPos_ = event->pos();
	}
	QWidget::mousePressEvent(event);
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent* event) {
	dragging_ = false;
	setCursor(Qt::CrossCursor);
	QWidget::mouseReleaseEvent(event);
}