#include "hoststreamwindow.h"
#include "../stream/streamwindow.hpp"
#include "ui_hoststreamwindow.h"
#include "utils.h"

#include <QAbstractSocket>
#include <QClipboard>
#include <QGraphicsOpacityEffect>
#include <QHostAddress>
#include <QLabel>
#include <QNetworkInterface>
#include <QPropertyAnimation>
#include <QTimer>

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

  // Create the notification label if it doesn't exist
  QLabel *notificationLabel = new QLabel(this);
  notificationLabel->setAlignment(Qt::AlignCenter);
  notificationLabel->setStyleSheet(
      "background-color: rgba(0, 0, 0, 100);" // Semi-transparent black
      "color: white;"
      "font-weight: bold;"
      "padding: 10px;"
      "border-radius: 10px;");
  notificationLabel->setFixedSize(250, 50); // Fixed size for the notification
  notificationLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

  // Set text and position
  notificationLabel->setText("Copied IP address to clipboard!");
  notificationLabel->raise(); // Bring to the top of UI
  notificationLabel->show();

  // Center it at the bottom (adjust as needed)
  int x = (width() - notificationLabel->width()) / 2;
  int y = (height() - notificationLabel->height()) / 2;
  notificationLabel->move(x, y);

  // Apply opacity effect
  QGraphicsOpacityEffect *opacityEffect =
      new QGraphicsOpacityEffect(notificationLabel);
  notificationLabel->setGraphicsEffect(opacityEffect);

  // Create the fade-out animation
  QPropertyAnimation *fadeOut =
      new QPropertyAnimation(opacityEffect, "opacity");
  fadeOut->setDuration(1500); // 1 second fade-out
  fadeOut->setStartValue(1.0);
  fadeOut->setEndValue(0.0);

  connect(fadeOut, &QPropertyAnimation::finished, this,
          [notificationLabel, opacityEffect]()
          {
            opacityEffect->deleteLater();
            notificationLabel->deleteLater();
          });

  fadeOut->start();
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
