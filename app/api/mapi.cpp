#include "mapi.hpp"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <iostream>

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

void mercury::fetch_public_streams()
{
  // TODO: implement
}
