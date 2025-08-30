#pragma once

#include <QWidget>
#include <QSplitter>
#include <QMainWindow>

#include "image_canvas/ImageCanvas.h"
#include "frames/RightSidebar.h"
#include "menu/menu.h"

class MainFrame : public QMainWindow {
    Q_OBJECT
public:
    MainFrame(QMainWindow* parent = nullptr);
    void onEsc();

private:
    void build();
    void connectSignals();
    void buildRightSidebar(QSplitter* splitter);
    void buildImageCanvas(QSplitter* splitter);
    void buildMenu();

private:
    ImageCanvas* canvas_;
    RightSidebar* rightSidebar_;
    Menu* menu_;
};
