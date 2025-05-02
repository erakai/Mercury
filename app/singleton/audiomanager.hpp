#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QCoreApplication>
#include <QAudioSource>
#include <QAudioBufferOutput>
#include <QAudioBuffer>
#include <QBuffer>
#include <QByteArray>
#include <QTimer>
#include <QDebug>
#include <QMutex>
#include <QMediaPlayer>
#include <QtCore/qstringview.h>
#include <QtMultimedia/qaudiodevice.h>
#include <QtMultimedia/qaudiooutput.h>
#include <QtMultimedia/qmediaplayer.h>

class AudioManager : public QObject
{
  Q_OBJECT

public:
  static AudioManager &instance()
  {
    static AudioManager instance;
    return instance;
  }

  bool start_recording(qint32 buffer_size_ms);
  void stop_recording();

  bool is_recording();
  static bool is_audio_loud(QByteArray &audio, float threshold = 0.05);

  void set_audio_device(QAudioDevice &audio_device);
  void set_media_player(QMediaPlayer &media_player);
  bool has_media_player() { return _media_player != nullptr; }
  bool has_audio_device() { return _audio_device != nullptr; }

  QByteArray get_lastmsec(int msec);

private slots:
  void on_audio_data_ready();
  void on_audio_buffer_received(const QAudioBuffer &buffer);

private:
  AudioManager(QObject *parent = nullptr)
      : QObject(parent), _is_recording(false), _buffer_device(&_audio_bytearray)
  {
    setup_audio();
  }

  AudioManager(const AudioManager &) = delete;
  AudioManager &operator=(const AudioManager &) = delete;

  void setup_audio();

private:
  bool _is_recording;
  int _max_buffer_size;
  QAudioFormat _format;
  QAudioSource *_audio_source = nullptr;
  QByteArray _audio_bytearray;
  QBuffer _buffer_device;
  QMutex _mutex;
  QAudioDevice *_audio_device = nullptr;
  QMediaPlayer *_media_player = nullptr;
  QAudioBufferOutput *_audio_buffer_output = nullptr;
  QAudioOutput *_audio_output = nullptr;
};

#endif // AUDIOMANAGER_H
