#include "toastnotification.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>

void ToastNotification::showToast(QWidget *parent, const QString &text,
                                  int duration, ToastType toastType)
{
  ToastNotification *toast = new ToastNotification(parent);
  switch (toastType)
  {
  case WARNING:
    toast->setStyleSheet(
        toast->styleSheet().append("color: rgb(227, 208, 113);"));
    break;
  case ERROR:
    toast->setStyleSheet(toast->styleSheet().append("color: red;"));
    break;
  default:
    toast->setStyleSheet(toast->styleSheet().append("color: white;"));
  }
  toast->setText(text);
  toast->raise();
  toast->show();

  // Center at the bottom of the parent
  int x = (parent->width() - toast->width()) / 2;
  int y = (parent->height() - toast->height()) / 2;
  toast->move(x, y);

  // Start fade-out animation
  toast->startFadeOut(duration);
}

ToastNotification::ToastNotification(QWidget *parent) : QLabel(parent)
{
  setAlignment(Qt::AlignCenter);
  setStyleSheet("background-color: rgba(192, 192, 192, 255);"
                "font-weight: bold;"
                "padding: 10px;"
                "border-radius: 10px;");
  setFixedSize(250, 50);
  setAttribute(Qt::WA_TransparentForMouseEvents);
}

void ToastNotification::startFadeOut(int duration)
{
  QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(this);
  setGraphicsEffect(opacityEffect);

  QPropertyAnimation *fadeOut =
      new QPropertyAnimation(opacityEffect, "opacity");
  fadeOut->setDuration(duration);
  fadeOut->setStartValue(1.0);
  fadeOut->setEndValue(0.0);

  connect(fadeOut, &QPropertyAnimation::finished, this,
          [this, opacityEffect]()
          {
            opacityEffect->deleteLater();
            deleteLater(); // Automatically cleans up itself
          });

  fadeOut->start();
}
