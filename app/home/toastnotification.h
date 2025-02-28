#ifndef TOASTNOTIFICATION_H
#define TOASTNOTIFICATION_H

#include <QLabel>
#include <QObject>
#include <QWidget>

enum ToastType
{
  NOTICE,
  WARNING,
  ERROR
};

class ToastNotification : public QLabel
{
  Q_OBJECT
public:
  static void showToast(QWidget *parent, const QString &text,
                        int duration = 1500, ToastType toastType = NOTICE);

private:
  explicit ToastNotification(QWidget *parent);
  void startFadeOut(int duration);
};

#endif // TOASTNOTIFICATION_H
