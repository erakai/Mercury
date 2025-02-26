#include "logger.hpp"
#include <mftp.hpp>

std::shared_ptr<QUdpSocket> acquire_mftp_socket(int port)
{
  std::shared_ptr<QUdpSocket> sock = std::make_shared<QUdpSocket>();

  // Bind socket to target port on this machine
  if (!sock->bind(QHostAddress::Any, port))
  {
    log("Unable to bind MFTP socket.", ll::ERROR);
    return nullptr;
  }

  log("MFTP Socket created and bound on %s and port %d.",
      sock->localAddress().toString().toStdString().c_str(), port, ll::NOTE);

  return sock;
}

bool send_datagram(std::shared_ptr<QUdpSocket> sock, QHostAddress dest_ip,
                   int dest_port, MFTP_Header header, AV_Frame payload)
{
  // Assume MTU size is 8192. Divide up the data into that many
  // fragments
  int total_size = payload.audio.size() + payload.video.size();
  int mtu = 1500 - sizeof(MFTP_Header);
  int number_of_fragments = 1 + total_size / mtu;
  header.total_fragments = number_of_fragments;

  int current_fragment = 0;
  while (current_fragment < number_of_fragments)
  {
    // Set up data that we will be sending across
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds << header.version;
    ds << header.payload_type;
    ds << header.seq_num;
    ds << header.total_fragments;
    ds << current_fragment;
    ds << header.timestamp;
    ds << header.audio_len;
    ds << header.video_len;
    ds << header.source_name;

    // TODO: SLICE UP
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

    current_fragment += 1;
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
  ds >> header.total_fragments;
  ds >> header.fragment_num;
  ds >> header.timestamp;
  ds >> header.audio_len;
  ds >> header.video_len;
  strcpy(header.source_name, data.sliced(sizeof(MFTP_Header) - 11, 12).data());

  if (header.audio_len + header.video_len <= 0)
    return true;

  // If we have a payload, process it
  data.slice(sizeof(MFTP_Header), header.audio_len + header.video_len);

  if (header.audio_len + header.video_len != data.size())
  {
    log("Expected length of audio/video payload not received: %d vs %d.",
        (header.audio_len + header.video_len), data.size(), ll::WARNING);
    return false;
  }

  payload.audio = data.sliced(0, header.audio_len);
  payload.video = data.sliced(header.audio_len, header.video_len);

  return true;
}
