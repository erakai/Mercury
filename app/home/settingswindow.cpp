#include "settingswindow.h"
#include "toastnotification.h"
#include "ui_settingswindow.h"
#include "config/mconfig.hpp"
#include "utils.h"

#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QRandomGenerator>
#include <QtCore/qregularexpression.h>

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
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio1, 5);
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio2, 12);
  ui->hostStreamFramerateButtonGroup->setId(ui->streamFramerateRadio3, 24);
}

void SettingsWindow::set_up()
{
  // Grab all settings to update the ui
  QString app_dir = QCoreApplication::applicationDirPath();
  QSettings settings(app_dir + "/config/MercuryClientSettings.ini",
                     QSettings::IniFormat);

  // GENERAL SETTINGS SETUP

  ui->displayNameLineEdit->setText(mercury::get_alias(settings));
  ui->darkModeCheckBox->setChecked(mercury::get_dark_mode(settings));

  // CLIENT SETTINGS SETUP

  ui->defaultClientUdpPortLineEdit->setText(
      QString::number(mercury::get_defaultClientUdpPort(settings)));

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
  if (streamFramerateOption != 5 && streamFramerateOption != 12 &&
      streamFramerateOption != 24)
  {
    streamFramerateOption = 5;
  }
  ui->hostStreamFramerateButtonGroup->button(streamFramerateOption)
      ->setChecked(true);

  ui->maxViewerCountSpinBox->setValue(mercury::get_host_max_viewers(settings));

  ui->defaultHostUdpPortLineEdit->setText(
      QString::number(mercury::get_defaultHostUdpPort(settings)));
  ui->defaultHostTcpPortLineEdit->setText(
      QString::number(mercury::get_defaultHostTcpPort(settings)));

  ui->whitelist_checkbox->setChecked(mercury::get_whitelist_enabled(settings));
  ui->blacklist_checkbox->setChecked(mercury::get_blacklist_enabled(settings));

  QString whitelistText = mercury::get_whitelist(settings).join("\n");
  ui->whitelist_text->setText(whitelistText);
  QString blacklistText = mercury::get_blacklist(settings).join("\n");
  ui->blacklist_text->setText(blacklistText);
}

void SettingsWindow::on_applyButton_clicked()
{
  QString alias = ui->displayNameLineEdit->text();
  bool darkMode = ui->darkModeCheckBox->isChecked();
  int hostStreamResOption = ui->hostStreamResolutionButtonGroup->checkedId();
  int hostFramerateOption = ui->hostStreamFramerateButtonGroup->checkedId();
  int maxViewerCount = ui->maxViewerCountSpinBox->value();
  int defaultClientUdpPort = ui->defaultClientUdpPortLineEdit->text().toInt();
  int defaultHostTcpPort = ui->defaultHostTcpPortLineEdit->text().toInt();
  int defaultHostUdpPort = ui->defaultHostUdpPortLineEdit->text().toInt();

  bool blacklistEnabled = ui->blacklist_checkbox->isChecked();
  bool whitelistEnabled = ui->whitelist_checkbox->isChecked();
  QString blacklist = ui->blacklist_text->toPlainText();
  QString whitelist = ui->whitelist_text->toPlainText();
  blacklist.replace(QRegularExpression(";"), "");
  whitelist.replace(QRegularExpression(";"), "");
  blacklist.replace(QRegularExpression("\n"), ";");
  whitelist.replace(QRegularExpression("\n"), ";");

  if (alias.length() > 15)
  {
    alias = Utils::instance().getDisplayName();
    ToastNotification::showToast(this, "Invalid alias provided.", 3000,
                                 WARNING);
    return;
  }
  else if (alias.length() == 0)
  {
    // Small pool of adjectives and nouns
    QStringList adjectives = {"Swift",  "Brave", "Clever", "Mighty",
                              "Silent", "Loyal", "Fierce", "Witty"};
    QStringList nouns = {"Falcon", "Tiger", "Wolf",   "Panther",
                         "Eagle",  "Shark", "Dragon", "Phoenix"};

    // Select random adjective and noun
    QString randomAdjective =
        adjectives.at(QRandomGenerator::global()->bounded(adjectives.size()));
    QString randomNoun =
        nouns.at(QRandomGenerator::global()->bounded(nouns.size()));

    // Combine and ensure it's under 15 characters
    alias = randomAdjective + randomNoun;
    ui->displayNameLineEdit->setText(alias);
  }

  // notify other objects that the name should be updated
  emit aliasChanged(alias);

  mercury::save_all_settings(
      alias, darkMode, hostStreamResOption, hostFramerateOption, maxViewerCount,
      defaultClientUdpPort, defaultHostTcpPort, defaultHostUdpPort,
      blacklistEnabled, whitelistEnabled, blacklist, whitelist);

  ToastNotification::showToast(this, "All changes saved!");
}
