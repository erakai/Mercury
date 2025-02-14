#include "hstp.hpp"
#include "logger.hpp"
#include <QtCore/qstringview.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <sys/_endian.h>

bool HstpHandler::connect(QHostAddress &addr, int port)
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

  return false;
}

bool HstpHandler::send_msg()
{
  if (get_status() != MSG_STATUS::IN_PROGRESS)
  {
    log("Unable to send message, message not in progress", ll::ERROR);
    return false;
  }

  QByteArray byte_array = _serialize(*m_hdr);

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
  QDataStream ds(&byte_array, QIODevice::WriteOnly);

  ds.writeRawData(hdr.sender_alias, 18);

  uint8_t s = hdr.options.size();
  ds << s;

  for (const auto &opt : hdr.options)
  {
    ds << opt.type;
    ds << opt.len;
    ds.writeRawData(opt.data.get(), opt.len);
  }

  return byte_array;
}

HSTP_Header HstpHandler::_deserialize(const QByteArray &buff)
{
  HSTP_Header hdr;
  QDataStream ds(buff);

  // get alias
  ds.readRawData(hdr.sender_alias, 18);

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

    hdr.options.push_back(opt);
  }

  return hdr;
}
