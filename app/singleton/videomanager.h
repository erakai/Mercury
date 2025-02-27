#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QObject>
#include <QImage>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>

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

private:
  QMediaCaptureSession *m_captureSession;
  QVideoSink *m_videoSink;
  QImage m_lastImage;

  // Private constructor prevents direct instantiation.
  explicit VideoManager(QObject *parent = nullptr)
      : QObject(parent), m_captureSession(nullptr),
        m_videoSink(new QVideoSink(this))
  {
    // TODO: optimize this, this is really inefficient but will work for now.
    // Connect the QVideoSink's videoFrameChanged signal to update the cached
    // image.
    connect(m_videoSink, &QVideoSink::videoFrameChanged, this,
            [this](const QVideoFrame &frame)
            {
              if (frame.isValid())
              {
                // Create a copy of the frame and map it for reading.
                QVideoFrame cloneFrame(frame);
                if (cloneFrame.map(QVideoFrame::ReadOnly))
                {
                  // Convert the frame to a QImage and store it.
                  m_lastImage = cloneFrame.toImage();
                  cloneFrame.unmap();
                }
              }
            });
  }

  // Destructor (can be defaulted or customized)
  ~VideoManager() = default;

  // Delete copy constructor and assignment operator to prevent copies.
  VideoManager(const VideoManager &) = delete;
  VideoManager &operator=(const VideoManager &) = delete;
};

#endif // VIDEOMANAGER_H
