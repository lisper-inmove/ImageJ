#pragma once
#include <QDialog>
#include <QImage>

class QLabel;

class HistogramDialog : public QDialog {
    Q_OBJECT
public:
    explicit HistogramDialog(const QImage& img, QWidget* parent = nullptr);

    // 便捷调用：直接弹出对话框
    static void showForImage(const QImage& img, QWidget* parent = nullptr);

    // 工具方法：计算/绘制（如需别处复用）
    static void computeGrayHistogram(const QImage& img, quint64 (&hist)[256], quint64& maxv);
    static QImage renderHistogramImage(const quint64 (&hist)[256], quint64 maxv,
                                       int plotW = 256, int plotH = 160);

private:
    QLabel* imageLabel_{nullptr};
    QLabel* statsLabel_{nullptr};
};
