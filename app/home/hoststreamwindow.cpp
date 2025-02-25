#include "hoststreamwindow.h"
#include "../stream/streamwindow.hpp"
#include "toastnotification.h"
#include "ui_hoststreamwindow.h"
#include "utils.h"

#include <QClipboard>
#include <QHostAddress>
#include <QLabel>
#include <QNetworkInterface>

HostStreamWindow::HostStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::HostStreamWindow)
{
  // Get the ip address here to display

  ui->setupUi(this);

  QString ip = Utils::instance().getIpAddress();
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
  // Copy IP address to clipboard
  QApplication::clipboard()->setText(Utils::instance().getIpAddress());

  ToastNotification::showToast(this, "Copied IP address to clipboard!", 1500);
}

void HostStreamWindow::on_hostButton_clicked()
{
  // This sets itself to delete on close, so no memory leak (I think)
  std::shared_ptr<HostService> serv = std::make_shared<HostService>();
  serv->stream_name = ui->streamNameLineEdit->text().toStdString();
  StreamWindow *w =
      new StreamWindow(ui->lineEdit_2->text().toStdString(), serv);
  w->show();

  parentWidget()->hide();
  this->close();
}
