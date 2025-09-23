#include "layout/JBody.h"
#include "utils/logger.h"

JBody::JBody(QWidget* parent): QWidget(parent) {

}

JBody::~JBody() {
    delete canvas_;
}

void JBody::build() {
    setWindowTitle("JBody");
    setObjectName("JBody");
    setMinimumSize(width_, height_);

    // QPalette pal = palette();
    // pal.setColor(QPalette::Window, Qt::white);
    // setAutoFillBackground(true);
    // setPalette(pal);
    buildCanvas();

    show();
}

void JBody::buildCanvas() {
    canvas_ = new JCanvas(this);
    QSize size(width_, height_);
    canvas_->setSize(size);
    canvas_->show();
}

void JBody::open() {
    LOG_DEBUG("Menu Open is clicked...");
    canvas_->open();
}
