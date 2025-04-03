#ifndef STREAMDISPLAYCONTROLS_H
#define STREAMDISPLAYCONTROLS_H
#include <QWidget>
#include <QPushButton>

class StreamDisplayControls : public QWidget
{
  Q_OBJECT;

private:
  QPushButton *fullscreenButton;

public:
  StreamDisplayControls(QWidget *parent);

signals:
  void fullScreenButtonPressed();
};

#endif // STREAMDISPLAYCONTROLS_H
