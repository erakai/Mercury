#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QObject>
#include <QImage>
#include <QDateTime>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QtCore/qlogging.h>
#include <QtMultimedia/qmediaplayer.h>

#include "config/mconfig.hpp"

class VideoManager : public QObject
{
  Q_OBJECT
public:
  enum VideoImageStatus
  {
    SUCCESS,
    FAILURE,
    MEDIACAPTURENOTSET
  };

  static VideoManager &instance()
  {
    static VideoManager instance;
    return instance;
  }

  void setMediaCaptureSession(QMediaCaptureSession &session);
  VideoImageStatus GetVideoImage(QImage &image);
  bool mediaCaptureIsSet() { return m_captureSession != nullptr; }
  bool mediaPlayerIsSet() { return m_mediaPlayer != nullptr; }

  void setMediaPlayer(QMediaPlayer &player);
  QMediaPlayer *getMediaPlayer();

private:
  QMediaCaptureSession *m_captureSession;
  QMediaPlayer *m_mediaPlayer;
  QVideoSink *m_videoSink;
  QImage m_lastImage;

  void onVideoFrameChange(const QVideoFrame &frame);

  uint64_t last_frame_s = 0;

  // Private constructor prevents direct instantiation.
  explicit VideoManager(QObject *parent = nullptr)
      : QObject(parent), m_captureSession(nullptr),
        m_videoSink(new QVideoSink(this))
  {
    if (!m_lastImage.load("assets/audio_waveform.png"))
    {
      qDebug() << "Failed to load in default image for player";
    };
    connect(m_videoSink, &QVideoSink::videoFrameChanged, this,
            &VideoManager::onVideoFrameChange);
  }

  // Destructor (can be defaulted or customized)
  ~VideoManager()
  {
    if (m_captureSession)
      delete (m_captureSession);
  }

  // Delete copy constructor and assignment operator to prevent copies.
  VideoManager(const VideoManager &) = delete;
  VideoManager &operator=(const VideoManager &) = delete;
};

#endif // VIDEOMANAGER_H
