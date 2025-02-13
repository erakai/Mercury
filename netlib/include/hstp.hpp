#pragma once

#include <QHostAddress>
#include <QTcpSocket>
#include <QtCore/qstringview.h>
#include <cstdint>
#include <memory>
#include <vector>

/*
Packet formatting.

HSTP must be designed to allow for modular information, as the control messages
sent will expand in scope. To do this, we send over a preliminary header, which
contains data universal in control messages. Appended to this header is a
dynamic number of options. Each options is specified by a type, gives a length,
and contains data specific to that option.

These options will be documented in docs/hstp_options.txt.
They are inspired by DHCP options: https://www.ietf.org/rfc/rfc2132.txt.

HSTP Format:
[sender_alias[18], uint8_t option_count, options[]]
*/

#define ALIAS_SIZE 18

struct Option
{
  uint8_t type;
  uint8_t len; // length of data field in bytes (may be 0)
  const char *data;
};

struct HSTP_Header
{
  char sender_alias[ALIAS_SIZE];
  std::vector<Option> options;
};

/*
General usage.
*/

enum class MSG_STATUS
{
  READY,
  IN_PROGRESS,
  SENDING
};

class HstpHandler
{
  friend class HstpHandlerTester;

public:
  HstpHandler()
      : m_current_status(MSG_STATUS::READY),
        m_qtcp_sock(std::shared_ptr<QTcpSocket>(new QTcpSocket(nullptr)))
  {
  }

  HstpHandler(std::shared_ptr<QTcpSocket> sock)
      : m_current_status(MSG_STATUS::READY), m_qtcp_sock(sock)
  {
  }

  // connection
  bool connect(QHostAddress addr, int port);
  void close();

  // transmission
  bool init_msg(char sender_alias[18]);
  bool add_option_echo(char *msg);
  bool send_msg();
  void clear_msg();

  // retrival
  std::shared_ptr<HSTP_Header> read();
  std::vector<std::shared_ptr<HSTP_Header>> readAll();

  // status
  MSG_STATUS get_status();

private:
  MSG_STATUS m_current_status;
  std::shared_ptr<QTcpSocket> m_qtcp_sock;

  QByteArray _serialize(HSTP_Header &hdr);
  HSTP_Header _deserialize(const QByteArray &buff);

  HSTP_Header *m_hdr;
};

class HstpHandlerTester
{
public:
  bool test_serialize(HstpHandler &obj);
  bool test_deserialize(HstpHandler &obj);
};
