#include "volumecontrolwidget.hpp"

VolumeControlWidget::VolumeControlWidget(QWidget *parent)
    : QWidget(parent), is_muted(false), last_volume(50)
{
  mute_button = new QPushButton("", this);
  mute_button->setIcon(QIcon::fromTheme("audio-volume-high"));
  volume_slider = new QSlider(Qt::Horizontal, this);
  volume_slider->setRange(0, 100);
  volume_slider->setValue(last_volume);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(mute_button);
  layout->addWidget(volume_slider);
  setLayout(layout);

  connect(mute_button, &QPushButton::clicked, this,
          &VolumeControlWidget::toggle_mute);
  connect(volume_slider, &QSlider::valueChanged, this,
          &VolumeControlWidget::on_volume_changed);
}

void VolumeControlWidget::toggle_mute()
{
  is_muted = !is_muted;
  if (is_muted)
  {
    mute_button->setIcon(QIcon::fromTheme("audio-volume-muted"));
    last_volume = volume_slider->value();
    volume_slider->setValue(0);
    emit volume_changed(0);
  }
  else
  {
    mute_button->setIcon(QIcon::fromTheme("audio-volume-high"));
    volume_slider->setValue(last_volume);
    emit volume_changed(last_volume);
  }
}

void VolumeControlWidget::on_volume_changed(int value)
{
  if (value > 0)
  {
    is_muted = false;
    mute_button->setIcon(QIcon::fromTheme("audio-volume-high"));
  }
  else
  {
    mute_button->setIcon(QIcon::fromTheme("audio-volume-muted"));
  }
  emit volume_changed(value);
}
