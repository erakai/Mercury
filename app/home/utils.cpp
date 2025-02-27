#include "utils.h"
#include "configcontrol/mconfig.hpp"

Utils::Utils()
    : settings("../../config/MercuryClientSettings.ini", QSettings::IniFormat)
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
