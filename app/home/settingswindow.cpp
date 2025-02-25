#include "settingswindow.h"
#include "toastnotification.h"
#include "ui_settingswindow.h"

#include <QDebug>

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
                                               1);
  ui->clientStreamResolutionButtonGroup->setId(ui->clientStreamResolutionRadio2,
                                               2);
  ui->clientStreamResolutionButtonGroup->setId(ui->clientStreamResolutionRadio3,
                                               3);
  ui->clientStreamResolutionButtonGroup->setId(ui->clientStreamResolutionRadio4,
                                               4);
  // Host Stream Resolution
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio1, 1);
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio2, 2);
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio3, 3);
  ui->hostStreamResolutionButtonGroup->setId(ui->hostStreamResolutionRadio4, 4);
  // Host framerate
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio1, 1);
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio2, 2);
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio3, 3);
}

void SettingsWindow::set_up()
{ // Grab all settings to update the ui
  // Kris put settings here
  // GENERAL

  ui->displayNameLineEdit->setText("Hungry Banana"); // TODO @Kris set this
  ui->darkModeCheckBox->setChecked(true);            // TODO @Kris set this

  // CLIENT

  // TODO @Kris you can set this to whatever you like, just change the
  // corresponding values in the set_button_ids above AND in the error check
  // below
  int clientStreamResOption = 4;
  if (clientStreamResOption > 4 || clientStreamResOption < 1)
  {
    clientStreamResOption = 2;
  }
  ui->clientStreamResolutionButtonGroup->button(clientStreamResOption)
      ->setChecked(true);

  // HOST

  // TODO @Kris you can set this to whatever you like, just change the
  // corresponding values in the set_button_ids above AND in the error check
  // below
  int hostStreamResOption = 4;
  if (hostStreamResOption > 4 || hostStreamResOption < 1)
  {
    hostStreamResOption = 2;
  }
  ui->hostStreamResolutionButtonGroup->button(hostStreamResOption)
      ->setChecked(true);

  // TODO @Kris you can set this to whatever you like, just change the
  // corresponding values in the set_button_ids above AND in the error check
  // below
  int streamFramerateOption = 3;
  if (streamFramerateOption > 3 || streamFramerateOption < 1)
  {
    streamFramerateOption = 2;
  }
  ui->hostStreamFramerateButtonGroup->button(streamFramerateOption)
      ->setChecked(true);

  ui->maxViewerCountSpinBox->setValue(40); // TODO @Kris set this
}

void SettingsWindow::on_applyButton_clicked()
{
  // TODO @Kris write the current values to ini
  QString displayName = ui->displayNameLineEdit->text();
  bool darkMode = ui->darkModeCheckBox->isChecked();
  int clientStreamResOption =
      ui->clientStreamResolutionButtonGroup->checkedId();
  int hostStreamResOption = ui->hostStreamResolutionButtonGroup->checkedId();
  int hostFramerateOption = ui->hostStreamFramerateButtonGroup->checkedId();
  int maxViewerCount = ui->maxViewerCountSpinBox->value();

  qDebug() << displayName << darkMode << clientStreamResOption
           << hostStreamResOption << hostFramerateOption << maxViewerCount;

  // TODO @Kris if the end of writing is a call back, call this after
  ToastNotification::showToast(this, "All changes saved!");
}
