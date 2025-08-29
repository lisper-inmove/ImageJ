#pragma once
#include <QWidget>
#include <QSize>
#include <QPoint>
#include <QPointF>

class QTabWidget;
class QLabel;
class QPushButton;

class RightSidebar : public QWidget {
    Q_OBJECT
public:
    explicit RightSidebar(QWidget* parent = nullptr);

    // 外部更新显示用
    void setInfo(const QString& path, const QSize& size);
    void setView(const QPointF& offset, double scale);
    void setMouse(const QPoint& widgetPos, const QPointF& imagePos, bool inside);

signals:
    // 操作按钮转发为信号，供外部（MainWidget）连接
    void openImageRequested();
    void fitToWindowRequested();
    void resetViewRequested();
    void zoomActualPixelsRequested();
    void histogramRequested();

private:
    void buildUi();

private:
    QTabWidget* tabs_{nullptr};

    // Tab1: 图片信息
    QLabel* lbPath_{nullptr};
    QLabel* lbSize_{nullptr};
    QLabel* lbScale_{nullptr};
    QLabel* lbOffset_{nullptr};
    QLabel* lbMouse_{nullptr};

    // Tab2: 操作按钮
    QPushButton* btnOpen_{nullptr};
    QPushButton* btnFit_{nullptr};
    QPushButton* btnReset_{nullptr};
    QPushButton* btnActual_{nullptr};
    QPushButton* btnHist_{nullptr};
};
