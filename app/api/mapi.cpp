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
                                QString streamIP)
{
  QUrl requestURL(BASE_URL + "/addstream");

  QJsonObject json;
  json["streamName"] = streamName;
  json["hostTCP"] = hostTCP;
  json["streamIP"] = streamIP;

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
  QObject::connect(reply, &QNetworkReply::finished,
                   [reply]() { reply->deleteLater(); });
}

QJsonArray mercury::fetch_public_streams()
{
  QUrl url(BASE_URL + "/fetchstreams");
  QNetworkRequest request(url);

  QNetworkReply *reply = networkManager()->get(request);

  QEventLoop loop;

  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QJsonArray result;

  if (reply->error())
  {
    qDebug() << "Request failed:" << reply->errorString();
    reply->deleteLater();
    return result;
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
    result = doc.array();
  }

  reply->deleteLater();
  return result;
}
