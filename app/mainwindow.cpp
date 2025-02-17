#include "mainwindow.hpp"
#include "settingswindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  // Make this class use a ui file (optional). To see examples of classes
  // that don't rely on ui files, open an example in Qt Creator (the screenshot
  // example is really good).
  ui.setupUi(this);
}

void MainWindow::on_settingsButton_clicked()
{
  SettingsWindow settingsWindow;
  settingsWindow.setModal(true);
  settingsWindow.exec();
}
