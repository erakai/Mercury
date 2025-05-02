#ifndef STREAMDISPLAYCONTROLS_H
#define STREAMDISPLAYCONTROLS_H
#include "stream/qualitycontrolbutton.hpp"
#include "stream/volumecontrolwidget.hpp"
#include <QWidget>
#include <QPushButton>
#include <QtWidgets/qpushbutton.h>

class StreamDisplayControls : public QWidget
{
  Q_OBJECT;

private:
  QPushButton *fullscreenButton;
  QPushButton *playpauseButton;
  VolumeControlWidget *volume_control;

public:
  StreamDisplayControls(QWidget *parent);

  // Making this public to expose signal
  QualityControlButton *quality_control;

signals:
  void fullScreenButtonPressed();
  void volume_changed(int volume);
};

#endif // STREAMDISPLAYCONTROLS_H
