#include "dialogs/histogram_dialog.h"

#include <QPainter>
#include <QPen>

#include "core/image_data.h"
#include "core/image_document.h"

HistogramDialog::HistogramDialog(ImageDocument* doc,
                                 const QRect& selection,
                                 QWidget* parent)
    : QDialog(parent), document_(doc), selection_(selection) {
  setWindowTitle("灰度直方图");
  setMinimumSize(540, 300);
  resize(600, 360);

  histogram_.fill(0);
  computeHistogram();
}

void HistogramDialog::computeHistogram() {
  if (!document_ || !document_->is_valid()) {
    return;
  }

  const auto& data = document_->image_data();
  const int img_w = data.width();
  const int img_h = data.height();

  // Determine the effective rect to compute histogram over
  QRect effective_rect;
  if (selection_.isValid() && !selection_.isEmpty()) {
    effective_rect = selection_.intersected(QRect(0, 0, img_w, img_h));
  }

  if (effective_rect.isEmpty()) {
    // Full image path
    if (data.format() == ImageData::PixelFormat::kGray8) {
      // Fast path: iterate raw buffer directly
      const uint8_t* buf = data.data();
      const size_t total = static_cast<size_t>(img_w) * img_h;
      for (size_t i = 0; i < total; ++i) {
        histogram_[buf[i]]++;
      }
    } else {
      // kRGB24 or kRGBA32: pixel-by-pixel with luminance
      for (int y = 0; y < img_h; ++y) {
        for (int x = 0; x < img_w; ++x) {
          const uint8_t* p = data.pixel(x, y);
          int gray;
          if (data.format() == ImageData::PixelFormat::kRGBA32) {
            gray = static_cast<int>(0.299 * p[0] + 0.587 * p[1] +
                                    0.114 * p[2]);
          } else {  // kRGB24
            gray = static_cast<int>(0.299 * p[0] + 0.587 * p[1] +
                                    0.114 * p[2]);
          }
          if (gray < 0) gray = 0;
          if (gray > 255) gray = 255;
          histogram_[gray]++;
        }
      }
    }
  } else {
    // Sub-rect (selection) path
    for (int y = effective_rect.y(); y < effective_rect.bottom(); ++y) {
      for (int x = effective_rect.x(); x < effective_rect.right(); ++x) {
        const uint8_t* p = data.pixel(x, y);
        int gray;
        if (data.format() == ImageData::PixelFormat::kGray8) {
          gray = p[0];
        } else if (data.format() == ImageData::PixelFormat::kRGBA32) {
          gray = static_cast<int>(0.299 * p[0] + 0.587 * p[1] +
                                  0.114 * p[2]);
        } else {  // kRGB24
          gray = static_cast<int>(0.299 * p[0] + 0.587 * p[1] +
                                  0.114 * p[2]);
        }
        if (gray < 0) gray = 0;
        if (gray > 255) gray = 255;
        histogram_[gray]++;
      }
    }
  }
}

void HistogramDialog::paintEvent(QPaintEvent* /*event*/) {
  QPainter painter(this);
  painter.fillRect(rect(), Qt::white);

  const int left_margin = 50;
  const int bottom_margin = 40;
  const int top_margin = 10;
  const int right_margin = 10;

  const int chart_x = left_margin;
  const int chart_y = top_margin;
  const int chart_w = width() - left_margin - right_margin;
  const int chart_h = height() - top_margin - bottom_margin;

  if (chart_w <= 0 || chart_h <= 0) return;

  // Find max value for Y scaling
  int max_val = 1;
  for (int i = 0; i < 256; ++i) {
    if (histogram_[i] > max_val) max_val = histogram_[i];
  }

  // Draw dotted grid lines and Y-axis labels
  painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
  QFont label_font = painter.font();
  label_font.setPixelSize(10);
  painter.setFont(label_font);

  for (int i = 0; i < 5; ++i) {
    int y = chart_y + chart_h - (chart_h * i / 4);
    painter.drawLine(chart_x, y, chart_x + chart_w, y);

    int tick_val = max_val * i / 4;
    QString label = QString::number(tick_val);
    QRect text_rect(0, y - 8, left_margin - 5, 16);
    painter.setPen(Qt::black);
    painter.drawText(text_rect, Qt::AlignRight | Qt::AlignVCenter, label);
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
  }

  // Draw bars
  const double bar_width = static_cast<double>(chart_w) / 256.0;
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(80, 80, 80));

  for (int i = 0; i < 256; ++i) {
    if (histogram_[i] == 0) continue;
    int bar_h = static_cast<int>((static_cast<double>(histogram_[i]) / max_val) * chart_h);
    if (bar_h < 1) bar_h = 1;
    int bx = chart_x + static_cast<int>(i * bar_width);
    int bw = static_cast<int>((i + 1) * bar_width) - bx;
    if (bw < 1) bw = 1;
    painter.drawRect(bx, chart_y + chart_h - bar_h, bw, bar_h);
  }

  // Draw axes
  painter.setPen(QPen(Qt::black, 1));
  painter.drawLine(chart_x, chart_y, chart_x, chart_y + chart_h);  // Y axis
  painter.drawLine(chart_x, chart_y + chart_h, chart_x + chart_w, chart_y + chart_h);  // X axis

  // X-axis ticks and labels
  const int x_ticks[] = {0, 64, 128, 192, 255};
  painter.setPen(Qt::black);
  for (int tick : x_ticks) {
    int tx = chart_x + static_cast<int>(tick * bar_width + bar_width / 2);
    painter.drawLine(tx, chart_y + chart_h, tx, chart_y + chart_h + 5);
    QString label = QString::number(tick);
    QRect text_rect(tx - 15, chart_y + chart_h + 6, 30, bottom_margin - 8);
    painter.drawText(text_rect, Qt::AlignHCenter | Qt::AlignTop, label);
  }
}
