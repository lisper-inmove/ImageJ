#include "image_canvas/ImageCanvas.h"

#include <QRubberBand>
#include <QMouseEvent>
#include <QPointF>

#include "widgets/ImagePreviewDialog.h"
#include "widgets/HistogramDialog.h"

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
    } else if (selecting_ && rb_) {
        QPointF imgPoint = toImgCoord(event->pos());
        if (imgPoint.x() != -1) {
            selectEnd_ = event->pos();
        }
        rb_->setGeometry(QRect(selectStart_, selectEnd_).normalized());
        event->accept();
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

void ImageCanvas::selectRect(QMouseEvent* event) {
    selecting_ = true;
    selectStart_ = event->pos();
    selectEnd_ = event->pos();
    if (!rb_) {
        rb_ = std::make_unique<QRubberBand>(QRubberBand::Rectangle, this);
    }
    rb_->setGeometry(QRect(selectStart_, QSize()));
    rb_->show();
    event->accept();
}

void ImageCanvas::mousePressEvent(QMouseEvent* event) {
    const bool ctrl = event->modifiers() & Qt::ControlModifier;
    const bool left = event->button() == Qt::LeftButton;
    if (ctrl && left && !img_.isNull()) {
        selectRect(event);
    } else {
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
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (dragging_) {
        dragging_ = false;
        setCursor(Qt::CrossCursor);
    }
    if (selecting_) {
        onSelectFinish();
    }
	QWidget::mouseReleaseEvent(event);
}


void ImageCanvas::onSelectFinish() {
    selecting_ = false;
    rb_->hide();
    QPoint start = toImgCoord(selectStart_).toPoint();
    QPoint end = toImgCoord(selectEnd_).toPoint();
    QRect rect = QRect(start, end).normalized();
    if (rect.width() > 10 || rect.height() > 10) {
        rect = rect.intersected(QRect(0, 0, img_.width(), img_.height()));
        QImage cropped = img_.copy(rect);
        if (hSelecting_) {
            HistogramDialog::showForImage(cropped, this);
            // hSelecting_ = false;
        } else {
            ImagePreviewDialog dlg(cropped, this);
            dlg.exec();
        }
    }
}
