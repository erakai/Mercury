#include "streamdisplay.hpp"
#include <QtCore/qbuffer.h>
#include <QtMultimedia/qaudiosink.h>
#include "config/mconfig.hpp"

StreamDisplay::StreamDisplay(
    QWidget *parent, function<bool(QImage &, QByteArray &)> get_next_frame)
    : QWidget(parent), get_next_frame(get_next_frame)
{
  current_fps = FPS;
  fps_timer = new QTimer(this);
  connect(fps_timer, &QTimer::timeout, this,
          &StreamDisplay::acquire_next_frame);

  QAudioFormat format;
  // Set up the format, eg.
  format.setSampleRate(44100);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Int16);

  // Audio:
  audio_sink = new QAudioSink(format, this);
  int bytes_per_s =
      (format.sampleRate() * format.channelCount() * format.bytesPerSample());
  audio_sink->setBufferSize(bytes_per_s * 10);
  audio_buffer = audio_sink->start();

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
  graphics_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Show the graphics view.
  graphics_view->show();
}

void StreamDisplay::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  graphics_view->setGeometry(0, 0, width(),
                             height()); // Force full expansion
  graphics_scene->setSceneRect(0, 0, graphics_view->width(),
                               graphics_view->height());
  video_item->setPos(0, 0);
  video_item->setSize(QSizeF(graphics_view->width(), graphics_view->height()));
}

void StreamDisplay::begin_playback()
{
  video_player->play();

  fps_timer->start(1000 / FPS);
}

void StreamDisplay::stop_playback()
{
  video_player->stop();
  audio_sink->stop();
  audio_buffer->close();
}

void StreamDisplay::set_new_fps(int new_fps)
{
  if (new_fps != current_fps)
  {
    current_fps = new_fps;
    FPS = new_fps;
    if (fps_timer->isActive())
      fps_timer->start(1000 / new_fps);
  }
}

void StreamDisplay::acquire_next_frame()
{
  // Hacky way to watch for FPS being updated
  if (current_fps != FPS)
    set_new_fps(FPS);

  if (get_next_frame(next_video_image, next_audio_frame))
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

    if (audio_buffer->isOpen() && audio_buffer->isWritable())
    {
      QMutexLocker locker(&audio_mutex);
      audio_buffer->write(next_audio_frame);
      audio_buffer->waitForBytesWritten(1000 / FPS);
    }
  }
}
