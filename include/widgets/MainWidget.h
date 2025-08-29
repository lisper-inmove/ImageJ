#pragma once
#include <QWidget>

class ImageCanvas;
class RightSidebar;  // 新增前置声明

class MainWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainWidget(QWidget* parent = nullptr);

private:
    void buildUi();
    void connectSignals();

private slots:
    void updateInfoTab(const QString& path, const QSize& size);
    void updateViewTab(double /*zoom*/, const QPointF& offset, double scale);
    void updateMouseLabel(const QPoint& widgetPos, const QPointF& imagePos, bool inside);
    void showHistogram();

private:
    ImageCanvas*  canvas_{nullptr};
    RightSidebar* sidebar_{nullptr};   // 替代原来的 labels / buttons / tabs
};
