#pragma once

#include <QWidget>
#include <QSplitter>

#include "image_canvas/ImageCanvas.h"
#include "frames/RightSidebar.h"

class MainFrame : public QWidget {
    Q_OBJECT
public:
    MainFrame(QWidget* parent = nullptr);

private:
    void build();
    void connectSignals();
    void buildRightSidebar(QSplitter* splitter);
    void buildImageCanvas(QSplitter* splitter);

private:
    ImageCanvas* canvas_;
    RightSidebar* rightSidebar_;
};
