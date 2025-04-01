#pragma once

#include "home/mainwindow.hpp"

#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioSink>
#include <QBuffer>
#include <QFile>
#include <QMediaPlayer>
#include <QPainter>
#include <QTimer>
#include <QVideoFrame>
#include <QVideoSink>
#include <QVideoWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QWidget>
#include <QtMultimedia/qaudiooutput.h>
#include <QtMultimedia/qmediaplayer.h>
#include <functional>
#include <QtLogging>
#include <QMutex>

using namespace std;

class StreamDisplay : public QWidget
{
  Q_OBJECT;

public:
  StreamDisplay(QWidget *parent,
                function<bool(QImage &, QByteArray &)> get_next_frame);

  // Begins drawing and requesting frames
  void begin_playback();

public slots:
  // Called FPS times a second
  void acquire_next_frame();

  void set_new_fps(int fps);

protected:
  void resizeEvent(QResizeEvent *event) override;

private:
  // This timer will emit a timeout signal every x milliseconds, which will tell
  // the display to try to acquire the next video/audio frame
  QTimer *fps_timer;
  int current_fps;

  // methods to ensure we know what to next display
  function<bool(QImage &, QByteArray &)> get_next_frame;
  QImage next_video_image;
  QByteArray next_audio_frame;

  // I think that audio will have to somehow be written to a QIODevice that a
  // QMediaPlayer reads from.
  // https://stackoverflow.com/questions/35365600/play-a-qaudiobuffer
  // https://stackoverflow.com/questions/4473608/how-to-play-sound-with-qt
  QIODevice *audio_buffer;
  QAudioSink *audio_sink;
  QFile sourceFile;

  QMediaPlayer *video_player;
  QVideoSink *video_sink;

  QGraphicsView *graphics_view;
  QGraphicsScene *graphics_scene;
  QGraphicsVideoItem *video_item;

  QMutex audio_mutex;
};
