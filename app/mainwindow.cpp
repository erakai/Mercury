#include "mainwindow.hpp"
#include "hoststreamwindow.h"
#include "joinstreamwindow.h"
#include "settingswindow.h"
#include "networkmanager.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  // Make this class use a ui file (optional). To see examples of classes
  // that don't rely on ui files, open an example in Qt Creator (the screenshot
  // example is really good).
  ui.setupUi(this);

  QString ip = NetworkManager::instance().getIpAddress();
  ui.ip_address_label->setText(ip);
}

void MainWindow::on_settingsButton_clicked()
{
  SettingsWindow settingsWindow;
  settingsWindow.setModal(true);
  settingsWindow.exec();
}

void MainWindow::on_hostButton_clicked()
{
  HostStreamWindow hostStreamWindow;
  hostStreamWindow.setModal(true);
  hostStreamWindow.exec();
}

void MainWindow::on_joinButton_clicked()
{
  JoinStreamWindow joinStreamWindow;
  joinStreamWindow.setModal(true);
  joinStreamWindow.exec();
}
