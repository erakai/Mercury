#include "mainwindow.hpp"
#include "mnet.hpp"
#include "../configControl/mconfig.hpp"
#include "../streampreview/streampreviewwindow.h"

#include <QApplication>
#include <iostream>

int main(int argc, char **argv)
{
  mercury::init_mercury_network();

  QApplication a(argc, argv);
  QCoreApplication::setApplicationName("Mercury");
  QCoreApplication::setApplicationVersion("0.0.1");

  MainWindow w;
  w.show();

  StreamPreviewWindow spw;
  spw.show();

  if (!(mercury::check_config_file_presence()))
  {

    std::cout << "Config file not found!" << std::endl;
    mercury::curl_default_config();
  }

  mercury::change_host_settings();

  return a.exec();
}
