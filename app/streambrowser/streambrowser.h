#ifndef STREAMBROWSER_H
#define STREAMBROWSER_H

#include <QDialog>

class StreamBrowser : public QDialog
{
  Q_OBJECT

public:
  explicit StreamBrowser(QWidget *parent = nullptr,
                         const QStringList streamNames = {},
                         const QList<int> hostTCPs = {},
                         const QStringList streamIPs = {},
                         const QList<QPixmap> thumbnails = {});

private:
  QWidget *createUI();
  bool eventFilter(QObject *watched, QEvent *event);
  QStringList streamNames;
  QList<int> hostTCPs;
  QStringList streamIPs;
  QList<QPixmap> thumbnails;
};

#endif // STREAMBROWSER_HPP
