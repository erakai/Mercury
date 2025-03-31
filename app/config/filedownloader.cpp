#include "filedownloader.hpp"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QObject>
#include <QDebug>
#include <QEventLoop>

void mercury::download_file(const QString &url, const QString &fileName)
{
  QNetworkAccessManager manager;
  QNetworkRequest request(url);
  QNetworkReply *reply = manager.get(request);

  QEventLoop loop;

  QObject::connect(reply, &QNetworkReply::finished, [&]() {
    if (reply->error() == QNetworkReply::NoError) {
      QFile file(fileName);
      if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();
      } else {
        qWarning() << "Failed to open file for writing:" << fileName;
      }
    } else {
      qWarning() << "Download error:" << reply->errorString();
    }

    reply->deleteLater();
    manager.deleteLater();
    loop.quit();
  });

  loop.exec();
  return;
}
