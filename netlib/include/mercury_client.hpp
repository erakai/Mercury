#pragma once

#include "hstp.hpp"
#include "mftp.hpp"
#include <QtCore/qtypes.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qudpsocket.h>
#include <memory>
#include <vector>

class MercuryClient : public QObject
{
  Q_OBJECT;

public:
  MercuryClient(const std::string &alias) : m_alias(alias)
  {
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
  bool establish_connection(const QHostAddress &host, quint16 port);

  /*
   * Disconnects the TCP/UDP sockets. Slot allows UI to disconnect on signal.
   */
  bool disconnect();

  /*
   * Informs the server that the client is sending out a chat message.
   */
  void send_chat(const std::string &chat_msg);

  /*
   * Connected to the readyRead signal of QTcpSocket. Will process the given
   * HSTP message and emit signals based on that.
   */
  void process_received_hstp_messages();

signals:

  /*
   * Emits that an echo option happened and prints the messsage to the logger.
   * Slot should only be used for debugging purposes.
   */
  void emit_echo_option();

  /*
   * Default option for processing hstp messages, when no other emit option
   * could be made. Should be used only for debugging and error states
   */
  std::shared_ptr<HSTP_Header> emit_generic_option();

private:
  /*
  Private method to help set up the signals/slots of the client.
  */
  void connect_signals_and_slots();

  // This class will require a jitter buffer ordered by sequence number for RTP
  // implementation. Investigate: https://doc.qt.io/qt-6/qbuffer.html
  std::vector<AV_Frame> m_jitter_buffer;

  std::shared_ptr<QUdpSocket> m_mftp_sock;
  std::shared_ptr<QTcpSocket> m_hstp_sock;

  std::string m_alias;

  HstpHandler m_hstp_handler;
};
