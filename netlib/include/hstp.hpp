#pragma once

#include "netlib_global.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QtCore/qhashfunctions.h>
#include <QtCore/qobject.h>
#include <QtCore/qstringview.h>
#include <QtCore/qtmetamacros.h>
#include <QtCore/qtypes.h>
#include <QtNetwork/qtcpsocket.h>
#include <QPoint>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
#include <QtEndian>

/*
Packet formatting.

HSTP must be designed to allow for modular information, as the control messages
sent will expand in scope. To do this, we send over a preliminary header, which
contains data universal in control messages. Appended to this header is a
dynamic number of options. Each options is specified by a type, gives a length,
and contains data specific to that option.

These options will be documented in docs/hstp_options.txt.
They are inspired by DHCP options: https://www.ietf.org/rfc/rfc2132.txt.
*/

#define ALIAS_SIZE 18
#define CHAT_SIZE_LIMIT 500
#define MAX_STRING_SIZE 1024
#define HSTP_HEADER_SIZE                                                       \
  ALIAS_SIZE * sizeof(char) + sizeof(uint16_t) + sizeof(uint8_t)

struct NETLIB_EXPORT Option
{
  uint8_t type;
  uint16_t len; // length of data field in bytes (may be 0)
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

  friend std::ostream &operator<<(std::ostream &os, const Option &option)
  {
    for (size_t i = 0; i < option.len; ++i)
    {
      os << "0x" << std::hex << std::uppercase << std::setw(2)
         << std::setfill('0')
         << static_cast<int>(static_cast<unsigned char>(option.data[i])) << " ";
    }
    return os << std::dec; // Reset to decimal format
  }
};

struct NETLIB_EXPORT HSTP_Header
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

// Helper structure for a 2D point
struct AnnotationPoint
{
  uint16_t x;
  uint16_t y;
};

struct NETLIB_EXPORT HSTP_Annotation
{
  // A vector of points
  std::vector<AnnotationPoint> points;
  // RGB color values
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  // Thickness instead of the erase flag
  int8_t thickness;

  // Default constructor
  HSTP_Annotation() = default;

  HSTP_Annotation(const std::vector<QPoint> &v, uint8_t r, uint8_t g, uint8_t b,
                  uint8_t thick)
      : red(r), green(g), blue(b), thickness(thick)
  {
    points.reserve(v.size());
    for (const auto &qp : v)
    {
      // Convert QPoint to Point (casting to uint16_t)
      points.push_back(
          {static_cast<uint16_t>(qp.x()), static_cast<uint16_t>(qp.y())});
    }
  }

  // Constructor that deserializes from a shared_ptr<char[]>
  HSTP_Annotation(const std::shared_ptr<char[]> &buffer)
  {
    const char *ptr = buffer.get();

    // First, deserialize the number of points
    uint16_t num_points;
    std::memcpy(&num_points, ptr, sizeof(num_points));
    ptr += sizeof(num_points);

    // Resize the vector and deserialize each point
    points.resize(num_points);
    for (size_t i = 0; i < num_points; ++i)
    {
      std::memcpy(&points[i].x, ptr, sizeof(points[i].x));
      ptr += sizeof(points[i].x);
      std::memcpy(&points[i].y, ptr, sizeof(points[i].y));
      ptr += sizeof(points[i].y);
    }

    // Deserialize the RGB values
    std::memcpy(&red, ptr, sizeof(red));
    ptr += sizeof(red);

    std::memcpy(&green, ptr, sizeof(green));
    ptr += sizeof(green);

    std::memcpy(&blue, ptr, sizeof(blue));
    ptr += sizeof(blue);

    // Deserialize the thickness (stored as a single byte)
    std::memcpy(&thickness, ptr, sizeof(thickness));
  }

  // Serializes the current annotation to a shared_ptr<char[]>
  std::shared_ptr<char[]> serialize() const
  {
    // Calculate total size:
    // - 2 bytes for the number of points
    // - 4 bytes per point (2 bytes for x and 2 for y)
    // - 3 bytes for RGB
    // - 1 byte for thickness
    size_t dataSize = sizeof(uint16_t) +
                      points.size() * (sizeof(uint16_t) * 2) + sizeof(red) +
                      sizeof(green) + sizeof(blue) + sizeof(thickness);

    std::shared_ptr<char[]> buffer(new char[dataSize],
                                   std::default_delete<char[]>());
    char *ptr = buffer.get();

    // Serialize the number of points
    uint16_t num_points = points.size();
    std::memcpy(ptr, &num_points, sizeof(num_points));
    ptr += sizeof(num_points);

    // Serialize each point
    for (const auto &pt : points)
    {
      std::memcpy(ptr, &pt.x, sizeof(pt.x));
      ptr += sizeof(pt.x);
      std::memcpy(ptr, &pt.y, sizeof(pt.y));
      ptr += sizeof(pt.y);
    }

    // Serialize the RGB values
    std::memcpy(ptr, &red, sizeof(red));
    ptr += sizeof(red);

    std::memcpy(ptr, &green, sizeof(green));
    ptr += sizeof(green);

    std::memcpy(ptr, &blue, sizeof(blue));
    ptr += sizeof(blue);

    // Serialize the thickness as a byte
    std::memcpy(ptr, &thickness, sizeof(thickness));

    return buffer;
  }
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

class NETLIB_EXPORT HstpHandler
{
  friend class HstpHandlerTest;

public:
  HstpHandler() : m_current_status(MSG_STATUS::READY) {}

  // messaging processing, allows you to build a HSTP header
  bool init_msg(const char sender_alias[ALIAS_SIZE]);

  bool add_option_echo(const char *msg);
  bool add_option_establishment(bool is_start, uint16_t port,
                                const QByteArray &password = nullptr);
  bool add_option_chat(const char alias_of_chatter[ALIAS_SIZE],
                       const char *chat_msg);
  bool add_option_annotation(const HSTP_Annotation &annotation);
  bool add_option_stream_title(const char *stream_title)
  {
    return add_option_generic_string(3, stream_title);
  }
  bool add_option_viewer_count(uint32_t viewers)
  {
    return add_option_generic_uint32(4, viewers);
  }

  void clear_msg();
  std::shared_ptr<QByteArray> output_msg();
  bool output_msg_to_socket(const std::shared_ptr<QTcpSocket> &sock);

  static std::shared_ptr<HSTP_Header>
  bytes_to_msg(const std::shared_ptr<QByteArray> &buff);

  // status
  MSG_STATUS get_status();

private:
  MSG_STATUS m_current_status;

  static std::shared_ptr<QByteArray>
  _serialize(const std::shared_ptr<HSTP_Header> &hdr);
  static std::shared_ptr<HSTP_Header>
  _deserialize(const std::shared_ptr<QByteArray> &buff);

  bool add_option_generic_string(uint8_t type, const char *gen_str);
  bool add_option_generic_uint32(uint8_t type, uint32_t uint32);

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

class NETLIB_EXPORT HstpProcessor : public QObject
{
  Q_OBJECT;

public:
  explicit HstpProcessor(QObject *parent = nullptr) {}

  /*
   * Takes a single chunk of TCP packet information and attempt to process it.
   */
  void process(const QByteArray &chunk);

  /*
   * Given a header, will emit a series of signals that can be slotted to when
   * HSTP packets arrive
   */
  void emit_header(const std::shared_ptr<HSTP_Header> &hdr_ptr);

signals:

  /*
   * In the case where the option is malformed and cannot be determined, this
   * will be emitted for the option with a header only containing the
   * malformed option. Should be handled in error states and debugging.
   */
  void received_generic(const std::shared_ptr<HSTP_Header> &hdr_ptr);

  /*
   * Emits the fact an echo option occured, the function will print the
   * echo. This signal should be intercepted for debug reasons.
   */
  void received_echo(const char alias[ALIAS_SIZE], const std::string &msg);

  /*
   * Emits during establishment of the client and server for basic syncing
   */
  void received_establishment(const char alias[ALIAS_SIZE], bool is_start,
                              uint16_t mftp_port,
                              const QByteArray &password = nullptr);
  /*
   * Emits when a chat option is used informing user of a new chat.
   */
  void received_chat(const char alias[ALIAS_SIZE],
                     const char alias_of_chatter[ALIAS_SIZE],
                     const std::string &chat);

  /*
   * Emits when an annotation pixel is received
   */
  void received_annotation(const char alias[ALIAS_SIZE],
                           const HSTP_Annotation &annotation);

  /*
   * Emits when a (new) stream title is used.
   */
  void received_stream_title(const char alias[ALIAS_SIZE],
                             const char *stream_title);

  /*
   * Emits when a (new) viewer count is received.
   */
  void received_viewer_count(const char alias[ALIAS_SIZE], uint32_t viewers);

private:
#define HANDLER_PARAMS const char alias[ALIAS_SIZE], const Option &opt

  QByteArray m_hstp_buffer;
  uint16_t m_hstp_opt_len = -1;

  /*
   * Attempts to process a single HSTP header from the hstp_buffer. On
   * success, calls HstpHandler.process(header) and emits option signals and
   * returns the number of bytes read. On failure logs an error and returns
   * -1.
   */
  qint16 process_single_hstp_message(qint16 opt_size);

  // generic handlers: used to prevent code duplication:
  void handle_string(HANDLER_PARAMS,
                     void (HstpProcessor::*signal)(const char alias[ALIAS_SIZE],
                                                   const char *str));
  void handle_uint32(HANDLER_PARAMS,
                     void (HstpProcessor::*signal)(const char alias[ALIAS_SIZE],
                                                   uint32_t uint32));

  void handle_default(HANDLER_PARAMS);       // n/a
  void handle_echo(HANDLER_PARAMS);          // 0
  void handle_establishment(HANDLER_PARAMS); // 1
  void handle_chat(HANDLER_PARAMS);          // 2
  void handle_stream_title(HANDLER_PARAMS)   // 3
  {
    handle_string(alias, opt, &HstpProcessor::received_stream_title);
  }
  void handle_viewer_count(HANDLER_PARAMS) // 4
  {
    handle_uint32(alias, opt, &HstpProcessor::received_viewer_count);
  }

  void handle_annotation(HANDLER_PARAMS); // 5
};
