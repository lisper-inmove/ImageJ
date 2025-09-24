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
    void build(QString path);

private:
    void buildUi(QString path);
    void connectSignals();
    void buildMenubar();
    void buildBody(QSplitter* splitter, QString path);
    void buildRightside(QSplitter* splitter);

private:
    JToolbar* toolbar_;
    JBody* body_;
    JRightside* rightside_;

    // 打开图片
    QAction* actOpen_;
    // 直方图
    QAction* actHist_;

    quint32 width_ = 1200;
    quint32 height_ = 800;

    quint32 leftWidth_ = width_ * 0.8;
    quint32 rightWidth_ = width_ * 0.2;

};
