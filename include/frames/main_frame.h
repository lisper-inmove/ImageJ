#pragma once
#include <QWidget>

class ImageCanvas;
class RightSidebar;

class MainFrame : public QWidget {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);

private:
    void buildUi();
    void connectSignals();
};
