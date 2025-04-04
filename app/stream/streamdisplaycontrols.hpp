#ifndef STREAMDISPLAYCONTROLS_H
#define STREAMDISPLAYCONTROLS_H
#include "stream/volumecontrolwidget.hpp"
#include <QWidget>
#include <QPushButton>

class StreamDisplayControls : public QWidget
{
  Q_OBJECT;

private:
  QPushButton *fullscreenButton;
  VolumeControlWidget *volume_control;

public:
  StreamDisplayControls(QWidget *parent);

signals:
  void fullScreenButtonPressed();
  void volume_changed(int volume);
};

#endif // STREAMDISPLAYCONTROLS_H
