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
  std::shared_ptr<HostService> serv = std::make_shared<HostService>();
  serv->stream_name = ui->stream_name_line->text().toStdString();
  StreamWindow *w = new StreamWindow("test", serv);
  w->show();

  parentWidget()->hide();
  this->close();
}
