#pragma once

#include "mftp.hpp"

#include <QTcpServer>
#include <QUdpSocket>
#include <vector>

/*
This struct helps us keep track of our client list. It is low level and not
meant to be exposed to the actual application. Here, id is an arbitrary
unique identifier the server assigns every incoming connection.
*/
struct Client
{
  int id;
  char alias[18] = {0};

  QTcpSocket *hstp_sock;
  QUdpSocket *mftp_sock;

  std::vector<AV_Frame> jitter_buffer;
};

class MercuryServer : public QTcpServer
{
  Q_OBJECT;

public:
  MercuryServer();

  // Overall server control
  void start_server(int port);
  void close_server();

  // Media control
  void retrieve_next_client_frame(int id);

  // Control connection helpers
  // void handle_chat_message(...);
  // void handle_annotation_datagram(...);

public slots:
  // Connection management
  void new_client_connection(QTcpSocket *socket);
  void force_disconnect_client(int id);

  // MFTP slots
  void process_received_mftp_messages(int id);
  void send_frame(AV_Frame frame);

  // HSTP slots
  void process_received_hstp_messages(int id);

signals:
  // Client list control for app usage
  void client_disconnect(int id, char *alias);
  void client_connect(int id, char *alias);

private:
  std::unordered_map<int, Client> clients;
};
