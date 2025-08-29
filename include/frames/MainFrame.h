#pragma once
#include <QWidget>

class ImageCanvas;
class RightSidebar;  // 新增前置声明

class MainFrame : public QWidget {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);
    int add(int a, int b);

private:
    void buildUi();
    void connectSignals();

private slots:
    void showHistogram();
    void onMouseChange(const QPoint& wPos, const QPointF& iPos, bool inside);

private:
    ImageCanvas*  canvas_{nullptr};
    RightSidebar* sidebar_{nullptr};
};
