#include "mercury_client.hpp"
#include "mftp.hpp"
#include <memory>

JitterEntry MercuryClient::retrieve_next_frame()
{
  if (m_jitter_buffer.size() == 0)
  {
    JitterEntry entry;
    entry.seq_num = -1;
    entry.timestamp = -1;
    return entry;
  }

  JitterEntry first = m_jitter_buffer[0];
  m_jitter_buffer.pop_front();
  return first;
}

bool MercuryClient::establish_connection(const QHostAddress &host,
                                         quint16 hstp_port, quint16 mftp_port)
{
  if (host.isNull())
  {
    log("Invalid QHostAddress", ll::ERROR);
    return false;
  }

  if (hstp_port == 0 || mftp_port == 0)
  {
    log("Invalid ports", ll::ERROR);
  }

  m_hstp_sock->connectToHost(host, hstp_port);

  acquire_mftp_socket(m_mftp_sock, mftp_port);

  if (!m_mftp_sock || !m_mftp_sock->isValid())
  {
    log("Failed to acquire valid mftp socket", ll::ERROR);
    return false;
  }

  if (m_hstp_sock->waitForConnected(2000))
  {
    std::string address = host.toString().toStdString();
    log("Connected to client at %s with HSTP: %d, MFTP: %d", address.c_str(),
        hstp_port, mftp_port, ll::NOTE);

    // send establishment message
    m_hstp_handler.init_msg(m_alias.c_str());
    m_hstp_handler.add_option_establishment(true, mftp_port);
    if (!m_hstp_handler.output_msg_to_socket(m_hstp_sock))
    {
      log("Failed to send establishment message to socket", ll::ERROR);
      return false;
    }

    return true;
  }
  else
  {
    log("Failed connection for HSTP/TCP or MFTP/UDP after 2000 msec.",
        ll::ERROR);
    return false;
  }
}

bool MercuryClient::disconnect()
{
  m_mftp_sock->close();
  m_hstp_sock->close();
  return true;
}

bool MercuryClient::talk_tuah(const std::string &chat_msg)
{
  m_hstp_handler.init_msg(m_alias.c_str());
  m_hstp_handler.add_option_chat(m_alias.c_str(), chat_msg.c_str());
  if (!m_hstp_handler.output_msg_to_socket(m_hstp_sock))
  {
    log("Failed to send chat message to socket", ll::ERROR);
    return false;
  }

  return true;
}

void MercuryClient::process_received_hstp_messages()
{
  if (m_hstp_sock && m_hstp_sock->state() == QAbstractSocket::ConnectedState)
  {
    QByteArray data = m_hstp_sock->readAll();
    m_hstp_processor_ptr->process(data);
  }
  else
  {
    log("Issue processing received hstp message", ll::ERROR);
  }
}

void MercuryClient::connect_signals_and_slots()
{
  connect(m_hstp_sock.get(), &QTcpSocket::readyRead, this,
          &MercuryClient::process_received_hstp_messages);

  connect(m_mftp_sock.get(), &QUdpSocket::readyRead, this,
          [&]() { m_mftp_processor->process_ready_datagrams(m_mftp_sock); });

  connect(m_mftp_processor.get(), &MFTPProcessor::frame_ready, this,
          &MercuryClient::insert_into_jitter_buffer);
}

void MercuryClient::insert_into_jitter_buffer(MFTP_Header header,
                                              QAudioBuffer audio, QImage video)
{
  JitterEntry new_entry;
  new_entry.seq_num = header.seq_num;
  new_entry.timestamp = header.timestamp;
  new_entry.video = video;
  new_entry.audio = audio;

  for (auto it = m_jitter_buffer.rbegin(); it != m_jitter_buffer.rend(); ++it)
  {
    if (it->seq_num < header.seq_num)
    {
      m_jitter_buffer.insert(it.base(), new_entry);
      return;
    }
  }

  m_jitter_buffer.push_front(new_entry);
}
