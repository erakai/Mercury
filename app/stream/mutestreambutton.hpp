#ifndef MUTE_BUTTON_WIDGET_H
#define MUTE_BUTTON_WIDGET_H

#include <QCheckBox>
#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QHBoxLayout>

class MuteStreamButton : public QCheckBox
{
  Q_OBJECT

public:
  explicit MuteStreamButton(QWidget *parent = nullptr);

  bool is_muted();

signals:
  void mute_status_changed(bool is_muted);

private slots:
  void toggle_mute();

private:
  bool _is_muted;
  QIcon muted_icon;
  QIcon unmuted_icon;
};

#endif // MUTE_BUTTON_WIDGET_H
