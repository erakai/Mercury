#include "hstp.hpp"
#include "logger.hpp"
#include "mftp.hpp"
#include <QtCore/qstringview.h>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sys/_endian.h>

bool HstpHandler::connect(QHostAddress addr, int port)
{
  m_qtcp_sock->connectToHost(addr, port);

  if (m_qtcp_sock->waitForConnected())
  {
    return true;
  }

  return false;
}

void HstpHandler::close()
{
  m_qtcp_sock->close();
}

bool HstpHandler::init_msg(char sender_alias[18])
{
  strcpy(m_hdr->sender_alias, sender_alias);

  m_current_status = MSG_STATUS::IN_PROGRESS;

  return true;
}

bool HstpHandler::add_option_echo(char *msg)
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
  opt.data = str_msg.c_str();
  m_hdr->options.push_back(opt);

  return false;
}

bool HstpHandler::send_msg()
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to send message, message not in progress", ll::ERROR);
    return false;
  }

  return true;
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

QByteArray HstpHandler::_serialize(HSTP_Header &hdr)
{
  QByteArray byte_array;

  byte_array.push_back(hdr.sender_alias);
  for (int i = 0; i < ALIAS_SIZE - strlen(hdr.sender_alias); i++)
  {
    byte_array.push_back('\0');
  }

  byte_array.push_back((uint8_t) hdr.options.size());

  for (const auto &opt : hdr.options)
  {
    byte_array.push_back(opt.type);
    byte_array.push_back(opt.len);
    byte_array.push_back(opt.data);
  }

  return byte_array;
}

HSTP_Header HstpHandler::_deserialize(const QByteArray &buff)
{
  HSTP_Header hdr;

  QByteArray::const_iterator it = buff.begin();

  std::string alias_str;
  for (int i = 0; i < ALIAS_SIZE; i++)
  {
    it++;
    alias_str[i] = (char) *it;
  }
  return hdr;
}

bool HstpHandlerTester::test_serialize(HstpHandler &handler)
{
  HSTP_Header hdr;
  Option echo_option;

  strcpy(hdr.sender_alias, "Alex");
  echo_option.type = 1;
  echo_option.len = 7;
  echo_option.data = "hello";

  hdr.options.push_back(echo_option);

  QByteArray byte_array;
  byte_array = handler._serialize(hdr);

  std::string out;
  for (QByteArray::iterator it = byte_array.begin(); it != byte_array.end();
       it++)
  {
    if (*it == '\0')
      out.push_back('0');
    else
      out.push_back(*it);
  }

  log(out.c_str(), ll::NOTE);

  return true;
}
