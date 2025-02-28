#ifndef UTILS_H
#define UTILS_H

#include <QNetworkInterface>
#include <QObject>
#include <QString>
#include <QSettings>

class Utils : public QObject
{
Q_OBJECT // Ensure this is in the header, not in the .cpp file

    public : static Utils &
             instance()
  {
    static Utils instance;
    return instance;
  }

  QString getIpAddress() const { return ipAddress; }
  QString getDisplayName();
  int getDefaultClientUdpPort();
  int getDefaultHostUdpPort();
  int getDefaultHostTcpPort();

private:
  explicit Utils();
  ~Utils() = default;
  Utils(const Utils &) = delete;
  Utils &operator=(const Utils &) = delete;

  QString ipAddress;
  QSettings settings;
};

#endif
