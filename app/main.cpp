#include "mainwindow.hpp"
#include "mnet.hpp"

#include <QApplication>

int main(int argc, char **argv)
{
  mercury::init_mercury_network();

  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
