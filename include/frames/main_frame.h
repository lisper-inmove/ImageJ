#pragma once
#include <QWidget>

#include "core/image_data.h"

class QLabel;
class QSplitter;
class QSettings;
class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QMenuBar;
class QStatusBar;
class ImageCanvas;
class ImageDocument;
class RightSidebar;

class MainFrame : public QWidget {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onColorSpaceChanged(int index);
    void onChannelGainsChanged(const QVector<int>& gains);

private:
    void buildUi();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();
    void loadDroppedFile(const QString& file_path);
    void setupLoadedDocument(ImageDocument* doc, const QString& file_path);
    void updateStatusBarPixelInfo(const QPoint& image_pos);
    void loadWindowSettings();
    void saveWindowSettings();
    void setDefaultGeometry();
    bool validateSettings();
    void applyChannelGains(ImageData& data);

    QSplitter* splitter_;
    ImageCanvas* image_canvas_;
    RightSidebar* right_sidebar_;
    QSettings* settings_;
    QMenuBar* menu_bar_;
    QStatusBar* status_bar_;
    QLabel* pixel_info_label_;
    ImageData original_data_;
    int current_colorspace_;
    QVector<int> channel_gains_;
};
