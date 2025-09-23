#pragma once
#include <QMainWindow>
#include <QMenubar>
#include <QSplitter>
#include "layout/JToolbar.h"
#include "layout/JBody.h"
#include "layout/JRightside.h"

class ImageCanvas;
class RightSidebar;  // 新增前置声明

class MainFrame : public QMainWindow {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);

private:
    void buildUi();
    void connectSignals();
    void buildMenubar();
    void buildBody(QSplitter* splitter);
    void buildRightside(QSplitter* splitter);

private:
    JToolbar* toolbar_;
    JBody* body_;
    JRightside* rightside_;

    // 打开图片
    QAction* act_open_;
    // 直方图
    QAction* act_hist_;

    quint32 width_ = 1200;
    quint32 height_ = 800;

    quint32 left_width_ = width_ * 0.8;
    quint32 right_width_ = width_ * 0.2;

};
