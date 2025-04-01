#include <mftp.hpp>

#include <QVariant>
std::shared_ptr<QUdpSocket> acquire_mftp_socket(int port)
{
  std::shared_ptr<QUdpSocket> sock = std::make_shared<QUdpSocket>();

  acquire_mftp_socket(sock, port);

  return sock;
}

void acquire_mftp_socket(std::shared_ptr<QUdpSocket> sock, int port)
{
  // Bind socket to target port on this machine
  if (!sock->bind(QHostAddress::Any, port))
  {
    qCritical("Unable to bind MFTP socket.");
  }
  else
  {
    qInfo("MFTP Socket created and bound on port %d.", port);
  }

  sock->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 800000);
}

bool send_datagram(std::shared_ptr<QUdpSocket> sock,
                   std::vector<QHostAddress> dest_ip,
                   std::vector<int> dest_port, MFTP_Header &header,
                   QByteArray &video_bytes, QAudioBuffer audio)
{
  QByteArray payload_array;

  QByteArray audio_bytes = QByteArray(audio.constData<char>());
  payload_array.append(audio_bytes);
  payload_array.append(video_bytes);

  // Set header size
  header.audio_len = audio_bytes.size();
  header.video_len = video_bytes.size();

  // Assume MTU size is 1500 (for Ethernet). Divide up the data into that many
  // fragments
  int total_size = payload_array.size();
  int mtu = 1500 - (sizeof(MFTP_Header));
  int number_of_fragments = 1 + (total_size / mtu);
  header.total_fragments = number_of_fragments;

  int current_fragment = 0;
  int current_byte = 0;
  while (current_fragment < number_of_fragments)
  {
    // Set up data that we will be sending across
    header.fragment_num = current_fragment++;
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds << header.version;
    ds << header.payload_type;
    ds << header.seq_num;
    ds << header.total_fragments;
    ds << header.fragment_num;
    ds << header.timestamp;
    ds << header.audio_len;
    ds << header.video_len;
    data.append(header.source_name, 12);

    // Grab our slice of the payload
    int length_of_payload = mtu;
    if (total_size - current_byte < length_of_payload)
      length_of_payload = total_size - current_byte;

    if (total_size > 0)
    {
      QByteArray sliced_payload =
          payload_array.sliced(current_byte, length_of_payload);
      data.append(sliced_payload);
      current_byte += length_of_payload;
    }

    // Define datagram
    for (int i = 0; i < (int) dest_ip.size(); i++)
    {
      QNetworkDatagram dg(data, dest_ip[i], dest_port[i]);

      // Send
      if (sock->writeDatagram(dg) < 0)
      {
        qCritical("Datagram failed to send due to size (%lld bytes).",
                  data.size());
      }
    }
  }

  return true;
}

void MFTPProcessor::process_ready_datagrams(std::shared_ptr<QUdpSocket> socket,
                                            Metrics &metrics)
{
  while (socket->hasPendingDatagrams())
  {
    QNetworkDatagram datagram = socket->receiveDatagram();
    metrics.register_datagram_received();

    process_datagram(datagram, metrics);

    // See if we can release any of the frames
    bool released_any = false;
    for (int i = 0; i < MAX_FRAMES_TO_REASSEMBLE; i++)
    {
      PartialFrame partial = partial_frames[i];
      if (partial.remaining_fragments == 0)
      {
        release_complete_frame(i);

        released_any = true;
      }
    }

    // Shift remaining partial frames left if we released any
    if (released_any)
      fix_partial_frame_array();
  }
}

bool MFTPProcessor::process_datagram(QNetworkDatagram datagram,
                                     Metrics &metrics)
{
  // if new find first -1 in partial_frames and fill

  QByteArray data = datagram.data();
  metrics.register_data_received(data.size() * 8);
  MFTP_Header header = {0};

  if (data.size() < size_of_mftp_header())
  {
    qWarning("Invalid datagram received - only %lld bytes.", data.size());
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

  for (int i = 0; i < 12; i++)
    ds >> header.source_name[i];

  // Take remaining data
  data.slice(size_of_mftp_header(), data.size() - size_of_mftp_header());

  // Cases: either new frame and we have space, new frame and we don't have
  // space, or existing frame

  int first_free_frame = -1;
  for (int i = 0; i < MAX_FRAMES_TO_REASSEMBLE; i++)
  {
    PartialFrame &curr_frame = partial_frames[i];
    if (curr_frame.header.seq_num == header.seq_num)
    {
      // Case: Existing Frame
      curr_frame.remaining_fragments -= 1;

      if (curr_frame.remaining_fragments < 0)
      {
        qCritical("Received too many datagrams for a frame somehow.");
        return false;
      }

      curr_frame.datagrams[header.fragment_num] = data;
      return true;
    }
    else if (curr_frame.remaining_fragments == -1 && first_free_frame == -1)
    {
      first_free_frame = i;
    }
  }

  PartialFrame new_frame;
  new_frame.header = header;
  new_frame.remaining_fragments = header.total_fragments - 1;
  new_frame.datagrams.resize(header.total_fragments);
  new_frame.datagrams[header.fragment_num] = data;

  // For loss calculation
  metrics.register_expected_datagrams(header.total_fragments);

  if (first_free_frame == -1)
  {
    // Case: new frame and we don't have space
    if (partial_frames[0].header.seq_num > new_frame.header.seq_num)
    {
      qInfo("Received partial frame %d but oldest current frame is %d.",
            new_frame.header.seq_num, partial_frames[0].header.seq_num);
      return false;
    }

    qInfo("Dropping partial frame %d (had %d remaining fragments out of %d) in "
          "exchange for new frame %d.",
          partial_frames[0].header.seq_num,
          partial_frames[0].remaining_fragments,
          partial_frames[0].header.total_fragments, new_frame.header.seq_num);
    first_free_frame = 0;
  }

  // Case: new frame and we have space
  for (int i = first_free_frame; i < MAX_FRAMES_TO_REASSEMBLE - 1; i++)
  {
    partial_frames[i] = partial_frames[i + 1];
  }
  partial_frames[MAX_FRAMES_TO_REASSEMBLE - 1] = new_frame;

  return true;
}

void MFTPProcessor::release_complete_frame(int index)
{
  PartialFrame &frame = partial_frames[index];

  QByteArray payload;
  for (int i = 0; i < frame.header.total_fragments; i++)
    payload.append(frame.datagrams[i]);

  QByteArray audio_bytes = payload.sliced(0, frame.header.audio_len);
  QByteArray video_bytes = payload.sliced(
      frame.header.audio_len, frame.header.video_len + frame.header.audio_len);

  QImage video;
  video.loadFromData(video_bytes, "JPG");
  QAudioBuffer audio(audio_bytes, QAudioFormat());

  frame.remaining_fragments = -1;

  if (video.isNull())
  {
    qWarning("Received corrupted data - unable to load image frame.");
    return;
  }

  emit frame_ready(frame.header, audio, video);
}

void MFTPProcessor::fix_partial_frame_array()
{
  int write_index = 0;

  // Move valid frames to the leftmost position
  for (int read_index = 0; read_index < MAX_FRAMES_TO_REASSEMBLE; read_index++)
  {
    if (partial_frames[read_index].remaining_fragments != -1)
    {
      if (write_index != read_index)
        partial_frames[write_index] = partial_frames[read_index];
      write_index++;
    }
  }

  for (int i = write_index; i < MAX_FRAMES_TO_REASSEMBLE; i++)
    partial_frames[i] = PartialFrame();
}

void MFTPProcessor::print_partial_frame_array()
{
  std::string to_log("Partial Frame Array: ");
  for (int i = 0; i < MAX_FRAMES_TO_REASSEMBLE; i++)
  {
    PartialFrame f = partial_frames[i];
    if (f.header.seq_num == 0)
      to_log += ("0 ");
    else
      to_log += std::format("{}({}/{}) ", f.header.seq_num,
                            f.header.total_fragments - f.remaining_fragments,
                            f.header.total_fragments);
  }
  qInfo("%s", to_log.c_str());
}

void print_header(MFTP_Header header)
{
  qInfo("Datagram: Ver %d Type %d Seq #%d Total Frags %d Frag #%d Timestamp %d "
        "Audio Len %d Video Len %d Source %s",
        header.version, header.payload_type, header.seq_num,
        header.total_fragments, header.fragment_num, header.timestamp,
        header.audio_len, header.video_len, header.source_name);
}

int size_of_mftp_header()
{
  MFTP_Header header;
  return sizeof(header.version) + sizeof(header.payload_type) +
         sizeof(header.seq_num) + sizeof(header.total_fragments) +
         sizeof(header.fragment_num) + sizeof(header.timestamp) +
         sizeof(header.audio_len) + sizeof(header.video_len) + SOURCE_NAME_LEN;
}
