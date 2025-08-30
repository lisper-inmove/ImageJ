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

private:
    QTabWidget* tabs_{nullptr};
    QLabel* lbPath_{nullptr};
    QLabel* lbSize_{ nullptr };
    QLabel* lbMouse_{ nullptr };
    int32_t width_{300};
};
