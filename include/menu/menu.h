#pragma once

#include <QObject>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>

class QMainWindow;

class Menu: public QObject {
    Q_OBJECT
public:
    explicit Menu(QMainWindow* main);
    void build();

private:
    QMainWindow* mw_{nullptr};

    QMenu* mFile_{};
    QAction* actOpen_{};

    QMenu* mOperate_{};
    QAction* actHistogram_{};

private:
    void buildMFile();
    void buildMOperate();

signals:
    // 打开新图片
    void openRequested();
    // 灰度直方图
    void histogramRequested();
};
