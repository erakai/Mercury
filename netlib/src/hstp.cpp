#include "hstp.hpp"
#include <cstdint>
#include <cstring>
#include <sys/_endian.h>

bool HstpHandler::init_msg(const char sender_alias[18])
{
  m_hdr = std::make_shared<HSTP_Header>();
  strcpy(m_hdr->sender_alias, sender_alias);
  m_current_status = MSG_STATUS::IN_PROGRESS;

  return true;
}

bool HstpHandler::add_option_echo(const char *msg)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to add option, uninitalized message.", ll::ERROR);
    return false;
  }

  std::string str_msg = std::string(msg);
  Option opt;
  opt.type = 0;
  opt.len = str_msg.length();
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);
  std::memcpy(opt.data.get(), msg, str_msg.length());

  m_hdr->options.push_back(opt);

  return true;
}

bool HstpHandler::add_option_establishment(bool is_start, uint16_t port)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to add option, uninitalized message.", ll::ERROR);
    return false;
  }

  Option opt;
  opt.type = 1;
  opt.len = 3;
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);
  opt.data[0] = is_start ? 0 : 1;
  uint16_t network_port = htons(port);
  std::memcpy(&opt.data[1], &network_port, sizeof(uint16_t));

  m_hdr->options.push_back(opt);

  return true;
}
bool HstpHandler::add_option_chat(const char alias_of_chatter[ALIAS_SIZE],
                                  const char *chat_msg)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to add option, uninitalized message.", ll::ERROR);
    return false;
  }

  // First construct the three different variables we are serializing
  char alias_buffer[ALIAS_SIZE] = {0};
  std::memcpy(alias_buffer, alias_of_chatter, strlen(alias_of_chatter));

  std::string str_chat_msg = std::string(chat_msg);
  if (str_chat_msg.length() > CHAT_SIZE_LIMIT)
    str_chat_msg = str_chat_msg.substr(0, CHAT_SIZE_LIMIT);

  // Then create the option and copy the adta into it
  Option opt;
  opt.type = 2;
  opt.len = ALIAS_SIZE + sizeof(uint32_t) + str_chat_msg.length();
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  std::memcpy(opt.data.get(), alias_buffer, ALIAS_SIZE);
  uint32_t net_len_of_msg = htonl(str_chat_msg.length());
  std::memcpy(opt.data.get() + ALIAS_SIZE, &net_len_of_msg, sizeof(uint32_t));
  std::memcpy(opt.data.get() + ALIAS_SIZE + sizeof(uint32_t),
              str_chat_msg.c_str(), str_chat_msg.length());

  m_hdr->options.push_back(opt);

  return true;
}

std::shared_ptr<QByteArray> HstpHandler::output_msg()
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to emit message, uninitalized message.", ll::ERROR);
    clear_msg();
    return nullptr;
  }

  std::shared_ptr<QByteArray> bytes = _serialize(m_hdr);
  clear_msg();
  return bytes;
}

bool HstpHandler::output_msg_to_socket(const std::shared_ptr<QTcpSocket> &sock)
{
  std::shared_ptr<QByteArray> establish_msg = output_msg();
  if (!establish_msg->isEmpty() && sock &&
      sock->state() == QAbstractSocket::ConnectedState && sock->isWritable())
  {
    if (sock->write(*establish_msg) == establish_msg->size())
    {
      return true;
    }
    else
    {
      log("Failed to write all HSTP message bytes", ll::ERROR);
      return false;
    }
  }
  else
  {
    log("Failed to output HSTP message to socket, issue with message or socket",
        ll::ERROR);
    return false;
  }
}

std::shared_ptr<HSTP_Header>
HstpHandler::bytes_to_msg(const std::shared_ptr<QByteArray> &buff)
{
  return _deserialize(buff);
}

void HstpHandler::clear_msg()
{
  strcpy(m_hdr->sender_alias, "");
  m_hdr->options.clear();
  m_current_status = MSG_STATUS::READY;
}

MSG_STATUS HstpHandler::get_status()
{
  return m_current_status;
}

std::shared_ptr<QByteArray>
HstpHandler::_serialize(const std::shared_ptr<HSTP_Header> &hdr)
{
  std::shared_ptr<QByteArray> byte_array_ptr = std::make_shared<QByteArray>();
  QDataStream ds(byte_array_ptr.get(), QIODevice::WriteOnly);

  // write alias
  ds.writeRawData(hdr->sender_alias, 18);

  // write size of options
  uint16_t size_opts = 0;
  for (const auto &opt : hdr->options)
  {
    size_opts +=
        (2 + opt.len); // 1 bye for type, 1 byte for len, opt.len bytes for data
  }
  ds << htons(size_opts);

  // write # of options
  uint8_t s = hdr->options.size();
  ds << s;

  // write options
  for (const auto &opt : hdr->options)
  {
    ds << opt.type;
    ds << opt.len;
    ds.writeRawData(opt.data.get(), opt.len);
  }

  return byte_array_ptr;
}

std::shared_ptr<HSTP_Header>
HstpHandler::_deserialize(const std::shared_ptr<QByteArray> &buff)
{

  std::shared_ptr<HSTP_Header> hdr = std::make_shared<HSTP_Header>();
  QDataStream ds(buff.get(), QIODevice::ReadOnly);

  // get alias
  ds.readRawData(hdr->sender_alias, ALIAS_SIZE);

  // get option len (no point in storing it cause we can get that from the
  // vector fairly easily)
  uint16_t opt_len;
  ds >> opt_len;

  // get option count
  uint8_t opt_count;
  ds >> opt_count;

  // read in all options
  for (int i = 0; i < opt_count; i++)
  {
    Option opt;
    ds >> opt.type;
    ds >> opt.len;
    opt.data = std::shared_ptr<char[]>(new char[opt.len]);
    ds.readRawData(opt.data.get(), opt.len);

    hdr->options.push_back(opt);
  }

  return hdr;
}

bool HstpHandler::add_option_generic_string(uint8_t type, const char *gen_str)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to add option, uninitalized message.", ll::ERROR);
    return false;
  }

  std::string str(gen_str);
  if (str.size() > MAX_STRING_SIZE)
  {
    str.substr(0, MAX_STRING_SIZE);
  }

  Option opt;
  opt.type = type;
  opt.len = str.length();
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  std::memcpy(opt.data.get(), str.c_str(), opt.len);

  m_hdr->options.push_back(opt);

  return true;
}

bool HstpHandler::add_option_generic_uint32(uint8_t type, uint32_t uint32)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to add option, uninitalized message.", ll::ERROR);
    return false;
  }

  Option opt;
  opt.type = 4;
  opt.len = sizeof(uint32_t);
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  uint32_t net_int = htonl(uint32);
  std::memcpy(opt.data.get(), &net_int, sizeof(uint32_t));

  m_hdr->options.push_back(opt);

  return true;
}

void HstpProcessor::process(const QByteArray &chunk)
{
  if (chunk.isNull() || chunk.isEmpty())
  {
    return;
  }

  m_hstp_buffer.append(chunk);
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
      m_hstp_buffer.remove(0, bytes_processed);
    }
  }
}

void HstpProcessor::emit_header(const std::shared_ptr<HSTP_Header> &hdr_ptr)
{
  /*
   * The CIA wants you think that the switch statement is nothing but if
   * then else.
   */
  for (const auto &opt : hdr_ptr->options)
  {
    switch (opt.type)
    {
    case 0: // echo
      handle_echo(hdr_ptr->sender_alias, opt);
      break;
    case 1: // establishment
      handle_establishment(hdr_ptr->sender_alias, opt);
      break;
    case 2: // chat
      handle_chat(hdr_ptr->sender_alias, opt);
      break;
    default:
      handle_default(hdr_ptr->sender_alias, opt);
    }
  }
}

qint16 HstpProcessor::process_single_hstp_message(qint16 opt_size)
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

  std::shared_ptr<HSTP_Header> hdr_ptr = HstpHandler::bytes_to_msg(buff);
  emit_header(hdr_ptr);
  return buff->size();
}
void HstpProcessor::handle_default(HANDLER_PARAMS)
{
  std::shared_ptr<HSTP_Header> hdr_ptr = std::make_shared<HSTP_Header>();
  std::strcpy(hdr_ptr->sender_alias, alias);
  hdr_ptr->options.push_back(opt);

  emit received_generic(hdr_ptr);
}

void HstpProcessor::handle_echo(HANDLER_PARAMS)
{
  std::string msg(opt.data.get());
  log(msg.c_str(), ll::NOTE);

  emit received_echo(alias, msg);
}

void HstpProcessor::handle_establishment(const char alias[18],
                                         const Option &opt)
{
  if (!opt.data || opt.len != 3)
  {
    log("Something went wrong with handling an establishment option...",
        ll::ERROR);
    handle_default(alias, opt);
    return;
  }

  bool is_start = static_cast<uint8_t>(opt.data[0]) == 0 ? true : false;

  uint16_t mftp_port;
  std::memcpy(&mftp_port, &opt.data[1], sizeof(uint16_t));
  mftp_port = ntohs(mftp_port);

  emit received_establishment(alias, is_start, mftp_port);
}

void HstpProcessor::handle_chat(HANDLER_PARAMS)
{
  if (!opt.data || opt.len == 0)
  {
    log("Something went wrong with handling a chat option...", ll::ERROR);
    handle_default(alias, opt);
    return;
  }

  char alias_of_chatter[ALIAS_SIZE] = {0};
  uint32_t len_of_message;

  std::memcpy(&alias_of_chatter, opt.data.get(), sizeof(char) * ALIAS_SIZE);
  std::memcpy(&len_of_message, opt.data.get() + ALIAS_SIZE, sizeof(uint32_t));
  if (len_of_message == 0)
  {
    log("Something went wrong with handling chat message", ll::ERROR);
    return;
  }

  len_of_message = ntohl(len_of_message);

  std::string chat_msg(opt.data.get() + ALIAS_SIZE + sizeof(uint32_t));

  if (chat_msg.empty() || std::strlen(alias_of_chatter) == 0)
  {
    log("Something went wrong with handling chat message", ll::ERROR);
    return;
  }

  emit received_chat(alias, alias_of_chatter, chat_msg);
}
