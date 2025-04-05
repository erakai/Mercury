#include "mutestreambutton.hpp"
#include <QtCore/qtmetamacros.h>

MuteStreamButton::MuteStreamButton(QWidget *parent)
    : QCheckBox("Stream Muted"), _is_muted(false)
{
  muted_icon = QIcon::fromTheme("audio-volume-muted");
  unmuted_icon = QIcon::fromTheme("audio-volume-high");

  setIcon(unmuted_icon);
  setIconSize(QSize(24, 24));
  connect(this, &QCheckBox::checkStateChanged, this,
          &MuteStreamButton::toggle_mute);
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
    setIcon(muted_icon);
  }
  else
  {
    setIcon(unmuted_icon);
  }
  emit mute_status_changed(_is_muted);
}
