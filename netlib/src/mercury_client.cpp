#include "mercury_client.hpp"

AV_Frame MercuryClient::retrieve_next_frame()
{
  AV_Frame frame;
  return frame;
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
  m_mftp_sock->connectToHost(host, mftp_port);

  if (m_hstp_sock->waitForConnected(2000) &&
      m_mftp_sock->waitForConnected(2000))
  {
    std::string address = host.toString().toStdString();
    log("Connected to client at %s with HSTP: %d, MFTP: %d", address.c_str(),
        hstp_port, mftp_port, ll::NOTE);
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
  return false;
}

void MercuryClient::process_received_hstp_messages()
{
  // read everything given
  m_hstp_buffer.append(m_hstp_sock->readAll());
  while (m_hstp_buffer.size() >= HSTP_HEADER_SIZE)
  {
    // get size of options
    quint16 opt_size =
        (static_cast<uchar>(m_hstp_buffer[ALIAS_SIZE - 1 + 1]) << 8) |
        static_cast<uchar>(m_hstp_buffer[ALIAS_SIZE - 1 + 2]);
    opt_size = ntohs(opt_size);

    if (m_hstp_buffer.size() >= HSTP_HEADER_SIZE + opt_size)
    {
      qint16 bytes_processed = process_single_hstp_message(opt_size);
      printf("I proccessed this data!: %d\n", bytes_processed);
      m_hstp_buffer.remove(0, bytes_processed);
    }
  }
  emit test_readyread_callback();
}

qint16 MercuryClient::process_single_hstp_message(qint16 opt_size)
{
  std::shared_ptr<QByteArray> buff = std::make_shared<QByteArray>();

  if (m_hstp_buffer.size() >= HSTP_HEADER_SIZE + opt_size)
  {
    buff->append(m_hstp_buffer.mid(0, HSTP_HEADER_SIZE + opt_size));
  }
  else
  {
    log("Issue reading HSTP single packet", ll::ERROR);
    return -1;
  }

  std::shared_ptr<HSTP_Header> hdr_ptr = m_hstp_handler.bytes_to_msg(buff);
  m_hstp_processor_ptr->process(hdr_ptr);
  return buff->size();
}

void MercuryClient::connect_signals_and_slots()
{
  connect(m_hstp_sock.get(), &QTcpSocket::readyRead, this,
          &MercuryClient::process_received_hstp_messages);
}
