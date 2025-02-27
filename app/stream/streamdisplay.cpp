#include "streamdisplay.hpp"
#include <QtMultimedia/qaudiosink.h>

StreamDisplay::StreamDisplay(QWidget *parent,
                             function<bool(QImage &)> get_next_video_frame,
                             function<bool(QBuffer &)> get_next_audio_frame)
    : QWidget(parent), get_next_video_frame(get_next_video_frame),
      get_next_audio_frame(get_next_audio_frame)
{
  // PLACEHOLDER ==========================
  QPalette pal = QPalette();
  pal.setColor(QPalette::Window, Qt::blue);
  setAutoFillBackground(true);
  setPalette(pal);
  // END PLACEHOLDER ==========================

  // TODO: Add Audio

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
  sourceFile.setFileName("assets/fart.raw");
  sourceFile.open(QIODevice::ReadOnly);
  audio_sink->start(&sourceFile);
  // AUDIO TESTING END ====

  video_player = new QMediaPlayer(this);

  video_widget = new QVideoWidget(this);
  video_sink = video_widget->videoSink();
  video_player->setVideoOutput(video_sink);

  video_widget->setMinimumWidth(1280); // TODO: No idea if this right
  video_widget->setMinimumHeight(720);

  video_widget->show();

  begin_playback();
}

void StreamDisplay::begin_playback()
{
  video_player->play();

  fps_timer->start(1000 / 30.0);
}

void StreamDisplay::acquire_next_frame()
{
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
      log("Invalid Frame", ll::ERROR);
    }
  }
}
