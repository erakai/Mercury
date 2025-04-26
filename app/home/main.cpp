#include "mainwindow.hpp"
#include "config/mconfig.hpp"

#include <QApplication>
#include <QPushButton>
#include <QDialog>
#include <QDir>
#include <QTextEdit>
#include <QStandardPaths>

int main(int argc, char **argv)
{
  qSetMessagePattern("%{time process} %{type} - %{message}");

  QApplication a(argc, argv);
  QCoreApplication::setApplicationName("Mercury");
  QCoreApplication::setApplicationVersion("0.0.1");

  // Generate config file
  if (!(mercury::check_config_file_presence()))
  {

    qWarning("Config file not found! Fixing...");
    mercury::curl_default_config();
  }

  // Confirm assets file
  QString app_dir = QCoreApplication::applicationDirPath();
  QDir assets_path(app_dir + "/assets");
  if (!assets_path.exists())
  {
    qCritical("Assets folder not found!");

    QDialog *modalDialog = new QDialog;
    modalDialog->setWindowTitle("Error");
    QTextEdit *textArea = new QTextEdit(modalDialog);
    textArea->setText(
        "Error: Unable to find assets folder. Please reinstall Mercury.");
    textArea->setReadOnly(true);
    QPushButton *closeButton = new QPushButton("Close", modalDialog);
    QAbstractButton::connect(closeButton, &QPushButton::clicked, modalDialog,
                             &QDialog::accept);

    QVBoxLayout *dialogLayout = new QVBoxLayout(modalDialog);
    dialogLayout->addWidget(textArea);
    dialogLayout->addWidget(closeButton);
    modalDialog->setLayout(dialogLayout);

    modalDialog->exec();
    return 1;
  }

  MainWindow w;
  w.show();

  return a.exec();
}
