#include "logger.hpp"
#include <mftp.hpp>

std::shared_ptr<QUdpSocket> acquire_mftp_socket(int port)
{
  std::shared_ptr<QUdpSocket> sock =
      std::shared_ptr<QUdpSocket>(new QUdpSocket(nullptr));

  // Bind socket to target port on this machine
  sock->bind(QHostAddress::LocalHost, port);

  log("MFTP Socket created and bound on port %d.", port, ll::NOTE);

  return sock;
}

bool send_datagram(std::shared_ptr<QUdpSocket> sock, QHostAddress dest_ip,
                   int dest_port, MFTP_Header header, AV_Frame payload)
{
  // Set up data that we will be sending across
  QByteArray data;
  QDataStream ds(&data, QIODevice::WriteOnly);
  ds << header.version;
  ds << header.payload_type;
  ds << header.seq_num;
  ds << header.timestamp;
  ds << header.audio_len;
  ds << header.video_len;
  ds << header.source_name;
  data.append(payload.audio);
  data.append(payload.video);

  // Define datagram
  QNetworkDatagram dg(data, dest_ip, dest_port);

  // Send
  if (sock->writeDatagram(dg) < 0)
  {
    log("Datagram failed to send due to size (%d bytes).", data.size(),
        ll::WARNING);
    return false;
  }

  return true;
}

bool process_datagram(std::shared_ptr<QUdpSocket> sock, MFTP_Header &header,
                      AV_Frame &payload)
{
  if (!sock->hasPendingDatagrams())
    return false;

  // Acquire datagram
  QNetworkDatagram dg = sock->receiveDatagram();
  QByteArray data = dg.data();

  if (data.size() < sizeof(MFTP_Header))
  {
    log("Invalid datagram received - only %d bytes.", data.size(), ll::WARNING);
    return false;
  }

  // Read in the header
  QDataStream ds(data);
  ds >> header.version;
  ds >> header.payload_type;
  ds >> header.seq_num;
  ds >> header.timestamp;
  ds >> header.audio_len;
  ds >> header.video_len;
  strcpy(header.source_name, data.sliced(sizeof(MFTP_Header) - 11, 12).data());

  if (header.audio_len + header.video_len <= 0)
    return true;

  // If we have a payload, process it
  data.slice(sizeof(MFTP_Header), header.audio_len + header.video_len);
  payload.audio = data.sliced(0, header.audio_len);
  payload.video = data.sliced(header.audio_len, header.video_len);

  return true;
}
