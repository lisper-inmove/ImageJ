#pragma once

#include <QWidget>

class JRightside: public QWidget {
    Q_OBJECT
public:
    JRightside(QWidget* parent = nullptr);
    void build();
    void setWidth(quint32 v) {width_ = v;}
    void setHeight(quint32 v) {height_ = v;}

private:
    quint32 width_;
    quint32 height_;
};
