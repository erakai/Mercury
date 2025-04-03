#pragma once

#include "metrics.hpp"
#include "netlib_global.h"
#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QNetworkDatagram>
#include <QUdpSocket>
#include <memory>
#include <vector>

#define MAX_FRAMES_TO_REASSEMBLE 4
#define SOURCE_NAME_LEN 12

constexpr uint8_t MFTP_VERSION = 1;

/*
Packet formatting. Note modifying this header rquires updating the
process_datagram implementation as well, or everything breaks.
*/

// Also update size_of_mftp_header
struct NETLIB_EXPORT MFTP_Header
{
  uint8_t version;                         // 1 byte
  uint16_t payload_type;                   // 2 bytes
  uint16_t seq_num;                        // 2 bytes
  uint16_t total_fragments;                // 2 bytes
  uint16_t fragment_num;                   // 2 bytes
  uint32_t timestamp;                      // 4 bytes
  uint32_t audio_len;                      // 4 bytes
  uint32_t video_len;                      // 4 bytes
  char source_name[SOURCE_NAME_LEN] = {0}; // 12 bytes

  // Used in testing
  bool operator==(const MFTP_Header &other) const
  {
    return version == other.version && payload_type == other.payload_type &&
           seq_num == other.seq_num && timestamp == other.timestamp &&
           audio_len == other.audio_len && video_len == other.video_len &&
           std::memcmp(source_name, other.source_name, sizeof(source_name)) ==
               0;
  }
};

/*
Processor class that re-assembles the fragmented datagrams into one big one.
*/

struct NETLIB_EXPORT PartialFrame
{
  int remaining_fragments = -1;
  MFTP_Header header;
  std::vector<QByteArray> datagrams;
};

class NETLIB_EXPORT MFTPProcessor : public QObject
{
  Q_OBJECT;

public:
  PartialFrame partial_frames[MAX_FRAMES_TO_REASSEMBLE];

public slots:
  void process_ready_datagrams(std::shared_ptr<QUdpSocket> socket,
                               Metrics &metrics);

signals:
  void frame_ready(MFTP_Header header, QByteArray audio, QImage video);

private:
  bool process_datagram(QNetworkDatagram datagram, Metrics &metrics);
  void release_complete_frame(int index);
  void fix_partial_frame_array();

  void print_partial_frame_array();
};

/*
General usage.
*/

NETLIB_EXPORT std::shared_ptr<QUdpSocket> acquire_mftp_socket(int port);
NETLIB_EXPORT void acquire_mftp_socket(std::shared_ptr<QUdpSocket>, int port);
NETLIB_EXPORT void print_header(MFTP_Header header);
NETLIB_EXPORT int size_of_mftp_header();

/*
Host-side functionality.
*/

NETLIB_EXPORT bool send_datagram(std::shared_ptr<QUdpSocket> sock,
                                 std::vector<QHostAddress> dest_ip,
                                 std::vector<int> dest_port,
                                 MFTP_Header &header, QImage video_image,
                                 QByteArray &audio);
