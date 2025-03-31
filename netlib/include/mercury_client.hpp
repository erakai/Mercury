#pragma once

#include "metrics.hpp"
#include "netlib_global.h"
#include "hstp.hpp"
#include "mftp.hpp"
#include <QImage>
#include <QtCore/qstringview.h>
#include <QtCore/qtypes.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qudpsocket.h>
#include <deque>
#include <memory>

#define MINIMUM_FRAME_COUNT_FOR_PLAYBACK 1

struct NETLIB_EXPORT JitterEntry
{
  int16_t seq_num;
  int32_t timestamp;
  QImage video;
  QAudioBuffer audio;
};

class NETLIB_EXPORT MercuryClient : public QObject
{
  friend class MercuryClientTest;
  Q_OBJECT;

public:
  MercuryClient(const std::string &alias) : m_alias(alias)
  {
    m_hstp_sock = std::make_shared<QTcpSocket>();
    m_mftp_sock = std::make_shared<QUdpSocket>();
    m_hstp_processor_ptr = std::make_shared<HstpProcessor>();
    m_mftp_processor = std::make_shared<MFTPProcessor>();
    connect_signals_and_slots();
  };

  /*
  Pops the next frame from the client's jitter buffer, and returns it packaged
  within the AV_Frame struct. Contains both audio and video.

  todo: should it be deserialized here instead and return as an audio/video
  object? what audio/video objects would we use?
  */
  JitterEntry retrieve_next_frame();

  std::shared_ptr<HstpProcessor> hstp_processor() const
  {
    return m_hstp_processor_ptr;
  }

  HstpHandler &hstp_handler() { return m_hstp_handler; }

  std::shared_ptr<QTcpSocket> hstp_sock() { return m_hstp_sock; };

  std::string get_alias() { return m_alias; }

  int jitter_buffer_size() { return m_jitter_buffer.size(); }

  Metrics &metrics() { return m_metrics; }

  void set_is_stable_connection(bool val = true, std::string reason = "")
  {
    unstable_connection_reason = reason;
    if (val != stable_connection)
      emit connection_stablity_updated(val);
    stable_connection = val;
  }

  bool is_stable_connection() { return stable_connection; }
  std::string get_connection_reason() { return unstable_connection_reason; }

public slots:
  /*
   * Connects to the given host address and port via both UDP/TCP. Slot allows
   * UI to connect the client on signal.
   *
   * The hstp port is the port the server is on (for tcp), and the mftp port is
   * the port the client is on (for udp).
   */
  bool establish_connection(const QHostAddress &host, quint16 hstp_port,
                            quint16 mftp_port,
                            const QByteArray &pass = nullptr);

  /*
   * Disconnects the TCP/UDP sockets. Slot allows UI to disconnect on signal.
   */
  bool disconnect();

  /*
   * Informs the server that the client is sending out a chat message.
   */
  bool send_chat_message(const std::string &chat_msg);

  /*
   * Informs the server that the client is sending out a vector of points for
   * annotations.
   */
  bool send_annotations(const HSTP_Annotation &annotation);

  /*
   * Connected to the readyRead signal of QTcpSocket. Will process the given
   * HSTP message and emit signals based on that.
   */
  void process_received_hstp_messages();

  /*
  Insert a new frame into the jitter buffer, using the MFTP header to evaluate
  where to put it.
  */
  void insert_into_jitter_buffer(MFTP_Header header, QAudioBuffer audio,
                                 QImage video);

signals:
  /*
  Emitted when this client is forcibly disconnected from the server.
  */
  void client_disconnected();

  /*
  Emitted when the jitter buffer has enough frames to begin playback.
  */
  void jitter_buffer_sufficiently_full();

  /*
  Emitted when the boolean stable_connection changes
  */
  void connection_stablity_updated(bool is_stable_connection);

private:
  // Represents whether this client has begun playback
  bool begun_playback = false;

  // Represents whether or not this client has a stable connection
  // This is set by stream/performancetab.cpp
  bool stable_connection = true;
  std::string unstable_connection_reason;

  /*
  Private method to help set up the signals/slots of the client.
  */
  void connect_signals_and_slots();

  // This class will require a jitter buffer ordered by sequence number for RTP
  // implementation. Investigate: https://doc.qt.io/qt-6/qbuffer.html
  std::deque<JitterEntry> m_jitter_buffer;

  std::shared_ptr<QUdpSocket> m_mftp_sock;
  std::shared_ptr<QTcpSocket> m_hstp_sock;

  std::string m_alias;

  HstpHandler m_hstp_handler;
  std::shared_ptr<HstpProcessor> m_hstp_processor_ptr;

  std::shared_ptr<MFTPProcessor> m_mftp_processor;

  Metrics m_metrics;
};
