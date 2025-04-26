#include "mainwindow.hpp"
#include "hoststreamwindow.h"
#include "joinstreamwindow.h"
#include "settingswindow.h"
#include "streambrowser/streambrowser.h"
#include "utils.h"
#include "../api/mapi.hpp"

#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  // Make this class use a ui file (optional). To see examples of classes
  // that don't rely on ui files, open an example in Qt Creator (the screenshot
  // example is really good).
  ui.setupUi(this);
  QString app_dir = QCoreApplication::applicationDirPath();
  QString file(app_dir + "/assets/MercuryLogo.png");
  QPixmap pix(file);
  ui.label->setPixmap(pix);

  ui.settingsButton->setText(Utils::instance().getDisplayName());
}

void MainWindow::updateSettingsButton(const QString &alias)
{
  ui.settingsButton->setText(alias);
}

void MainWindow::on_streamBrowserButton_clicked()
{
  QJsonArray publicStreamArray = mercury::fetch_public_streams();

  QStringList streamNames;
  QList<int> hostTCPs;
  QStringList streamIPs;

  for (const QJsonValue &stream : publicStreamArray)
  {
    if (!stream.isObject())
      continue;

    QJsonObject streamObject = stream.toObject();
    QString streamName = streamObject["streamName"].toString();
    int hostTCP = streamObject["hostTCP"].toInt();
    QString streamIP = streamObject["streamIP"].toString();

    streamNames.append(streamName);
    hostTCPs.append(hostTCP);
    streamIPs.append(streamIP);
  }

  StreamBrowser *streamBrowser =
      new StreamBrowser(this, streamNames, hostTCPs, streamIPs);
  streamBrowser->setAttribute(Qt::WA_DeleteOnClose);
  streamBrowser->setWindowTitle("Stream Browser");
  streamBrowser->resize(1440, 900);
  streamBrowser->setModal(false);
  streamBrowser->open();
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
