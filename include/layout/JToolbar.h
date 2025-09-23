#pragma once

#include <QToolBar>

class JToolbar: public QToolBar {
    Q_OBJECT
public:
    explicit JToolbar(QWidget* parent = nullptr);
};
