#include "singleton/audiomanager.hpp"
#include <QtCore/qlogging.h>
#include <QtCore/qsysinfo.h>
#include <QDateTime>
#include <QtGui/qpicture.h>
#include <QtMultimedia/qmediaplayer.h>

bool AudioManager::start_recording(qint32 buffer_size_ms)
{
  if (_is_recording)
  {
    qDebug() << "Issue with starting to record: Already recording!";
    return false;
  }

  if (_audio_device == nullptr && _media_player == nullptr)
  {
    qWarning()
        << "No audio device or media player is set, no audio will be played.";
    return false;
  }

  _max_buffer_size = buffer_size_ms;
  int bytes_per_ms = (_format.sampleRate() * _format.channelCount() *
                      _format.bytesPerSample()) /
                     1000;
  int max_buffer_size_bytes = _max_buffer_size * bytes_per_ms;
  _audio_bytearray.clear();
  _audio_bytearray.reserve(max_buffer_size_bytes);

  _buffer_device.open(QIODevice::ReadWrite);

  // Create QAudioSource instance
  if (has_audio_device())
  {
    _audio_source = new QAudioSource(*_audio_device, _format, this);
    _audio_source->start(&_buffer_device);

    if (_audio_source->state() == (QtAudio::ActiveState | QtAudio::IdleState))
    {
      connect(&_buffer_device, &QIODevice::readyRead, this,
              &AudioManager::on_audio_data_ready);
    }
    else
    {
      qWarning() << "Issue with starting audio source";
      return false;
    }
  }
  else if (has_media_player())
  {
    _audio_buffer_output = new QAudioBufferOutput(_format);
    _audio_output = new QAudioOutput();
    _audio_output->setMuted(true);

    _media_player->setAudioOutput(_audio_output);
    _media_player->setAudioBufferOutput(_audio_buffer_output);
    connect(_audio_buffer_output, &QAudioBufferOutput::audioBufferReceived,
            this, &AudioManager::on_audio_buffer_received);
  }
  else
  {
    _buffer_device.close();
    return false;
  }

  _is_recording = true;
  return true;
}

void AudioManager::stop_recording()
{
  if (!_is_recording)
    return;
  if (_audio_source != nullptr)
    _audio_source->stop();
  if (_buffer_device.isOpen())
    _buffer_device.close();
  if (_audio_buffer_output)
    delete _audio_buffer_output;

  _media_player = nullptr;
  _audio_device = nullptr;

  _is_recording = false;
}

bool AudioManager::is_recording()
{
  return _is_recording;
}

bool AudioManager::is_audio_loud(QByteArray &audio_data, float threshold)
{
  if (audio_data.size() < 2)
    return false; // Not enough data

  size_t num_samples = audio_data.size() / 2;
  double sum_squares = 0.0;

  // Pointer to raw audio bytes
  const int16_t *samples =
      reinterpret_cast<const int16_t *>(audio_data.constData());

  for (size_t i = 0; i < num_samples; i++)
  {
    int16_t sample = samples[i]; // Read 16-bit signed sample

    // Normalize to [-1, 1]
    double normalized_sample = sample / 32768.0;

    // Accumulate squared sample values
    sum_squares += normalized_sample * normalized_sample;
  }

  // Compute RMS
  double rms = std::sqrt(sum_squares / num_samples);

  // Compare with threshold
  return rms > threshold;
}

void AudioManager::set_audio_device(QAudioDevice &audio_device)
{
  if (!audio_device.isFormatSupported(_format))
  {
    qWarning() << "Default format not supported.";
    return;
  }
  _audio_device = &audio_device;
}

void AudioManager::set_media_player(QMediaPlayer &media_player)
{
  _media_player = &media_player;
}

QByteArray AudioManager::get_lastmsec(int n_ms)
{
  QMutexLocker locker(&_mutex);
  if (!_is_recording || n_ms > _max_buffer_size)
  {
    qWarning() << "Requested too much data or not recording.";
    return {};
  }

  int bytes_per_ms = (_format.sampleRate() * _format.channelCount() *
                      _format.bytesPerSample()) /
                     1000;
  int requested_bytes = n_ms * bytes_per_ms;
  int available_bytes = qMin(requested_bytes, _audio_bytearray.size());

  int aligned_bytes =
      (available_bytes / _format.bytesPerSample()) * _format.bytesPerSample();

  if (has_media_player() &&
      (_media_player->mediaStatus() == QMediaPlayer::EndOfMedia ||
       !_media_player->isPlaying()))
  {
    return {};
  }

  return _audio_bytearray.right(aligned_bytes);
}

void AudioManager::on_audio_data_ready()
{
  if (_audio_source == nullptr)
    return;

  if (!_buffer_device.isOpen())
    return;

  QByteArray audio_data = _buffer_device.readAll(); // Read from QAudioSource
  int bytes_per_sample = _format.bytesPerSample();

  // Align audio_data size to the nearest full sample
  int aligned_size = (audio_data.size() / bytes_per_sample) * bytes_per_sample;

  if (aligned_size > 0)
  {
    QMutexLocker locker(&_mutex);
    _audio_bytearray.append(
        audio_data.left(aligned_size)); // Append only full samples

    // Prevent buffer from exceeding max size
    int max_bytes = (_max_buffer_size * _format.sampleRate() *
                     _format.channelCount() * bytes_per_sample) /
                    1000;
    if (_audio_bytearray.size() > max_bytes)
    {
      _audio_bytearray.remove(0, _audio_bytearray.size() - max_bytes);
    }
  }
}

void AudioManager::on_audio_buffer_received(const QAudioBuffer &buffer)
{
  if (!buffer.isValid())
    return;

  const quint8 *data = buffer.constData<quint8>();
  int data_size = buffer.byteCount();

  if (data_size > 0)
  {
    QMutexLocker locker(&_mutex);

    _audio_bytearray.append(reinterpret_cast<const char *>(data), data_size);

    // Prevent buffer from exceeding max size
    int bytes_per_sample = _format.bytesPerSample();
    int max_bytes = (_max_buffer_size * _format.sampleRate() *
                     _format.channelCount() * bytes_per_sample) /
                    1000;

    if (_audio_bytearray.size() > max_bytes)
    {
      _audio_bytearray.remove(0, _audio_bytearray.size() - max_bytes);
    }
  }
}

void AudioManager::setup_audio()
{
  _format.setSampleRate(44100);
  _format.setChannelCount(1);
  _format.setSampleFormat(QAudioFormat::Int16);
}
