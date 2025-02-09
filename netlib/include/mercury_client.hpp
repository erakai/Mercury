#pragma once

#include "mftp.hpp"
#include <vector>

class MercuryClient : public QObject
{
  Q_OBJECT;

public:
  /*
  Pops the next frame from the client's jitter buffer, and returns it packaged
  within the AV_Frame struct. Contains both audio and video.

  todo: should it be deserialized here instead and return as an audio/video
  object? what audio/video objects would we use?
  */
  AV_Frame retrieve_next_frame();

private:
  // This class will require a jitter buffer ordered by sequence number for RTP
  // implementation. Investigate: https://doc.qt.io/qt-6/qbuffer.html
  std::vector<AV_Frame> jitter_buffer;
};
