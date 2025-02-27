#include "joinstreamwindow.h"
#include "../stream/streamservice.hpp"
#include "../stream/streamwindow.hpp"
#include "ui_joinstreamwindow.h"

JoinStreamWindow::JoinStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::JoinStreamWindow)
{
  ui->setupUi(this);
}

JoinStreamWindow::~JoinStreamWindow()
{
  qDebug() << "join stream window destructor was called";
  delete ui;
}

void JoinStreamWindow::on_cancelButton_clicked()
{
  this->close();
}

void JoinStreamWindow::on_joinButton_clicked()
{
  std::shared_ptr<ClientService> serv = std::make_shared<ClientService>();
  StreamWindow *w =
      new StreamWindow(ui->displayNameTextEdit->text().toStdString(), serv);
  w->show();

  parentWidget()
      ->hide(); // I think we want home to show back up after stream is closed
  this->close();
}
