#include "hoststreamwindow.h"
#include "singleton/videomanager.h"
#include "stream/streamwindow.hpp"
#include "toastnotification.h"
#include "ui_hoststreamwindow.h"
#include "utils.h"

#include <QtLogging>
#include <QTextEdit>
#include <QClipboard>
#include <QDesktopServices>
#include <QHostAddress>
#include <QLabel>
#include <QNetworkInterface>
#include <QDebug>
#include <QCryptographicHash>

HostStreamWindow::HostStreamWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::HostStreamWindow)
{
  // Get the ip address here to display

  ui->setupUi(this);
  connect(ui->tutorialButton, &QPushButton::clicked, this,
          &HostStreamWindow::tutorial_button_press);

  connect(ui->publicStream, &QCheckBox::toggled, this,
          &HostStreamWindow::public_stream_toggled);

  set_ethics_agreed_to(false);

  QString ip = Utils::instance().getIpAddress();
  ui->ipAddressButton->setText(ip);

  // Default ports (should be moved to settings)
  ui->displayNameLineEdit->setText(Utils::instance().getDisplayName());
  ui->tcpPortLineEdit->setText(
      QString::number(Utils::instance().getDefaultHostTcpPort()));
  ui->udpPortLineEdit->setText(
      QString::number(Utils::instance().getDefaultHostUdpPort()));

  ui->streamNameLineEdit->setFocus();

  // To make it look like password
  ui->passwordLineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
}

HostStreamWindow::~HostStreamWindow()
{
  delete ui;
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

void HostStreamWindow::public_stream_toggled()
{
  // If user tries to toggle public stream but has a password set, uncheck the
  // box
  if (ui->publicStream->checkState() == 2 &&
      ui->passwordLineEdit->text().toStdString().length() > 0)
  {
    ToastNotification::showToast(
        this, "Can't publicize password protected stream!", 2000);
    ui->publicStream->setChecked(false);
  }
  else if (ui->publicStream->checkState() == 2)
  {
    ui->passwordLineEdit->setEnabled(false);
  }
  else
  {
    ui->passwordLineEdit->setEnabled(true);
  }
}

void HostStreamWindow::on_ipAddressButton_clicked()
{
  // Copy IP address to clipboard
  QApplication::clipboard()->setText(Utils::instance().getIpAddress());

  ToastNotification::showToast(this, "Copied IP address to clipboard!", 1500);
}

void HostStreamWindow::on_cancelButton_clicked()
{
  this->close();
}

void HostStreamWindow::open_stream_window()
{
  delete (spw);

  if (!VideoManager::instance().mediaCaptureIsSet())
    return;

  std::string alias = ui->displayNameLineEdit->text().toStdString();
  quint16 tcpPort = ui->tcpPortLineEdit->text().toUShort();
  quint16 udpPort = ui->udpPortLineEdit->text().toUShort();

  QByteArray hashedPassword = nullptr;
  std::string password = ui->passwordLineEdit->text().toStdString();

  if (!password.empty())
  {
    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(password);

    hashedPassword = hasher.result();
    // qDebug() << hashedPassword.toStdString();
  }

  std::shared_ptr<HostService> serv =
      std::make_shared<HostService>(alias, tcpPort, udpPort, hashedPassword);
  serv->stream_name = ui->streamNameLineEdit->text().toStdString();
  serv->start_timestamp = QDateTime::currentSecsSinceEpoch();
  serv->reactions_enabled = ui->reactionsEnabledCheckBox->checkState() == 2;
  serv->ip_address = ui->ipAddressButton->text().toStdString();
  serv->public_stream = (ui->publicStream->checkState() == 2);
  serv->server->start_server();
  // This sets itself to delete on close, so no memory leak (I think)
  StreamWindow *w = new StreamWindow(alias, serv);
  if (w->set_up())
  {
    parentWidget()->hide();
    this->close();
  }
}

void HostStreamWindow::tutorial_button_press()
{
  if (!QDesktopServices::openUrl(QUrl(
          "https://github.com/erakai/Mercury/blob/main/docs/ServerHosting.md")))
  {
    qCritical("Unable to open tutorial URL.");
  }
}

void HostStreamWindow::on_ethics_button_clicked()
{
  QDialog *ethics_dialog = new QDialog(this);
  ethics_dialog->setStyleSheet(QString("QDialog {"
                                       "background-color: rgb(11, 17, 20);"
                                       "}"));

  ethics_dialog->setWindowTitle("Ethics Agreement");

  QTextEdit *text_area = new QTextEdit(ethics_dialog);
  text_area->setReadOnly(true);
  QString ethicsAgreement = R"(
        <h2 style='text-align: center;'>Mercury Ethics Agreement</h2>

        <p>By using or developing Mercury, you agree to the following terms:</p>

        <ol>
            <li><strong>Respect for Content:</strong> You will not upload, share, or stream any content that violates copyright laws, is illegal, or infringes on the rights of others.</li>
            <li><strong>Respect for Users:</strong> You agree to treat all other users with respect and refrain from using offensive, discriminatory, or harassing language or behavior.</li>
            <li><strong>Privacy and Confidentiality:</strong> You will respect the privacy of other users, refrain from sharing personal or sensitive information without consent, and follow all applicable privacy regulations.</li>
            <li><strong>Fair Use:</strong> You will not use the streaming service for any fraudulent, abusive, or harmful activities, including the distribution of malicious software or unauthorized content.</li>
            <li><strong>Compliance with Laws:</strong> You will comply with all applicable local, national, and international laws while using the application.</li>
        </ol>

        <p>By accepting these terms, you agree to use this service responsibly, ethically, and in accordance with all laws and regulations.</p>
        )";
  text_area->setHtml(ethicsAgreement);
  text_area->setFixedHeight(300);
  text_area->setFixedWidth(400);

  QPushButton *agree_button = new QPushButton("I Agree", ethics_dialog);
  connect(agree_button, &QPushButton::clicked, this,
          [=, this]()
          {
            set_ethics_agreed_to(true);
            ethics_dialog->accept();
          });

  QVBoxLayout *dialog_layout = new QVBoxLayout(ethics_dialog);
  dialog_layout->addWidget(text_area);
  dialog_layout->addWidget(agree_button);
  ethics_dialog->setLayout(dialog_layout);

  ethics_dialog->exec(); // This makes the dialog modal
}

void HostStreamWindow::set_ethics_agreed_to(bool agreed)
{
  ethics_agreed_to = agreed;
  ui->hostButton->setEnabled(ethics_agreed_to);
}
