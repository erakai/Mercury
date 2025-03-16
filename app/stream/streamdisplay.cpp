#include "streamdisplay.hpp"
#include <QtMultimedia/qaudiosink.h>

StreamDisplay::StreamDisplay(QWidget *parent,
                             function<bool(QImage &)> get_next_video_frame,
                             function<bool(QBuffer &)> get_next_audio_frame)
    : QWidget(parent), get_next_video_frame(get_next_video_frame),
      get_next_audio_frame(get_next_audio_frame)
{
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
  //  sourceFile.setFileName("assets/fart.raw");
  //  sourceFile.open(QIODevice::ReadOnly);
  //  audio_sink->start(&sourceFile);
  // AUDIO TESTING END ====

  // Set up the media player.
  video_player = new QMediaPlayer(this);

  // Instead of a QVideoWidget, create a QGraphicsView and scene.
  graphics_view = new QGraphicsView(this);
  graphics_scene = new QGraphicsScene(this);
  graphics_view->setScene(graphics_scene);

  // Create the QGraphicsVideoItem and add it to the scene.
  video_item = new QGraphicsVideoItem();
  graphics_scene->addItem(video_item);

  // Retrieve the video sink from the video item.
  video_sink = video_item->videoSink();

  // Set the media player's video output to the video item.
  video_player->setVideoOutput(video_item);

  // Set the minimum size as before.
  graphics_view->setMinimumWidth(1280);
  graphics_view->setMinimumHeight(720);

  // Show the graphics view.
  graphics_view->show();
}

void StreamDisplay::begin_playback()
{
  video_player->play();

  fps_timer->start(1000 / FPS);
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
      qCritical("Invalid VideoFrame received.");
    }
  }
}
