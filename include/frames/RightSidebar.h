#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QLabel>

class RightSidebar: public QWidget {
    Q_OBJECT
public:
    RightSidebar(QWidget* parent);
    ~RightSidebar() override = default;
    void imageInfoChanged(const QString& path, const QSize& size);
	void mouseMoved(const QPoint& pos, const QPointF& imgPos);

private:
    void build();
    void connectSignals();
    void updateImagePixelInfo(const QString& path);

private:
    QTabWidget* tabs_{nullptr};
    // 图片路径
    QLabel* lbPath_{nullptr};
    // 图片实际大小
    QLabel* lbSize_{ nullptr };
    // 鼠标位置
    QLabel* lbMouse_{ nullptr };
    // 像素统计信息
    QLabel* lbPixel_{ nullptr };
    int32_t width_{300};
};
