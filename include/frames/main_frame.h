#pragma once
#include <QWidget>

class QSplitter;
class QSettings;
class QCloseEvent;
class ImageCanvas;
class RightSidebar;

class MainFrame : public QWidget {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void connectSignals();
    void loadWindowSettings();
    void saveWindowSettings();
    void setDefaultGeometry();
    bool validateSettings();

    QSplitter* splitter_;
    ImageCanvas* image_canvas_;
    RightSidebar* right_sidebar_;
    QSettings* settings_;
};
