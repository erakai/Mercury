#pragma once

#include <QAudioBuffer>
#include <QPixmap>
#include <QTcpServer>
#include <QUdpSocket>
#include <memory>

/*
This struct helps us keep track of our client list. It is low level and not
meant to be exposed to the actual application.
*/
struct Client
{
  // Here, id is an arbitrary unique identifier the server assigns every
  // incoming connection. Alias is the client's alias.
  int id = -1;
  char alias[18] = {0};

  // This is a regular pointer because QTcpServer generates it.
  QTcpSocket *hstp_sock = nullptr;

  // This is a shared pointer because we create it and we are NOT
  // using raw pointers in this household.
  std::shared_ptr<QUdpSocket> mftp_sock;

  // Keeps track of the sequence number of the protocol.
  uint32_t frame_seq_num = 0;
};

class MercuryServer : public QTcpServer
{
  Q_OBJECT;

public:
  MercuryServer(int port) : port(port) { connect_signals_and_slots(); };

  /*
    Starts the server on the given port, meaning it will accept inbound
    HSTP connnections and add them to its client list.
  */
  void start_server();

  /*
  Closes the server. Sends a closing message to every client on its list
  before removing them and closing all socokets. Frees any memory.
  */
  void close_server();

  /*
  Given some client alias, returns the associated id. Returns -1 if unable
  to identify the client.
  */
  int convert_alias_to_id(char *alias);

  /*
  Control connection helpers (KRIS THIS ON YOU)
  */
  // void handle_chat_message(...);
  // void handle_annotation_datagram(...);

public slots:
  /*
  This will be connected to the newConnection signal the QTcpServer emits. It
  will acquire the next connected QTcpSocket*, and then create a new client
  struct to add to the list.
  */
  void new_client_connection();

  /*
  Forcibly disconnects a client given their id. Sends a closing message
  before removing them from the client list.
  */
  void force_disconnect_client(int id);

  /*
  This is connected to the readyRead signal of every QTcpSocket. When some
  HSTP message is received from any client, it triggers, identifies the options,
  and then calls the relevant "handle" helpers to handle them.
  */
  void process_received_hstp_messages(int id);

  /*
  Serializes a audio/video payload and then distributes it to every
  client in the client list.
  */
  void send_frame(char source[12], QAudioBuffer audio, QPixmap video);

signals:
  /*
  This is emitted whenever a QTcpSocket emits stateChanged (connect signal to
  signal).
  */
  void client_disconnect(int id, char *alias);

  /*
  This is emitted whenever a client is added to the client list.
  */
  void client_connect(int id, char *alias);

private:
  /*
  Private method to help set up the signals/slots of the server.
  */
  void connect_signals_and_slots();

  int port;
  std::unordered_map<int, Client> clients;
};
