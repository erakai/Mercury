#ifndef MAPI_HPP
#define MAPI_HPP

#include <QString>

namespace mercury
{
void add_public_stream(QString streamName, int hostTCP, QString streamIP);

void delete_public_stream(QString streamIP);

void fetch_public_streams();
} // namespace mercury

#endif // MAPI_HPP
