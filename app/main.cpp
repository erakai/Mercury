#include "mainwindow.hpp"
#include "mnet.hpp"
#include "windowmanager.h"

#include <QApplication>

int main(int argc, char **argv)
{
  mercury::init_mercury_network();

  WindowManager::instance().hello();

  // Retrieve and print the list of open window titles.
  WindowManager::instance().enumerateProcessesAndCaptureMainWindow();

  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
