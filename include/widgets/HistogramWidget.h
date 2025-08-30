#pragma once
#include <QWidget>
#include <QImage>

class HistogramWidget : public QWidget {
    Q_OBJECT
public:
    explicit HistogramWidget(QWidget* parent = nullptr);

    // 设置图像，内部会重新计算直方图
    void setImage(const QImage& img);

protected:
    void paintEvent(QPaintEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void leaveEvent(QEvent* ev) override;

private:
    // 将 x 坐标映射为直方图的索引（0..255），返回 -1 表示不在绘图区
    int xToBin(int x) const;

private:
    quint64 hist_[256]{};
    quint64 maxv_{1};
    quint64 total_{0};          // 总像素数

    // 绘图内边距（用于坐标换算）
    int padT_{10}, padB_{20}, padL_{10}, padR_{10};

    // 悬停的列索引（-1 表示没有）
    int hoverIndex_{-1};
};
