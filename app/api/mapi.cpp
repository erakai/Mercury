#include "mapi.hpp"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QEventLoop>

QString BASE_URL = "http://localhost:8000";

QNetworkAccessManager *networkManager()
{
  static QNetworkAccessManager manager;
  return &manager;
}

void mercury::add_public_stream(QString streamName, int hostTCP,
                                QString streamIP, QByteArray thumbnail)
{
  QUrl requestURL(BASE_URL + "/addstream");

  QString thumbnailBase64 = QString::fromUtf8(thumbnail.toBase64());
  QJsonObject json;
  json["streamName"] = streamName;
  json["hostTCP"] = hostTCP;
  json["streamIP"] = streamIP;
  json["thumbnail"] = thumbnailBase64;

  QJsonDocument doc(json);
  QByteArray payload = doc.toJson();

  QNetworkRequest request(requestURL);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QNetworkReply *reply = networkManager()->post(request, payload);
  QObject::connect(reply, &QNetworkReply::finished,
                   [reply]() { reply->deleteLater(); });
}

void mercury::delete_public_stream(QString streamIP)
{
  QUrl requestURL(BASE_URL + "/deletestream");

  QJsonObject json;
  json["streamIP"] = streamIP;

  QJsonDocument doc(json);
  QByteArray payload = doc.toJson();

  QNetworkRequest request(requestURL);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QNetworkReply *reply = networkManager()->post(request, payload);
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  reply->deleteLater();
}

QJsonArray mercury::fetch_public_streams()
{
  QUrl url(BASE_URL + "/fetchstreams");
  QNetworkRequest request(url);

  QNetworkReply *reply = networkManager()->get(request);

  QEventLoop loop;

  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QJsonArray streamList;

  if (reply->error())
  {
    qDebug() << "Request failed:" << reply->errorString();
    reply->deleteLater();
    return streamList;
  }

  QByteArray data = reply->readAll();
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

  if (parseError.error != QJsonParseError::NoError || !doc.isArray())
  {
    qDebug() << "JSON parse error:" << parseError.errorString();
  }
  else
  {
    streamList = doc.array();
  }

  reply->deleteLater();
  return streamList;
}

QByteArray qpixmap_to_bytearray(const QPixmap &pixmap)
{
  QByteArray byteArray;
  QBuffer buffer(&byteArray);
  buffer.open(QIODevice::WriteOnly);
  pixmap.save(&buffer, "PNG");
  return byteArray;
}
