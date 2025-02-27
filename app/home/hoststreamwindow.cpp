#include "hoststreamwindow.h"
#include "../stream/streamwindow.hpp"
#include "toastnotification.h"
#include "ui_hoststreamwindow.h"
#include "utils.h"

#include <QClipboard>
#include <QHostAddress>
#include <QLabel>
#include <QNetworkInterface>
#include <QDebug>

HostStreamWindow::HostStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::HostStreamWindow)
{
  // Get the ip address here to display

  ui->setupUi(this);

  QString ip = Utils::instance().getIpAddress();
  ui->ipAddressButton->setText(ip);
}

HostStreamWindow::~HostStreamWindow()
{
  delete ui;
}

void HostStreamWindow::on_hostButton_clicked()
{
  // This sets itself to delete on close, so no memory leak (I think)

  std::string alias = ui->displayNameLineEdit->text().toStdString();
  quint16 tcpPort = ui->tcpPortLineEdit->text().toUShort();
  quint16 udpPort = ui->udpPortLineEdit->text().toUShort();

  qDebug() << tcpPort << udpPort;

  std::shared_ptr<HostService> serv = std::make_shared<HostService>(alias);
  serv->stream_name = ui->streamNameLineEdit->text().toStdString();
  // serv->set_ports(tcpPort, udpPort);

  // join stream with provided ip, host tcp port, and own udp port
  // if (!serv->serv->start_server())
  // {
  //   ToastNotification::showToast(this, "No server could be found on that
  //   IP.",
  //                                4000);
  //   return;
  // }

  StreamWindow *w =
      new StreamWindow(ui->streamNameLineEdit->text().toStdString(), serv);
  w->show();

  parentWidget()->hide();
  this->close();
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
