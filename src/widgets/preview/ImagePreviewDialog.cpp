#include "widgets/preview/ImagePreviewDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

ImagePreviewDialog::ImagePreviewDialog(const QImage& img, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("裁剪预览"));
    auto* lay = new QVBoxLayout(this);
    auto* lab = new QLabel;
    lab->setAlignment(Qt::AlignCenter);
    lab->setPixmap(QPixmap::fromImage(img));
    lay->addWidget(lab, 1);
    resize(qMin(800, img.width()+40), qMin(600, img.height()+80));
}
