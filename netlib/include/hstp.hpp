#pragma once

#include <QTcpSocket>

/*
Packet formatting.

HSTP must be designed to allow for modular information, as the control messages
sent will expand in scope. To do this, we send over a preliminary header, which
contains data universal in control messages. Appended to this header is a
dynamic number of options. Each options is specified by a type, gives a length,
and contains data specific to that option.

These options will be documented in docs/hstp_options.txt.
They are inspired by DHCP options: https://www.ietf.org/rfc/rfc2132.txt.
*/

struct Option
{
  uint8_t type;
  uint8_t len; // in bytes
  uint8_t *data;
};

struct HSTP_Header
{
  char sender_alias[18];
  uint8_t option_count;
  Option options[];
};

/*
Host-side functionality.
*/

void send_packet(QTcpSocket sock, HSTP_Header header);

/*
Client-side functionality.
*/

void recv_packet(QTcpSocket sock, HSTP_Header &header);
