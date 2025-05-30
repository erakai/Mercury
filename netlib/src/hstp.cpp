#include "hstp.hpp"

#include <QtCore/qendian.h>
#include <QtCore/qstringview.h>
#include <cstdint>
#include <cstring>

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
    qCritical("Unable to add option, uninitalized message.");
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

bool HstpHandler::add_option_establishment(bool is_start, uint16_t port,
                                           const QByteArray &password)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = 1;
  opt.len = 36;
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);
  opt.data[0] = is_start ? 0 : 1;
  uint16_t network_port = qToBigEndian((uint16_t) port);
  std::memcpy(&opt.data[1], &network_port, sizeof(uint16_t));

  bool has_password = !password.isNull() && password.size() == 32;
  opt.data[3] = has_password ? 1 : 0;

  if (has_password)
    std::memcpy(&opt.data[4], password.constData(), 32);
  else
  {
    std::memset(&opt.data[4], '\0', 32);
  }

  m_hdr->options.push_back(opt);

  return true;
}
bool HstpHandler::add_option_chat(const char alias_of_chatter[ALIAS_SIZE],
                                  const char *chat_msg)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
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
  uint32_t net_len_of_msg = qToBigEndian((uint32_t) str_chat_msg.length());
  std::memcpy(opt.data.get() + ALIAS_SIZE, &net_len_of_msg, sizeof(uint32_t));
  std::memcpy(opt.data.get() + ALIAS_SIZE + sizeof(uint32_t),
              str_chat_msg.c_str(), str_chat_msg.length());

  m_hdr->options.push_back(opt);

  return true;
}

bool HstpHandler::add_option_annotation(const HSTP_Annotation &annotation)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = 5; // im using 5 for now
  opt.len = sizeof(uint16_t) + annotation.points.size() * (sizeof(float) * 2) +
            sizeof(annotation.red) + sizeof(annotation.green) +
            sizeof(annotation.blue) + sizeof(annotation.thickness) +
            sizeof(annotation.mode);
  opt.data = annotation.serialize();

  m_hdr->options.push_back(opt);

  return true;
}

bool HstpHandler::add_option_clear_annotations()
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = 13;
  opt.len = 0;
  m_hdr->options.push_back(opt);
  return true;
}

bool HstpHandler::add_option_enable_annotations(bool enabled)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = 14;
  opt.len = 1;
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  char *ptr = opt.data.get();
  std::memcpy(ptr, &enabled, sizeof(bool));
  m_hdr->options.push_back(opt);
  return true;
}

bool HstpHandler::add_option_reaction(uint32_t reaction)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = 11;
  opt.len = sizeof(uint32_t);
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  uint32_t net_int = qToBigEndian((uint32_t) reaction);
  std::memcpy(opt.data.get(), &net_int, sizeof(uint32_t));

  m_hdr->options.push_back(opt);

  return true;
}

bool HstpHandler::add_option_performance_request(uint64_t time)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = 8;
  opt.len = sizeof(uint64_t);
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  uint64_t net_int = qToBigEndian((uint64_t) time);
  std::memcpy(opt.data.get(), &net_int, sizeof(uint64_t));

  m_hdr->options.push_back(opt);

  return true;
}

bool HstpHandler::add_option_performance_metrics(uint16_t latency,
                                                 uint32_t throughput,
                                                 float loss, float fps)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = 9;
  opt.len = 14;
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  /*
    - uint16_t latency
    - uint32_t throughput
    - float loss
    - float fps
  */

  uint16_t net_latency = qToBigEndian(latency);
  std::memcpy(opt.data.get(), &net_latency, sizeof(uint16_t));

  uint32_t net_throughput = qToBigEndian(throughput);
  std::memcpy(opt.data.get() + 2, &net_throughput, sizeof(uint32_t));

  float net_loss = qToBigEndian(loss);
  std::memcpy(opt.data.get() + 6, &net_loss, sizeof(float));

  float net_fps = qToBigEndian(fps);
  std::memcpy(opt.data.get() + 10, &net_fps, sizeof(float));

  m_hdr->options.push_back(opt);

  return true;
}

std::shared_ptr<QByteArray> HstpHandler::output_msg()
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    qCritical("Unable to emit message, uninitalized message.");
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
      qCritical("Failed to write all HSTP message bytes");
      return false;
    }
  }
  else
  {
    qCritical("Failed to output HSTP message to socket, issue with message or "
              "socket");
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
        (3 + opt.len); // 1 bye for type, 2 byte for len, opt.len bytes for data
  }
  ds << qToBigEndian((uint16_t) size_opts);

  // write # of options
  uint8_t s = hdr->options.size();
  ds << s;

  // write options
  for (const auto &opt : hdr->options)
  {
    ds << opt.type;
    ds << qToBigEndian((uint16_t) opt.len);
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

    uint16_t net_len;
    ds >> net_len;
    opt.len = qFromBigEndian(net_len);
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
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  std::string str(gen_str);
  if (str.size() > MAX_STRING_SIZE)
  {
    str = str.substr(0, MAX_STRING_SIZE);
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
    qCritical("Unable to add option, uninitalized message.");
    return false;
  }

  Option opt;
  opt.type = type;
  opt.len = sizeof(uint32_t);
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);

  uint32_t net_int = qToBigEndian((uint32_t) uint32);
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
  while ((long long unsigned int) m_hstp_buffer.size() >= HSTP_HEADER_SIZE)
  {
    // get size of options
    quint16 opt_size =
        (static_cast<uchar>(m_hstp_buffer[ALIAS_SIZE - 1 + 1]) << 8) |
        static_cast<uchar>(m_hstp_buffer[ALIAS_SIZE - 1 + 2]);
    opt_size = qFromBigEndian((uint16_t) opt_size);

    if ((long long unsigned int) m_hstp_buffer.size() >=
        HSTP_HEADER_SIZE + opt_size)
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
    case 3: // stream title
      handle_stream_title(hdr_ptr->sender_alias, opt);
      break;
    case 4: // viewer count
      handle_viewer_count(hdr_ptr->sender_alias, opt);
      break;
    case 5: // annotation
      handle_annotation((hdr_ptr->sender_alias), opt);
      break;
    case 7: // fps
      handle_fps(hdr_ptr->sender_alias, opt);
      break;
    case 8: // performance request
      handle_performance_request(hdr_ptr->sender_alias, opt);
      break;
    case 9: // performance metrics
      handle_performance_metrics(hdr_ptr->sender_alias, opt);
      break;
    case 10: // stream start time
      handle_stream_start_time(hdr_ptr->sender_alias, opt);
      break;
    case 11: // reaction
      handle_reaction(hdr_ptr->sender_alias, opt);
      break;
    case 12: // reaction permission
      handle_reaction_permission(hdr_ptr->sender_alias, opt);
      break;
    case 13:
      handle_clear_annotations(hdr_ptr->sender_alias, opt);
      break;
    case 14:
      handle_enable_annotations(hdr_ptr->sender_alias, opt);
      break;
    default:
      handle_default(hdr_ptr->sender_alias, opt);
    }
  }
}

qint16 HstpProcessor::process_single_hstp_message(qint16 opt_size)
{
  std::shared_ptr<QByteArray> buff = std::make_shared<QByteArray>();

  if ((long long unsigned int) m_hstp_buffer.size() >=
      HSTP_HEADER_SIZE + opt_size)
  {
    buff->append(m_hstp_buffer.mid(0, HSTP_HEADER_SIZE + opt_size));
  }
  else
  {
    qWarning("Issue reading HSTP single packet");
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
  qInfo("%s", msg.c_str());
  emit received_echo(alias, msg);
}

void HstpProcessor::handle_establishment(const char alias[18],
                                         const Option &opt)
{
  if (!opt.data || opt.len != 36)
  {
    qCritical("Something went wrong with handling an establishment option...");
    handle_default(alias, opt);
    return;
  }

  bool is_start = static_cast<uint8_t>(opt.data[0]) == 0 ? true : false;

  uint16_t mftp_port;
  std::memcpy(&mftp_port, &opt.data[1], sizeof(uint16_t));
  mftp_port = qFromBigEndian((uint16_t) mftp_port);

  bool has_password = static_cast<uint8_t>(opt.data[3]) == 1 ? true : false;

  if (!has_password)
  {
    emit received_establishment(alias, is_start, mftp_port);
    return;
  }

  // get password
  QByteArray password = QByteArray::fromRawData(&opt.data[4], 32);
  emit received_establishment(alias, is_start, mftp_port, password);
}

void HstpProcessor::handle_chat(HANDLER_PARAMS)
{
  if (!opt.data || opt.len == 0)
  {
    qCritical("Something went wrong with handling a chat option...");
    handle_default(alias, opt);
    return;
  }

  char alias_of_chatter[ALIAS_SIZE] = {0};
  uint32_t len_of_message;

  std::memcpy(&alias_of_chatter, opt.data.get(), sizeof(char) * ALIAS_SIZE);
  std::memcpy(&len_of_message, opt.data.get() + ALIAS_SIZE, sizeof(uint32_t));

  len_of_message = qFromBigEndian((uint32_t) len_of_message);
  if (len_of_message == 0)
  {
    std::cout << opt << std::endl;
    qCritical("Something went wrong with handling chat message");
    return;
  }

  std::string chat_msg(opt.data.get() + ALIAS_SIZE + sizeof(uint32_t),
                       len_of_message);

  if (chat_msg.empty() || std::strlen(alias_of_chatter) == 0)
  {
    qCritical("Something went wrong with handling chat message");
    return;
  }

  emit received_chat(alias, alias_of_chatter, chat_msg);
}

void HstpProcessor::handle_annotation(HANDLER_PARAMS)
{
  // qDebug() << "HstpProcessor handling annotations";

  if (!opt.data || opt.len == 0)
  {
    qCritical("Something went wrong with handling an annotation option...");
    handle_default(alias, opt);
    return;
  }

  HSTP_Annotation annotation(opt.data);

  emit received_annotation(alias, annotation);
}

void HstpProcessor::handle_clear_annotations(HANDLER_PARAMS)
{
  emit received_clear_annotations();
}

void HstpProcessor::handle_enable_annotations(HANDLER_PARAMS)
{
  if (!opt.data || opt.len == 0)
  {
    qCritical("Something went wrong with handling enable_annotations");
    return;
  }

  bool enabled;
  std::memcpy(&enabled, opt.data.get(), sizeof(bool));

  emit received_enable_annotations(enabled);
}

void HstpProcessor::handle_performance_request(HANDLER_PARAMS)
{
  if (!opt.data || opt.len == 0)
  {
    qCritical(
        "Something went wrong with handling a performance request option...");
    handle_default(alias, opt);
    return;
  }

  uint64_t time;
  std::memcpy(&time, opt.data.get(), sizeof(uint64_t));
  time = qFromBigEndian((uint64_t) time);

  emit received_performance_request(alias, time);
}

void HstpProcessor::handle_performance_metrics(HANDLER_PARAMS)
{
  if (!opt.data || opt.len == 0)
  {
    qCritical(
        "Something went wrong with handling a performance metrics option...");
    handle_default(alias, opt);
    return;
  }

  uint16_t latency;
  uint32_t throughput;
  float loss;
  float fps;

  std::memcpy(&latency, opt.data.get(), sizeof(uint16_t));
  std::memcpy(&throughput, opt.data.get() + 2, sizeof(uint32_t));
  std::memcpy(&loss, opt.data.get() + 6, sizeof(float));
  std::memcpy(&fps, opt.data.get() + 10, sizeof(float));

  latency = qFromBigEndian(latency);
  throughput = qFromBigEndian(throughput);
  loss = qFromBigEndian(loss);
  fps = qFromBigEndian(fps);

  emit received_performance_metrics(alias, latency, throughput, loss, fps);
}

void HstpProcessor::handle_string(
    HANDLER_PARAMS, void (HstpProcessor::*signal)(const char alias[ALIAS_SIZE],
                                                  const char *str))
{
  if (!opt.data || opt.len == 0)
  {
    qCritical("Something went wrong with handling a string option...");
    handle_default(alias, opt);
    return;
  }

  std::string str(opt.data.get());
  emit(this->*signal)(alias, str.c_str());
}

void HstpProcessor::handle_uint32(
    HANDLER_PARAMS, void (HstpProcessor::*signal)(const char alias[ALIAS_SIZE],
                                                  uint32_t uint32))
{
  if (!opt.data || opt.len == 0)
  {
    qCritical("Something went wrong with handling a uint32 option...");
    handle_default(alias, opt);
    return;
  }

  uint32_t uint32;
  std::memcpy(&uint32, opt.data.get(), sizeof(uint32_t));
  uint32 = qFromBigEndian((uint32_t) uint32);

  emit(this->*signal)(alias, uint32);
}
