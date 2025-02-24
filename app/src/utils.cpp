#include "utils.h"

NetworkManager::NetworkManager()
{
  foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
  {
    if (address.protocol() == QAbstractSocket::IPv4Protocol &&
        !address.isLoopback())
    {
      ipAddress = address.toString();
      break;
    }
  }
}
