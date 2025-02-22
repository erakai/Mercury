#pragma once

#include "hstp.hpp"
#include "mftp.hpp"
#include <QtCore/qstringview.h>
#include <QtCore/qtypes.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qudpsocket.h>
#include <memory>
#include <vector>

class MercuryClient : public QObject
{
  friend class MercuryClientTest;
  Q_OBJECT;

public:
  MercuryClient(const std::string &alias) : m_alias(alias)
  {
    m_hstp_sock = std::make_shared<QTcpSocket>();
    m_mftp_sock = std::make_shared<QUdpSocket>();
    connect_signals_and_slots();
  };

  /*
  Pops the next frame from the client's jitter buffer, and returns it packaged
  within the AV_Frame struct. Contains both audio and video.

  todo: should it be deserialized here instead and return as an audio/video
  object? what audio/video objects would we use?
  */
  AV_Frame retrieve_next_frame();

public slots:
  /*
   * Connects to the given host address and port via both UDP/TCP. Slot allows
   * UI to connect the client on signal
   */
  bool establish_connection(const QHostAddress &host, quint16 hstp_port,
                            quint16 mftp_port);

  /*
   * Disconnects the TCP/UDP sockets. Slot allows UI to disconnect on signal.
   */
  bool disconnect();

  /*
   * Informs the server that the client is sending out a chat message.
   */
  bool talk_tuah(const std::string &chat_msg);

  /*
   * Connected to the readyRead signal of QTcpSocket. Will process the given
   * HSTP message and emit signals based on that.
   */
  void process_received_hstp_messages();

  std::shared_ptr<HstpProcessor> hstp_processor() const
  {
    return m_hstp_processor_ptr;
  }

signals:
  void test_readyread_callback(); // used only in testing purposes

private:
  /*
  Private method to help set up the signals/slots of the client.
  */
  void connect_signals_and_slots();

  /*
   * Attempts to process a single HSTP header from the hstp_buffer. On success,
   * calls HstpHandler.process(header) and emits option signals and returns
   * the number of bytes read. On failure logs an error and returns -1.
   */
  qint16 process_single_hstp_message(qint16 opt_size);

  // This class will require a jitter buffer ordered by sequence number for RTP
  // implementation. Investigate: https://doc.qt.io/qt-6/qbuffer.html
  std::vector<AV_Frame> m_jitter_buffer;

  std::shared_ptr<QUdpSocket> m_mftp_sock;
  std::shared_ptr<QTcpSocket> m_hstp_sock;

  std::string m_alias;

  HstpHandler m_hstp_handler;
  std::shared_ptr<HstpProcessor> m_hstp_processor_ptr;

  QByteArray m_hstp_buffer;
  uint16_t m_hstp_opt_len = -1;
};
