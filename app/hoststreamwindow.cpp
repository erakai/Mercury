#include "hoststreamwindow.h"
#include "stream/streamservice.hpp"
#include "stream/streamwindow.hpp"
#include "ui_hoststreamwindow.h"

#include <QClipboard>

HostStreamWindow::HostStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::HostStreamWindow)
{
  // Get the ip address here to display

  ui->setupUi(this);

  connect(ui->host_button, &QPushButton::released, this,
          &HostStreamWindow::on_host_button_clicked);
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

void HostStreamWindow::on_host_button_clicked()
{
  // This sets itself to delete on close, so no memory leak (I think)
  StreamWindow *w = new StreamWindow("test", std::make_shared<HostService>());
  w->show();

  parentWidget()->hide();
  this->close();
}
