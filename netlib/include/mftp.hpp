#include <QUdpSocket>

/*
Packet formatting
*/

struct MFTP_Packet
{
};

/*
General usage.
*/

QUdpSocket acquire_hstp_socket();

/*
Host-side functionality.
*/

void send_packet(QUdpSocket sock, MFTP_Packet packet);

/*
Client-side functionality.
*/

void recv_packet(QUdpSocket sock, MFTP_Packet &packet);
