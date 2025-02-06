#pragma once

#include "hstp.hpp"
#include "mftp.hpp"

class MercuryClient : public QObject
{
  Q_OBJECT;

public:
private:
  // This class will require a jitter buffer for RTP implementation.
  // Investigate: https://doc.qt.io/qt-6/qbuffer.html

  // How to serialize audio data into a QByteArray:
  // https://stackoverflow.com/questions/37577036/fill-qbytearray-from-qaudiobuffer
};
