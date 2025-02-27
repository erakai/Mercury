#include "joinstreamwindow.h"
#include "stream/streamservice.hpp"
#include "stream/streamwindow.hpp"
#include "home/toastnotification.h"
#include "ui_joinstreamwindow.h"

JoinStreamWindow::JoinStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::JoinStreamWindow)
{
  ui->setupUi(this);
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
  std::string alias = ui->displayNameTextEdit->text().toStdString();
  std::string server_address = ui->ipAddressTextEdit->text().toStdString();
  QHostAddress address(server_address.c_str());

  std::shared_ptr<ClientService> serv = std::make_shared<ClientService>(alias);

  if (!serv->client->establish_connection(address, 23333, 48484))
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
