#include "frames/main_frame.h"

MainFrame::MainFrame(QWidget *parent) : QWidget(parent) {
    setWindowTitle("ImageJ");
    resize(1280, 720);
}

MainFrame::~MainFrame() = default;
