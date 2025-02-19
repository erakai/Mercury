#pragma once

#include "logger.hpp"
#include <QHostAddress>
#include <QTcpSocket>
#include <QtCore/qobject.h>
#include <QtCore/qtmetamacros.h>
#include <QtCore/qtypes.h>
#include <cstdint>
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
[sender_alias[ALIAS_SIZE], uint8_t option_count, options[]]
*/

#define ALIAS_SIZE 18
#define CHAT_SIZE_LIMIT 500

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
  bool init_msg(char sender_alias[ALIAS_SIZE]);

  bool add_option_echo(const char *msg);
  bool add_option_establishment(bool is_start, uint16_t port);
  bool add_option_chat(const char *chat_msg);

  void clear_msg();
  std::shared_ptr<QByteArray> output_msg();

  std::shared_ptr<HSTP_Header>
  bytes_to_msg(const std::shared_ptr<QByteArray> &buff);

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

/*
 * Processor is intended to be in conjunction with clients/servers
 * as it provides the ability for user to input a recently received
 * HSTP packet and emit a series of signals based on it's options.
 * Users can then slot into these signals to do different actions.
 *
 * Abstracts away much of the need to know exact details of the HSTP
 * protocol.
 */

class HstpProcessor : public QObject
{
  Q_OBJECT
public:
  explicit HstpProcessor(QObject *parent = nullptr);

  /*
   * Given a header, will emit a series of signals that can be slotted to when
   * HSTP packets arrive
   */
  void process(const std::shared_ptr<HSTP_Header> &hdr_ptr);

signals:

  /*
   * In the case where the option is malformed and cannot be determined, this
   * will be emitted for the option with a header only containing the malformed
   * option. Should be handled in error states and debugging.
   */
  void emit_generic(const std::shared_ptr<HSTP_Header> &hdr_ptr);

  /*
   * Emits the fact an emit option occured, the function will print the
   * echo. This signal should be intercepted for debug reasons.
   */
  void emit_echo(const char alias[ALIAS_SIZE], const std::string &msg);

  /*
   * Emits during establishment of the client and server for basic syncing
   */
  void emit_establishment(const char alias[ALIAS_SIZE], bool is_start,
                          uint16_t mftp_port);
  /*
   * Emtis when a chat option is used informing user of a new chat.
   */
  void emit_chat(const char alias[ALIAS_SIZE], const std::string &chat);

private:
#define HANDLER_PARAMS const char alias[ALIAS_SIZE], const Option &opt

  void handle_default(HANDLER_PARAMS);       // n/a
  void handle_echo(HANDLER_PARAMS);          // 0
  void handle_establishment(HANDLER_PARAMS); // 1
  void handle_chat(HANDLER_PARAMS);          // 2
};
