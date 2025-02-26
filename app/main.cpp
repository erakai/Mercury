#include "mainwindow.hpp"

#include <QApplication>

int main(int argc, char **argv)
{
  QApplication a(argc, argv);
  QCoreApplication::setApplicationName("Mercury");
  QCoreApplication::setApplicationVersion("0.0.1");

  MainWindow w;
  w.show();

  return a.exec();
}
