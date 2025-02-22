#include "streamdisplay.hpp"

StreamDisplay::StreamDisplay(
    QWidget *parent, function<bool(QPixmap &)> get_next_video_frame,
    function<bool(QAudioBuffer &)> get_next_audio_frame)
    : QWidget(parent), get_next_video_frame(get_next_video_frame),
      get_next_audio_frame(get_next_audio_frame)
{
  // PLACEHOLDER ==========================
  QPalette pal = QPalette();
  pal.setColor(QPalette::Window, Qt::blue);
  setAutoFillBackground(true);
  setPalette(pal);
  // END PLACEHOLDER ==========================

  fps_timer = new QTimer(this);
  connect(fps_timer, &QTimer::timeout, this,
          &StreamDisplay::acquire_next_frame);

  audio_buffer = new QBuffer();
  audio_buffer->open(QIODevice::ReadWrite);
}

void StreamDisplay::begin_playback()
{
  fps_timer->start(1000 / 30.0);
  audio->setSourceDevice(audio_buffer);
  audio->play();
}

void StreamDisplay::paintEvent(QPaintEvent *e)
{
  QPainter painter(this);
  QRectF target(0, 0, 1280, 720);
  QRectF source(0, 0, next_video_frame.width(), next_video_frame.height());
  painter.drawPixmap(target, next_video_frame, source);
}

void StreamDisplay::acquire_next_frame()
{
  get_next_audio_frame(next_audio_frame);
  // somehow add audio frame to audio buffer?

  if (get_next_video_frame(next_video_frame))
  {
    // Calls paintEvent()
    update();
  }
}
