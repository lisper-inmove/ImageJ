#include "frames/RightSidebar.h"

#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtMath>

RightSidebar::RightSidebar(QWidget* parent)
    : QWidget(parent) {
    buildUi();
    setFixedWidth(300);
}

void RightSidebar::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);

    tabs_ = new QTabWidget(this);
    root->addWidget(tabs_);

    // ===== Tab 1: 图片信息 =====
    auto* infoPage = new QWidget;
    auto* infoLay  = new QFormLayout(infoPage);
    lbPath_   = new QLabel(tr("-"));
    lbPath_->setWordWrap(true);
    lbSize_   = new QLabel(tr("-"));
    lbScale_  = new QLabel(tr("-"));
    lbOffset_ = new QLabel(tr("-"));
    lbMouse_  = new QLabel(tr("-"));  // 鼠标坐标
    infoLay->addRow(tr("路径:"),       lbPath_);
    infoLay->addRow(tr("尺寸:"),       lbSize_);
    infoLay->addRow(tr("显示比例:"),   lbScale_);
    infoLay->addRow(tr("偏移:"),       lbOffset_);
    infoLay->addRow(tr("鼠标:"),       lbMouse_);
    tabs_->addTab(infoPage, tr("图片信息"));

    // ===== Tab 2: 可操作选项 =====
    auto* opsPage = new QWidget;
    auto* opsLay  = new QVBoxLayout(opsPage);
    btnOpen_   = new QPushButton(tr("打开图片"));
    btnFit_    = new QPushButton(tr("适应窗口"));
    btnReset_  = new QPushButton(tr("重置视图"));
    btnActual_ = new QPushButton(tr("100% 像素"));
    btnHist_   = new QPushButton(tr("灰度直方图"));
    btnHist_->setObjectName("btnHist_");
    opsLay->addWidget(btnOpen_);
    opsLay->addWidget(btnFit_);
    opsLay->addWidget(btnReset_);
    opsLay->addWidget(btnActual_);
    opsLay->addSpacing(12);
    opsLay->addWidget(btnHist_);
    opsLay->addStretch(1);
    tabs_->addTab(opsPage, tr("可操作选项"));

    // 转发按钮为信号
    connect(btnOpen_,  &QPushButton::clicked, this, &RightSidebar::openImageRequested);
    connect(btnFit_,   &QPushButton::clicked, this, &RightSidebar::fitToWindowRequested);
    connect(btnReset_, &QPushButton::clicked, this, &RightSidebar::resetViewRequested);
    connect(btnActual_,&QPushButton::clicked, this, &RightSidebar::zoomActualPixelsRequested);
    connect(btnHist_,  &QPushButton::clicked, this, &RightSidebar::histogramRequested);
}

void RightSidebar::setInfo(const QString& path, const QSize& size) {
    lbPath_->setText(path.isEmpty() ? tr("-") : path);
    lbSize_->setText(QString("%1 x %2").arg(size.width()).arg(size.height()));
}

void RightSidebar::setView(const QPointF& offset, double scale) {
    lbScale_->setText(QString::number(scale * 100.0, 'f', 1) + "%");
    lbOffset_->setText(QString("(%1, %2)")
                           .arg(int(std::round(offset.x())))
                           .arg(int(std::round(offset.y()))));
}

void RightSidebar::setMouse(const QPoint& widgetPos, const QPointF& imagePos, bool inside) {
    const QString w = QString("win:(%1,%2)").arg(widgetPos.x()).arg(widgetPos.y());
    QString i;
    i = QString("\nimg:(%1,%2)").arg(int(std::floor(imagePos.x()))).arg(int(std::floor(imagePos.y())));
    lbMouse_->setText(w + i);
}
