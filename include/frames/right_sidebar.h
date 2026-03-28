#ifndef IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
#define IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_

#include <QWidget>

class QLabel;
class QVBoxLayout;

class RightSidebar : public QWidget {
    Q_OBJECT
public:
    explicit RightSidebar(QWidget* parent = nullptr);

private:
    void buildUi();

    QLabel* placeholder_label_;
    QVBoxLayout* main_layout_;
};

#endif  // IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
