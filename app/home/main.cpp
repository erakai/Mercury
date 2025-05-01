#include "mainwindow.hpp"
#include "config/mconfig.hpp"

#include <QApplication>
#include <QPushButton>
#include <QDialog>
#include <QDir>
#include <QTextEdit>
#include <QStandardPaths>
#include <QVBoxLayout>

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

  // Load settings
  QString app_dir = QCoreApplication::applicationDirPath();
  const QString &outputFilename = app_dir + "/config/MercuryClientSettings.ini";
  mercury_settings =
      std::make_shared<QSettings>(outputFilename, QSettings::IniFormat);

  // Confirm assets file
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

  // get and apply style sheets
  QString styleSheetFilename = app_dir + "/styles/light.qss";
  if (mercury::get_dark_mode(*mercury_settings))
  {
    styleSheetFilename = app_dir + "/styles/dark.qss";
  }
  QFile styleSheetFile(styleSheetFilename);
  if (styleSheetFile.open(QFile::ReadOnly))
  {
    QString styles = styleSheetFile.readAll();
    a.setStyleSheet(styles);
    // a.setStyleSheet(a.styleSheet() + "\nQPushButton {background-color: rgb(0,
    // 162, 64);border: 2px solid rgb(249, 220, 161);padding: 6px
    // 12px;border-radius: 12px;");
  }
  else
  {
    qWarning("Failed to load QSS stylesheet.");
  }

  qInfo("Initializing main window...");
  MainWindow w;
  w.show();

  return a.exec();
}
