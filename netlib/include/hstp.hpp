#pragma once

#include <QTcpSocket>

/*
Packet formatting
*/

struct HSTP_Header
{
};

/*
General usage.
*/

QTcpSocket acquire_hstp_socket();

/*
Host-side functionality.
*/

void send_packet(QTcpSocket sock, HSTP_Header header);

/*
Client-side functionality.
*/

void recv_packet(QTcpSocket sock, HSTP_Header &header);
