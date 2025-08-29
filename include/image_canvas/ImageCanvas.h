#pragma once

#include <QWidget>
#include <QImage>
#include <QPointF>
#include <QPoint>
#include <QSize>
#include <QString>

class QPainter;
class QMouseEvent;
class QWheelEvent;
class QResizeEvent;
class QDragEnterEvent;
class QDropEvent;

class ImageCanvas : public QWidget {
    Q_OBJECT
public:
    explicit ImageCanvas(QWidget* parent = nullptr);

    // 视图状态
    double  zoom() const { return zoom_; }                       // 逻辑 zoom_（相对自适应基准）
    QPointF offset() const { return offset_; }                   // 平移偏移（相对居中原点）
    double  effectiveScale() const;                              // 实际显示倍数（= baseScale * zoom_）
    QSize   imageSize() const { return img_.size(); }
    QString imagePath() const { return imagePath_; }
    QImage  currentImage() const { return img_; }               // 供外部读取图像（QImage 是写时拷贝，安全）

public slots:
    // 供右侧栏按钮调用的操作
    void openImage();           // 打开图片（仅转调 chooseImage）
    void resetView();           // 复位到“自适应 + 居中”
    void fitToWindow();         // 同 resetView
    void zoomActualPixels();    // 100%像素显示（1 显示像素 = 1 图像像素）

signals:
    void imageInfoChanged(const QString& path, const QSize& size);                // 加载新图后触发
    void viewChanged(double zoom, const QPointF& offset, double scale);           // 缩放/平移/窗口变化时触发
    void mousePositionChanged(const QPoint& widgetPos, const QPointF& imagePos,   // 实时鼠标坐标
                              bool insideImage);

protected:
    // 绘制与事件处理
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    void drawCrosshair(QPainter&);
    void chooseImage();
    bool loadImage(const QString& path);
    double baseScale(const QSize& widgetSize) const;

    // 状态改变后统一发信号
    void notifyViewChanged();
    void notifyMousePos(const QPoint& widgetPos);  // 计算并发送鼠标位置信号

private:
    QImage  img_;
    QString imagePath_;
    double  zoom_   = 1.0;      // 额外缩放倍数（在 baseScale 基础上叠加）
    QPointF offset_ {0, 0};     // 相对“居中原点”的平移偏移（窗口坐标）
    bool    dragging_ = false;  // 是否正在拖拽
    QPoint  lastPos_;           // 拖拽时记录上一次鼠标位置
};
