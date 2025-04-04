#ifndef VOLUMECONTROLWIDGET_HPP
#define VOLUMECONTROLWIDGET_HPP

#include <QWidget>
#include <QSlider>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyle>

class VolumeControlWidget : public QWidget
{
  Q_OBJECT

public:
  explicit VolumeControlWidget(QWidget *parent = nullptr);

signals:
  void volume_changed(int volume);

private slots:
  void toggle_mute();
  void on_volume_changed(int value);

private:
  QPushButton *mute_button;
  QSlider *volume_slider;
  bool is_muted;
  int last_volume;
};

#endif // VOLUMECONTROLWIDGET_HPP
