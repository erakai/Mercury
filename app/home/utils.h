#ifndef UTILS_H
#define UTILS_H

#include <QNetworkInterface>
#include <QObject>
#include <QString>

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

private:
  explicit Utils();
  ~Utils() = default;
  Utils(const Utils &) = delete;
  Utils &operator=(const Utils &) = delete;

  QString ipAddress;
};

#endif
