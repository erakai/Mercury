#include "hstp.hpp"
#include <cstdint>
#include <memory>
#include <string>

bool HstpHandler::init_msg(char sender_alias[18])
{
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

  return true;
}

bool HstpHandler::add_option_chat(const char *chat_msg)
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to add option, uninitalized message.", ll::ERROR);
    return false;
  }

  std::string str_chat_msg = std::string(chat_msg);
  if (str_chat_msg.length() > CHAT_SIZE_LIMIT)
  {
    str_chat_msg = str_chat_msg.substr(0, CHAT_SIZE_LIMIT);
  }
  Option opt;
  opt.type = 0;
  opt.len = str_chat_msg.length();
  opt.data = std::shared_ptr<char[]>(new char[opt.len]);
  std::memcpy(opt.data.get(), str_chat_msg.c_str(), str_chat_msg.length());

  m_hdr->options.push_back(opt);

  return true;
}

std::shared_ptr<QByteArray> HstpHandler::output_msg()
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to emit message, uninitalized message.", ll::ERROR);
    return nullptr;
  }

  std::shared_ptr<QByteArray> bytes = _serialize(m_hdr);
  clear_msg();
  return bytes;
}

std::shared_ptr<HSTP_Header> HstpHandler::bytes_to_msg(QByteArray &buff)
{
  std::shared_ptr<QByteArray> buff = std::shared_ptr<QByteArray>(buff);
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

  ds.writeRawData(hdr->sender_alias, 18);

  uint8_t s = hdr->options.size();
  ds << s;

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
  ds.readRawData(hdr->sender_alias, 18);

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

void HstpProcessor::process(const std::shared_ptr<HSTP_Header> &hdr_ptr)
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

void HstpProcessor::handle_default(HANDLER_PARAMS)
{
  std::shared_ptr<HSTP_Header> hdr_ptr = std::make_shared<HSTP_Header>();
  std::strcpy(hdr_ptr->sender_alias, alias);
  hdr_ptr->options.push_back(opt);

  emit emit_generic(hdr_ptr);
}

void HstpProcessor::handle_echo(HANDLER_PARAMS)
{
  std::string msg(opt.data.get());
  log(msg.c_str(), ll::NOTE);

  emit emit_echo(alias, msg);
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

  emit emit_establishment(alias, is_start, mftp_port);
}

void HstpProcessor::handle_chat(HANDLER_PARAMS)
{
  if (!opt.data && opt.len != 0)
  {
    log("Something went wrong with handling a chat option...", ll::ERROR);
    handle_default(alias, opt);
    return;
  }

  std::string msg(opt.data.get());
  emit emit_chat(alias, msg);
}
