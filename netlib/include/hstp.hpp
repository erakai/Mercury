#pragma once

#include <QHostAddress>
#include <QTcpSocket>
#include <QtCore/qcontainerfwd.h>
#include <QtCore/qobject.h>
#include <QtCore/qstringview.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <logger.hpp>
#include <memory>
#include <sys/_endian.h>
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
  std::shared_ptr<char[]> data;
  bool operator==(const Option &other) const
  {
    if (type != other.type || len != other.len)
    {
      return false; // Types or lengths do not match
    }
    if (len == 0)
    {
      return true; // Both are empty
    }
    return std::memcmp(data.get(), other.data.get(), len) == 0;
  }
};

struct HSTP_Header
{
  char sender_alias[ALIAS_SIZE] = {0};
  std::vector<Option> options;

  bool operator==(const HSTP_Header &other) const
  {
    return std::memcmp(sender_alias, other.sender_alias, ALIAS_SIZE) == 0 &&
           options.size() == other.options.size() &&
           std::equal(options.begin(), options.end(), other.options.begin());
  }
};

enum class MSG_STATUS
{
  READY,
  IN_PROGRESS,
};

/*
 * HstpHandler offers the user an object to build out a HSTP message
 * through a series of function calls to ensure safety and proper use.
 * Furthermore, elimates the need to know exact details how avaliable
 * options.
 *
 * Use by starting with init_msg(alias) and then various add_option_* to
 * append to the message. Then emit_msg() to get the bytes and reset the
 * state of Handler for the next message.
 */

class HstpHandler
{
  friend class HstpHandlerTest;

public:
  HstpHandler() : m_current_status(MSG_STATUS::READY) {}

  // messaging processing, allows you to build a HSTP header
  bool init_msg(char sender_alias[18]);
  bool add_option_echo(const char *msg);
  void clear_msg();
  std::shared_ptr<QByteArray>
  emit_msg(); // emits the bytes from the constructed message and resets
  // status

  std::shared_ptr<HSTP_Header>
  bytes_to_msg(const std::shared_ptr<QByteArray>
                   &buff); // converts string of bytes to HSTP message

  // status
  MSG_STATUS get_status();

private:
  MSG_STATUS m_current_status;

  std::shared_ptr<QByteArray>
  _serialize(const std::shared_ptr<HSTP_Header> &hdr);
  std::shared_ptr<HSTP_Header>
  _deserialize(const std::shared_ptr<QByteArray> &buff);

  std::shared_ptr<HSTP_Header> m_hdr;
};
