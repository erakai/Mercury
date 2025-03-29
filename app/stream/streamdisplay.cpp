#include "streamdisplay.hpp"
#include <QtMultimedia/qaudiosink.h>
#include "config/mconfig.hpp"

StreamDisplay::StreamDisplay(QWidget *parent,
                             function<bool(QImage &)> get_next_video_frame,
                             function<bool(QBuffer &)> get_next_audio_frame)
    : QWidget(parent), get_next_video_frame(get_next_video_frame),
      get_next_audio_frame(get_next_audio_frame)
{
  // TODO: Add Audio

  current_fps = FPS;
  fps_timer = new QTimer(this);
  connect(fps_timer, &QTimer::timeout, this,
          &StreamDisplay::acquire_next_frame);

  // AUDIO TESTING ===
  QAudioFormat format;
  // Set up the format, eg.
  format.setSampleRate(44100);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Int16);

  audio_sink = new QAudioSink(format, this);
  /*
  sourceFile.setFileName("assets/fart.raw");
  sourceFile.open(QIODevice::ReadOnly);
  audio_sink->start(&sourceFile);
  */
  // AUDIO TESTING END ====

  video_player = new QMediaPlayer(this);

  video_widget = new QVideoWidget(this);
  video_sink = video_widget->videoSink();

  video_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  video_player->setVideoOutput(video_sink);

  video_widget->show();
}

void StreamDisplay::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  video_widget->setGeometry(0, 0, width(), height()); // Force full expansion
}

void StreamDisplay::begin_playback()
{
  video_player->play();

  fps_timer->start(1000 / FPS);
}

void StreamDisplay::set_new_fps(int new_fps)
{
  current_fps = new_fps;
  fps_timer->start(1000 / new_fps);
  FPS = new_fps;
}

void StreamDisplay::acquire_next_frame()
{
  // Hacky way to watch for FPS being updated
  if (current_fps != FPS)
  {
    set_new_fps(FPS);
  }

  if (get_next_audio_frame(next_audio_frame))
  {
    // somehow add audio frame to audio buffer?
  }

  if (get_next_video_frame(next_video_image))
  {
    QVideoFrame frame(next_video_image);
    if (frame.isValid())
    {
      video_sink->setVideoFrame(frame);
    }
    else
    {
      qCritical("Invalid VideoFrame received.");
    }
  }
}
