#include "frames/right_sidebar.h"

#include <QLabel>
#include <QVBoxLayout>

RightSidebar::RightSidebar(QWidget *parent)
    : QWidget(parent), placeholder_label_(nullptr), main_layout_(nullptr) {
  buildUi();
}

void RightSidebar::buildUi() {
  main_layout_ = new QVBoxLayout(this);

  placeholder_label_ = new QLabel("工具选项", this);
  placeholder_label_->setAlignment(Qt::AlignCenter);

  main_layout_->addWidget(placeholder_label_);
  main_layout_->addStretch(); // 添加弹性空间

  setLayout(main_layout_);
}
