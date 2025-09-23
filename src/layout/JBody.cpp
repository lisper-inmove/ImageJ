#include "layout/JBody.h"

JBody::JBody(QWidget* parent): QWidget(parent) {

}

void JBody::build() {
    setWindowTitle("JBody");
    setObjectName("JBody");
    setMinimumSize(width_, height_);

    // QPalette pal = palette();
    // pal.setColor(QPalette::Window, Qt::white);
    // setAutoFillBackground(true);
    // setPalette(pal);

    show();
}
