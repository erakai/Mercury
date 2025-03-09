#include "hoststreamwindow.h"
#include "singleton/videomanager.h"
#include "stream/streamwindow.hpp"
#include "toastnotification.h"
#include "ui_hoststreamwindow.h"
#include "utils.h"

#include <QtLogging>
#include <QClipboard>
#include <QDesktopServices>
#include <QHostAddress>
#include <QLabel>
#include <QNetworkInterface>
#include <QDebug>
#include <QCryptographicHash>

HostStreamWindow::HostStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::HostStreamWindow)
{
  // Get the ip address here to display

  ui->setupUi(this);
  connect(ui->tutorialButton, &QPushButton::clicked, this,
          &HostStreamWindow::tutorial_button_press);

  QString ip = Utils::instance().getIpAddress();
  ui->ipAddressButton->setText(ip);

  // Default ports (should be moved to settings)
  ui->displayNameLineEdit->setText(Utils::instance().getDisplayName());
  ui->tcpPortLineEdit->setText(
      QString::number(Utils::instance().getDefaultHostTcpPort()));
  ui->udpPortLineEdit->setText(
      QString::number(Utils::instance().getDefaultHostUdpPort()));

  ui->streamNameLineEdit->setFocus();
}

HostStreamWindow::~HostStreamWindow()
{
  delete ui;
}

void HostStreamWindow::on_hostButton_clicked()
{
  hide();

  spw = new StreamPreviewWindow();
  spw->show();
  spw->raise();          // for MacOS
  spw->activateWindow(); // for Windows

  connect(spw, &StreamPreviewWindow::closed, this,
          &HostStreamWindow::open_stream_window);
}

void HostStreamWindow::on_ipAddressButton_clicked()
{
  // Copy IP address to clipboard
  QApplication::clipboard()->setText(Utils::instance().getIpAddress());

  ToastNotification::showToast(this, "Copied IP address to clipboard!", 1500);
}

void HostStreamWindow::on_cancelButton_clicked()
{
  this->close();
}

void HostStreamWindow::open_stream_window()
{
  delete (spw);

  if (!VideoManager::instance().mediaCaptureIsSet())
    return;

  std::string alias = ui->displayNameLineEdit->text().toStdString();
  quint16 tcpPort = ui->tcpPortLineEdit->text().toUShort();
  quint16 udpPort = ui->udpPortLineEdit->text().toUShort();

  QByteArray hashedPassword = nullptr;
  std::string password = ui->passwordLineEdit->text().toStdString();

  qDebug() << password;
  if (!password.empty())
  {
    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(password);

    hashedPassword = hasher.result();
    qDebug() << hashedPassword.toStdString();
  }

  std::shared_ptr<HostService> serv =
      std::make_shared<HostService>(alias, tcpPort, udpPort, hashedPassword);
  serv->stream_name = ui->streamNameLineEdit->text().toStdString();
  serv->server->start_server();
  // This sets itself to delete on close, so no memory leak (I think)
  StreamWindow *w = new StreamWindow(alias, serv);
  w->show();

  parentWidget()->hide();
  this->close();
}

void HostStreamWindow::tutorial_button_press()
{
  if (!QDesktopServices::openUrl(QUrl(
          "https://github.com/erakai/Mercury/blob/main/docs/ServerHosting.md")))
  {
    qCritical("Unable to open tutorial URL.");
  }
}
