#ifndef MAPI_HPP
#define MAPI_HPP

#include <QString>
#include <QJsonArray>
#include <QPixmap>
#include <QBuffer>
#include <QByteArray>

namespace mercury
{
void add_public_stream(QString streamName, int hostTCP, QString streamIP,
                       QByteArray thumbnail);

void delete_public_stream(QString streamIP);

QJsonArray fetch_public_streams();

QByteArray qpixmap_to_bytearray(const QPixmap &pixmap);
} // namespace mercury
#endif // MAPI_HPP
