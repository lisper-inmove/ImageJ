#pragma once

#include <QWidget>

class JBody: public QWidget {
    Q_OBJECT
public:
    JBody(QWidget* parent = nullptr);
    void build();
    void setWidth(quint32 v) {width_ = v;}
    void setHeight(quint32 v) {height_ = v;}

private:
    quint32 width_;
    quint32 height_;
};
