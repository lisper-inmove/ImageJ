#include "layout/JRightside.h"

JRightside::JRightside(QWidget* parent): QWidget(parent) {

}

void JRightside::build() {
    setWindowTitle("JRightside");
    setObjectName("JRightside");
    setMinimumSize(width_, height_);

    // QPalette pal = palette();
    // pal.setColor(QPalette::Window, Qt::white);
    // setAutoFillBackground(true);
    // setPalette(pal);

    show();
}
