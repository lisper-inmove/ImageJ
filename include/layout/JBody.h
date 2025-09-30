#pragma once

#include <QWidget>=
#include <QResizeEvent>
#include "widgets/JCanvas.h"

class JBody: public QWidget {
    Q_OBJECT
public:
    JBody(QWidget* parent = nullptr);
    ~JBody();
    void build(QString path);
    void buildCanvas(QString path);

public:
    void setWidth(quint32 v) {width_ = v;}
    void setHeight(quint32 v) {height_ = v;}

public:
    void open();

private:
    quint32 width_;
    quint32 height_;
    JCanvas* canvas_;

private:
    void resizeEvent(QResizeEvent*) override;
};
