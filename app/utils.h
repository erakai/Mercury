#ifndef UTILS_H
#define UTILS_H

#include <QNetworkInterface>
#include <QObject>
#include <QString>

class NetworkManager : public QObject
{
Q_OBJECT // Ensure this is in the header, not in the .cpp file

    public : static NetworkManager &
             instance()
  {
    static NetworkManager instance;
    return instance;
  }

  QString getIpAddress() const { return ipAddress; }

private:
  explicit NetworkManager();
  ~NetworkManager() = default;
  NetworkManager(const NetworkManager &) = delete;
  NetworkManager &operator=(const NetworkManager &) = delete;

  QString ipAddress;
};

#endif
