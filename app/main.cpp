#include "mainwindow.hpp"
#include "mnet.hpp"
#include "configControl/mconfig.hpp"

#include <QApplication>
#include <iostream>

int main(int argc, char **argv)
{
  mercury::init_mercury_network();

  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  if (!(mercury::check_config_file_presence()))
  {

    std::cout << "Config file not found!" << std::endl;
    mercury::curl_default_config();

  }

  return a.exec();
}
