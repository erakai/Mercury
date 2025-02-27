#include "settingswindow.h"
#include "toastnotification.h"
#include "ui_settingswindow.h"
#include "config/mconfig.hpp"

#include <QDebug>
#include <QSettings>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsWindow)
{
  ui->setupUi(this);
  this->set_button_ids();
  this->set_up();
}

SettingsWindow::~SettingsWindow()
{
  delete ui;
}

void SettingsWindow::on_closeButton_clicked()
{
  this->close();
}

void SettingsWindow::set_button_ids()
{ // A helper to set button group buttonIds for easy reference
  // Client Stream Resolution
  ui->clientStreamResolutionButtonGroup->setId(ui->clientStreamResolutionRadio1,
                                               360);
  ui->clientStreamResolutionButtonGroup->setId(ui->clientStreamResolutionRadio2,
                                               720);
  ui->clientStreamResolutionButtonGroup->setId(ui->clientStreamResolutionRadio3,
                                               1080);
  ui->clientStreamResolutionButtonGroup->setId(ui->clientStreamResolutionRadio4,
                                               1440);
  // Host Stream Resolution
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio1,
                                             360);
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio2,
                                             720);
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio3,
                                             1080);
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio4,
                                             1440);
  // Host framerate
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio1, 15);
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio2, 30);
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio3, 60);
}

void SettingsWindow::set_up()
{
  // Grab all settings to update the ui
  QSettings settings("../../config/MercuryClientSettings.ini",
                     QSettings::IniFormat);

  // GENERAL SETTINGS SETUP

  ui->displayNameLineEdit->setText(mercury::get_alias(settings));
  ui->darkModeCheckBox->setChecked(mercury::get_dark_mode(settings));

  // CLIENT SETTINGS SETUP

  int clientStreamResOption = mercury::get_client_stream_res(settings);

  if (clientStreamResOption != 360 && clientStreamResOption != 720 &&
      clientStreamResOption != 1080 && clientStreamResOption != 1440)
  {
    clientStreamResOption = 720;
  }
  ui->clientStreamResolutionButtonGroup->button(clientStreamResOption)
      ->setChecked(true);

  // HOST SETTINGS SETUP

  int hostStreamResOption = mercury::get_host_stream_res(settings);
  if (hostStreamResOption != 360 && hostStreamResOption != 720 &&
      hostStreamResOption != 1080 && hostStreamResOption != 1440)
  {
    hostStreamResOption = 720;
  }
  ui->hostStreamResolutionButtonGroup->button(hostStreamResOption)
      ->setChecked(true);

  int streamFramerateOption = mercury::get_host_stream_fps(settings);
  if (streamFramerateOption != 15 && streamFramerateOption != 30 &&
      streamFramerateOption != 60)
  {
    streamFramerateOption = 60;
  }
  ui->hostStreamFramerateButtonGroup->button(streamFramerateOption)
      ->setChecked(true);

  ui->maxViewerCountSpinBox->setValue(mercury::get_host_max_viewers(settings));
}

void SettingsWindow::on_applyButton_clicked()
{
  QString alias = ui->displayNameLineEdit->text();
  bool darkMode = ui->darkModeCheckBox->isChecked();
  int clientStreamResOption =
      ui->clientStreamResolutionButtonGroup->checkedId();
  int hostStreamResOption = ui->hostStreamResolutionButtonGroup->checkedId();
  int hostFramerateOption = ui->hostStreamFramerateButtonGroup->checkedId();
  int maxViewerCount = ui->maxViewerCountSpinBox->value();

  emit aliasChanged(alias);

  mercury::save_all_settings(alias, darkMode, clientStreamResOption,
                             hostStreamResOption, hostFramerateOption,
                             maxViewerCount);

  qDebug() << alias << darkMode << clientStreamResOption << hostStreamResOption
           << hostFramerateOption << maxViewerCount;

  ToastNotification::showToast(this, "All changes saved!");
}
