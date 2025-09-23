#include <layout/JToolbar.h>

JToolbar::JToolbar(QWidget* parent): QToolBar(parent) {
    setObjectName("MainToolbar");
    setWindowTitle("MainToolbar");

    addAction(QIcon::fromTheme("document-open"), "Open");
    addAction(QIcon::fromTheme("document-save"), "Save");
}
