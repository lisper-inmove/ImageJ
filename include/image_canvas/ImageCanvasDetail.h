#pragma once
#include <QSize>
#include <QPointF>
#include <algorithm>

namespace imgcanvas_detail {

// 计算在给定 scale 下，让图像“等比适配窗口”时的左上角原点（使图像居中）
inline QPointF centerBaseOrigin(const QSize& widgetSize, const QSize& imgSize, double scale) {
    const double w = imgSize.width()  * scale;
    const double h = imgSize.height() * scale;
    return QPointF( (widgetSize.width()  - w) / 2.0,
                   (widgetSize.height() - h) / 2.0 );
}

// 将 offset 限制在 [-dx,+dx] × [-dy,+dy]，避免图片被拖出可视区域
inline QPointF clampOffsetForImage(const QSize& widgetSize,
                                   const QSize& imgSize,
                                   double scale,
                                   const QPointF& desiredOffset)
{
    /**
        图片中心与视窗的中心的偏移。用图片的中心减去视窗的中心
    */
    const double w = imgSize.width()  * scale;
    const double h = imgSize.height() * scale;
    const double dx = std::max(0.0, (w - widgetSize.width())  / 2.0);
    const double dy = std::max(0.0, (h - widgetSize.height()) / 2.0);

    const double ox = std::clamp(desiredOffset.x(), -dx, dx);
    const double oy = std::clamp(desiredOffset.y(), -dy, dy);
    return QPointF(ox, oy);
}

} // namespace imgcanvas_detail
