#ifndef TOASTNOTIFICATION_H
#define TOASTNOTIFICATION_H

#include <QLabel>
#include <QObject>
#include <QWidget>

class ToastNotification : public QLabel
{
  Q_OBJECT
public:
  static void showToast(QWidget *parent, const QString &text,
                        int duration = 1500);

private:
  explicit ToastNotification(QWidget *parent);
  void startFadeOut(int duration);
};

#endif // TOASTNOTIFICATION_H
