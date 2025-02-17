#include "hoststreamwindow.h"
#include "ui_hoststreamwindow.h"

#include <QClipboard>

HostStreamWindow::HostStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::HostStreamWindow)
{
  // Get the ip address here to display


  ui->setupUi(this);
}

HostStreamWindow::~HostStreamWindow()
{
  delete ui;
}

void HostStreamWindow::on_pushButton_clicked()
{
  this->close();
}

void HostStreamWindow::on_pushButton_3_clicked()
{
  QClipboard *cb = QApplication::clipboard();
  cb->setText(QString("copied IP address"));
}

