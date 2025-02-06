#include <QTcpSocket>

/*
Packet formatting
*/

struct HSTP_Packet
{
};

/*
General usage.
*/

QTcpSocket acquire_hstp_socket();

/*
Host-side functionality.
*/

void send_packet(QTcpSocket sock, HSTP_Packet packet);

/*
Client-side functionality.
*/

void recv_packet(QTcpSocket sock, HSTP_Packet &packet);
