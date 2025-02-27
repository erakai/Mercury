#include "videomanager.h"

void VideoManager::setMediaCaptureSession(QMediaCaptureSession &session)
{
  m_captureSession = &session;
  m_captureSession->setVideoSink(m_videoSink);
}

VideoManager::VideoImageStatus VideoManager::GetVideoImage(QImage &image)
{
  if (!m_captureSession)
    return MEDIACAPTURENOTSET;

  if (m_lastImage.isNull())
    return FAILURE;

  image = m_lastImage;
  return SUCCESS;
}
