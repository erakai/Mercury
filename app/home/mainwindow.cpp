#include "mainwindow.hpp"
#include "hoststreamwindow.h"
#include "joinstreamwindow.h"
#include "settingswindow.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  // Make this class use a ui file (optional). To see examples of classes
  // that don't rely on ui files, open an example in Qt Creator (the screenshot
  // example is really good).
  ui.setupUi(this);

  ui.settingsButton->setText(Utils::instance().getDisplayName());
}

void MainWindow::on_settingsButton_clicked()
{
  SettingsWindow settingsWindow(this);
  settingsWindow.setModal(true);
  settingsWindow.exec();
}

void MainWindow::on_hostButton_clicked()
{
  HostStreamWindow hostStreamWindow(this);
  hostStreamWindow.setModal(true);
  hostStreamWindow.exec();
}

void MainWindow::on_joinButton_clicked()
{
  JoinStreamWindow joinStreamWindow(this);
  joinStreamWindow.setModal(true);
  joinStreamWindow.exec();
}
