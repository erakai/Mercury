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

void MainWindow::updateSettingsButton(const QString &alias)
{
  ui.settingsButton->setText(alias);
}

void MainWindow::on_settingsButton_clicked()
{
  SettingsWindow *settingsWindow = new SettingsWindow(this);
  connect(settingsWindow, &SettingsWindow::aliasChanged, this,
          &MainWindow::updateSettingsButton);
  settingsWindow->setModal(true);
  settingsWindow->setAttribute(Qt::WA_DeleteOnClose);
  settingsWindow->open();
}

void MainWindow::on_hostButton_clicked()
{
  HostStreamWindow *hostStreamWindow = new HostStreamWindow(this);
  hostStreamWindow->setModal(false);
  hostStreamWindow->setAttribute(Qt::WA_DeleteOnClose);
  hostStreamWindow->open();
}

void MainWindow::on_joinButton_clicked()
{
  JoinStreamWindow *joinStreamWindow = new JoinStreamWindow(this);
  joinStreamWindow->setModal(true);
  joinStreamWindow->setAttribute(Qt::WA_DeleteOnClose);
  joinStreamWindow->open();
}
