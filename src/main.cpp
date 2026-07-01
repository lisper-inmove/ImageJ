#include "frames/main_frame.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainFrame w;
  w.show();
  return a.exec();
}
