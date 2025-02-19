#include "joinstreamwindow.h"
#include "stream/streamservice.hpp"
#include "stream/streamwindow.hpp"
#include "ui_joinstreamwindow.h"

JoinStreamWindow::JoinStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::JoinStreamWindow)
{
  ui->setupUi(this);

  connect(ui->joinButton, &QPushButton::released, this,
          &JoinStreamWindow::join_button_clicked);
}

JoinStreamWindow::~JoinStreamWindow()
{
  delete ui;
}

void JoinStreamWindow::on_cancelButton_clicked()
{
  this->close();
}

void JoinStreamWindow::join_button_clicked()
{
  StreamWindow *w = new StreamWindow("test", std::make_shared<ClientService>());
  w->show();

  parentWidget()->hide();
  this->close();
}
