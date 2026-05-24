#ifndef IMAGEJ_DIALOGS_HISTOGRAM_DIALOG_H_
#define IMAGEJ_DIALOGS_HISTOGRAM_DIALOG_H_

#include <QDialog>
#include <array>
#include <QRect>

class ImageDocument;

class HistogramDialog : public QDialog {
  Q_OBJECT
 public:
  explicit HistogramDialog(ImageDocument* doc,
                           const QRect& selection,
                           QWidget* parent = nullptr);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  void computeHistogram();

  ImageDocument* document_;
  QRect selection_;
  std::array<int, 256> histogram_;
};

#endif  // IMAGEJ_DIALOGS_HISTOGRAM_DIALOG_H_
