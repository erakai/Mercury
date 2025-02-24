#include "mainwindow.hpp"
#include "hoststreamwindow.h"
#include "joinstreamwindow.h"
#include "settingswindow.h"
#include "grabwindowstest.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  // Make this class use a ui file (optional). To see examples of classes
  // that don't rely on ui files, open an example in Qt Creator (the screenshot
  // example is really good).
  ui.setupUi(this);
  grabwindowstest gwt;
  gwt.exec();
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
