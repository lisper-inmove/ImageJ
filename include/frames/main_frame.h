#pragma once
#include <QWidget>

#include "core/image_data.h"

class QLabel;
class QSplitter;
class QSettings;
class QCloseEvent;
class QMenu;
class QMenuBar;
class QStatusBar;
class ImageCanvas;
class RightSidebar;

class MainFrame : public QWidget {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onColorSpaceChanged(int index);
    void onChannelGainsChanged(const QVector<int>& gains);

private:
    void buildUi();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();
    void openImage();
    void openRecentFile();
    void updateStatusBarPixelInfo(const QPoint& image_pos);
    void loadWindowSettings();
    void saveWindowSettings();
    void setDefaultGeometry();
    bool validateSettings();
    void addRecentFilePath(const QString& path);
    void updateRecentFileMenu();
    void applyChannelGains(ImageData& data);

    QSplitter* splitter_;
    ImageCanvas* image_canvas_;
    RightSidebar* right_sidebar_;
    QSettings* settings_;
    QMenuBar* menu_bar_;
    QMenu* recent_menu_;
    QStatusBar* status_bar_;
    QLabel* pixel_info_label_;
    ImageData original_data_;
    int current_colorspace_;
    QVector<int> channel_gains_;
};
