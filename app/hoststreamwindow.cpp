#include "hoststreamwindow.h"
#include "ui_hoststreamwindow.h"
#include "networkmanager.h"

#include <QClipboard>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QNetworkInterface>

HostStreamWindow::HostStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::HostStreamWindow)
{
  // Get the ip address here to display

  ui->setupUi(this);


  QString ip = NetworkManager::instance().getIpAddress();
  ui->ip_address_button->setText(ip);
}

HostStreamWindow::~HostStreamWindow()
{
  delete ui;
}

void HostStreamWindow::on_pushButton_clicked()
{
  this->close();
}

void HostStreamWindow::on_ip_address_button_clicked()
{
  // set up clipboard to copy ip address into
  QClipboard *cb = QApplication::clipboard();

  cb->setText(NetworkManager::instance().getIpAddress());
}

