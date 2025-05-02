#include "videomanager.h"
#include <QtMultimedia/qvideosink.h>

void VideoManager::setMediaCaptureSession(QMediaCaptureSession &session)
{
  m_captureSession = &session;
  m_captureSession->setVideoSink(m_videoSink);
}

void VideoManager::setMediaPlayer(QMediaPlayer &player)
{
  m_mediaPlayer = &player;
  m_mediaPlayer->setVideoSink(m_videoSink);
}

QMediaPlayer *VideoManager::getMediaPlayer()
{
  return m_mediaPlayer;
}

VideoManager::VideoImageStatus VideoManager::GetVideoImage(QImage &image)
{
  if (!m_captureSession && !m_mediaPlayer)
    return MEDIACAPTURENOTSET;

  if (m_lastImage.isNull())
    return FAILURE;

  image = m_lastImage;
  return SUCCESS;
}

// TODO: optimize this, this is really inefficient but will work for now.
// Connect the QVideoSink's videoFrameChanged signal to update the cached
// image.
void VideoManager::onVideoFrameChange(const QVideoFrame &frame)
{
  uint64_t current_time = QDateTime::currentMSecsSinceEpoch();
  uint64_t delta = current_time - last_frame_s;

  if (delta < (1000.0 / FPS - 5))
    return;

  last_frame_s = current_time;
  if (frame.isValid())
  {
    QVideoFrame cloneFrame(frame);
    m_lastImage = cloneFrame.toImage();
  }
}
