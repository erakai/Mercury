#include "hoststreamwindow.h"
#include "singleton/videomanager.h"
#include "stream/streamwindow.hpp"
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
  hide();

  spw = new StreamPreviewWindow();
  spw->show();
  spw->raise();          // for MacOS
  spw->activateWindow(); // for Windows

  connect(spw, &StreamPreviewWindow::closed, this,
          &HostStreamWindow::open_stream_window);
}

void HostStreamWindow::open_stream_window()
{
  delete (spw);

  if (!VideoManager::instance().mediaCaptureIsSet())
    return;

  std::string alias = ui->lineEdit_2->text().toStdString();

  std::shared_ptr<HostService> serv =
      std::make_shared<HostService>(alias, 23333, 32222);
  serv->stream_name = ui->streamNameLineEdit->text().toStdString();
  serv->server->start_server();
  // This sets itself to delete on close, so no memory leak (I think)
  StreamWindow *w = new StreamWindow(alias, serv);
  w->show();

  parentWidget()->hide();
  this->close();
}
