#include "mainwindow.hpp"
#include "mnet.hpp"
#include "streampreviewwindow.h"

#include <QApplication>

int main(int argc, char **argv)
{
  mercury::init_mercury_network();

  QApplication a(argc, argv);

  StreamPreviewWindow spw;
  spw.show();

  MainWindow w;
  w.show();

  return a.exec();
}
