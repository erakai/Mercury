#include "joinstreamwindow.h"
#include "stream/streamservice.hpp"
#include "stream/streamwindow.hpp"
#include "home/toastnotification.h"
#include "ui_joinstreamwindow.h"
#include "toastnotification.h"
#include "utils.h"

#include <QDebug>
#include <QCryptographicHash>

JoinStreamWindow::JoinStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::JoinStreamWindow)
{
  ui->setupUi(this);

  // Default ports (should move this to settings)
  ui->hostTcpPortLineEdit->setText(
      QString::number(Utils::instance().getDefaultHostTcpPort()));
  ui->clientUdpPortLineEdit->setText(
      QString::number(Utils::instance().getDefaultClientUdpPort()));
  ui->displayNameLineEdit->setText(Utils::instance().getDisplayName());
}

JoinStreamWindow::~JoinStreamWindow()
{
  delete ui;
}

void JoinStreamWindow::on_cancelButton_clicked()
{
  this->close();
}

void JoinStreamWindow::on_joinButton_clicked()
{
  std::string alias = ui->displayNameLineEdit->text().toStdString();
  std::string server_address = ui->ipAddressLineEdit->text().toStdString();
  QHostAddress address(server_address.c_str());
  std::string password = ui->passwordLineEdit->text().toStdString();

  QByteArray hashedPassword = QCryptographicHash::hash(
      QByteArray::fromStdString(password), QCryptographicHash::Sha256);

  quint16 hostTcpPort = ui->hostTcpPortLineEdit->text().toUShort();
  quint16 clientUdpPort = ui->clientUdpPortLineEdit->text().toUShort();
  std::shared_ptr<ClientService> serv = std::make_shared<ClientService>(alias);

  if (!serv->client->establish_connection(address, hostTcpPort, clientUdpPort,
                                          hashedPassword))
  {
    ToastNotification::showToast(this, "No server could be found on that IP.",
                                 4000);
    return;
  }

  StreamWindow *w = new StreamWindow(alias, serv);
  w->show();

  parentWidget()
      ->hide(); // I think we want home to show back up after stream is closed
  this->close();
}
