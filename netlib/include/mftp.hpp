#pragma once

#include <QByteArray>
#include <QNetworkDatagram>
#include <QUdpSocket>

#include <memory>

constexpr uint8_t MFTP_VERSION = 1;

/*
Packet formatting. Note modifying this header rquires updating the
process_datagram implementation as well, or everything breaks.
*/

struct MFTP_Header
{
  uint8_t version;       // 1 byte
  uint16_t payload_type; // 2 bytes
  uint16_t seq_num;      // 2 bytes
  uint32_t timestamp;    // 4 bytes
  uint16_t audio_len;    // 2 bytes
  uint16_t video_len;    // 2 bytes
  char source_name[12];  // 12 bytes
};

struct AV_Frame
{
  QByteArray audio;
  QByteArray video;
};

/*
General usage.
*/

std::shared_ptr<QUdpSocket> acquire_mftp_socket(int port);

/*
Host-side functionality.
*/

bool send_datagram(std::shared_ptr<QUdpSocket> sock, QHostAddress dest_ip,
                   int dest_port, MFTP_Header header, AV_Frame payload);

/*
Client-side functionality. Note that this is only a helper function to process
bytes into the header/payload. To properly receive packets, after acquiring
the socket, make sure to connect the "readyRead" signal to some slot:

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &YOUR_FUNCTION);

Then you may call process_packet within YOUR_FUNCTION to deserialize the data.
*/

bool process_datagram(std::shared_ptr<QUdpSocket> sock, MFTP_Header &header,
                      AV_Frame &payload);
