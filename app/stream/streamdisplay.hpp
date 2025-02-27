#pragma once

#include <QAudioBuffer>
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
#include <QWidget>
#include <QtMultimedia/qaudiooutput.h>
#include <QtMultimedia/qmediaplayer.h>
#include <functional>
#include <logger.hpp>

using namespace std;

// TODO: This should instead be pulled from the settings somehow - Kris
// responsibility
constexpr int FPS = 30;

class StreamDisplay : public QWidget
{
  Q_OBJECT;

public:
  StreamDisplay(QWidget *parent, function<bool(QImage &)> get_next_video_frame,
                function<bool(QBuffer &)> get_next_audio_frame);

  // Begins drawing and requesting frames
  void begin_playback();

public slots:
  // Called FPS times a second
  void acquire_next_frame();

private:
  // This timer will emit a timeout signal every x milliseconds, which will tell
  // the display to try to acquire the next video/audio frame
  QTimer *fps_timer;

  // methods to ensure we know what to next display
  function<bool(QImage &)> get_next_video_frame;
  function<bool(QBuffer &)> get_next_audio_frame;
  QImage next_video_image;
  QBuffer next_audio_frame;

  // I think that audio will have to somehow be written to a QIODevice that a
  // QMediaPlayer reads from.
  // https://stackoverflow.com/questions/35365600/play-a-qaudiobuffer
  // https://stackoverflow.com/questions/4473608/how-to-play-sound-with-qt
  QBuffer *audio_buffer;
  QAudioSink *audio_sink;
  QFile sourceFile;

  QMediaPlayer *video_player;
  QVideoWidget *video_widget;
  QVideoSink *video_sink;
};
