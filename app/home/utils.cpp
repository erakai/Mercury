#include "utils.h"
#include "config/mconfig.hpp"

#include <QCoreApplication>

Utils::Utils()
    : settings(QCoreApplication::applicationDirPath() +
                   "/config/MercuryClientSettings.ini",
               QSettings::IniFormat)
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

QString Utils::getDisplayName()
{
  return mercury::get_alias(settings);
}

int Utils::getDefaultClientUdpPort()
{
  return mercury::get_defaultClientUdpPort(settings);
}

int Utils::getDefaultHostUdpPort()
{
  return mercury::get_defaultHostUdpPort(settings);
}

int Utils::getDefaultHostTcpPort()
{
  return mercury::get_defaultHostTcpPort(settings);
}
