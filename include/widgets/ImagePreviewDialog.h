#pragma once

#include <QDialog>
#include <QImage>

class ImagePreviewDialog: public QDialog {
    Q_OBJECT
public:
    explicit ImagePreviewDialog(const QImage& img, QWidget* parent = nullptr);
};
