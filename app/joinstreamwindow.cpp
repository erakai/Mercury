#include "joinstreamwindow.h"
#include "stream/streamservice.hpp"
#include "stream/streamwindow.hpp"
#include "ui_joinstreamwindow.h"

JoinStreamWindow::JoinStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::JoinStreamWindow)
{
  ui->setupUi(this);

  connect(ui->joinButton, &QPushButton::released, this,
          &JoinStreamWindow::on_join_button_clicked);
}

JoinStreamWindow::~JoinStreamWindow()
{
  delete ui;
}

void JoinStreamWindow::on_cancelButton_clicked()
{
  this->close();
}

void JoinStreamWindow::on_join_button_clicked()
{
  std::shared_ptr<ClientService> serv = std::make_shared<ClientService>();
  StreamWindow *w =
      new StreamWindow(ui->displayNameTextEdit->text().toStdString(), serv);
  w->show();

  parentWidget()->hide();
  this->close();
}
