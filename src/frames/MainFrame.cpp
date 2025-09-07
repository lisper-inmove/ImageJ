#include "frames/MainFrame.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QDialog>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QtMath>
#include <QLabel>
#include <QVBoxLayout>

MainFrame::MainFrame(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    connectSignals();
}

void MainFrame::buildUi() {
}

void MainFrame::connectSignals() {
}
