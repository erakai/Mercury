#include "mutestreambutton.hpp"
#include <QtCore/qtmetamacros.h>

MuteStreamButton::MuteStreamButton(QWidget *parent)
    : QWidget(parent), _is_muted(false)
{
  mute_button = new QPushButton("Mute Stream");
  muted_icon = QIcon::fromTheme("audio-volume-muted");
  unmuted_icon = QIcon::fromTheme("audio-volume-high");
  ;

  mute_button->setIcon(unmuted_icon);
  mute_button->setIconSize(QSize(24, 24));
  connect(mute_button, &QPushButton::clicked, this,
          &MuteStreamButton::toggle_mute);

  auto *layout = new QHBoxLayout(this);
  layout->addWidget(mute_button);
  setLayout(layout);
}

bool MuteStreamButton::is_muted()
{
  return _is_muted;
}

void MuteStreamButton::toggle_mute()
{
  _is_muted = !_is_muted;

  if (_is_muted)
  {
    mute_button->setText("Unmute Stream");
    mute_button->setIcon(muted_icon);
  }
  else
  {
    mute_button->setText("Mute Stream");
    mute_button->setIcon(unmuted_icon);
  }
  emit mute_status_changed(_is_muted);
}
