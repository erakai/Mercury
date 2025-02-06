#pragma once

#include <QUdpSocket>

/*
Packet formatting
*/

struct MFTP_Header
{
  uint8_t version;
  uint16_t payload_type;
  uint16_t seq_num;
  uint32_t timestamp;
  uint16_t audio_len;
  uint16_t video_len;
  char source_name[12];
};

struct AV_Payload
{
};

/*
General usage.
*/

QUdpSocket acquire_mftp_socket();

/*
Host-side functionality.
*/

void send_packet(QUdpSocket sock, MFTP_Header header, AV_Payload payload);

/*
Client-side functionality.
*/

void recv_packet(QUdpSocket sock, MFTP_Header &header, AV_Payload &payload);
