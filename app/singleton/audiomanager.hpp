#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QCoreApplication>
#include <QAudioSource>
#include <QAudioBuffer>
#include <QBuffer>
#include <QByteArray>
#include <QTimer>
#include <QDebug>
#include <QMutex>
#include <QtCore/qstringview.h>
#include <QtMultimedia/qaudiodevice.h>

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

  void set_audio_device(QAudioDevice &audio_device);

  QByteArray get_lastmsec(int msec);

private slots:
  void on_audio_data_ready();

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
  QAudioSource *_audio_source;
  QByteArray _audio_bytearray;
  QBuffer _buffer_device;
  QMutex _mutex;
  QAudioDevice *_audio_device;
};

#endif // AUDIOMANAGER_H
