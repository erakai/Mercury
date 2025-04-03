#pragma once

#include "netlib_global.h"
#include "hstp.hpp"
#include <QAudioBuffer>
#include <QTcpServer>
#include <QUdpSocket>
#include <QVideoFrame>

#include <QtCore/qstringview.h>
#include <chrono>
#include <memory>

/*
This struct helps us keep track of our client list. It is low level and not
meant to be exposed to the actual application.
*/
struct NETLIB_EXPORT Client
{
  // Validated - has sent server port, alias, and password, etc
  bool validated = false;

  // Here, id is an arbitrary unique identifier the server assigns every
  // incoming connection. Alias is the client's alias.
  int id = -1;
  char alias[18] = {0};

  // UDP socket the client gave to send MFTP messages to
  uint16_t mftp_port = 0;

  // HSTP utility classes
  HstpHandler handler;
  std::shared_ptr<HstpProcessor> processor; // must be pointer
  std::shared_ptr<QTcpSocket> hstp_sock;
};

class NETLIB_EXPORT MercuryServer : public QTcpServer
{
  Q_OBJECT;

public:
  MercuryServer(std::string alias, const QByteArray &pass = nullptr)
      : host_alias(alias), host_pass(pass)
  {
  }
  MercuryServer(std::string alias, int tcp_port, int udp_port,
                const QByteArray &pass = nullptr)
      : host_alias(alias), host_pass(pass), tcp_port(tcp_port),
        udp_port(udp_port) {};
  MercuryServer(MercuryServer &server)
      : host_alias(server.host_alias), host_pass(server.host_pass),
        tcp_port(server.tcp_port), udp_port(server.udp_port)
  {
  }

  /*
    Starts the server on the given port, meaning it will accept inbound
    HSTP connnections and add them to its client list.
  */
  bool start_server();

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
  Retrieve client list.
  */
  std::unordered_map<int, Client> get_clients();

  /*
  Getters and setters. Note the ports cannot be changed once the server has been
  started.
  */
  void set_ports(int tcp_port, int udp_port);
  int get_tcp_port() { return tcp_port; };
  int get_udp_port() { return udp_port; }
  Client &get_client(int id) { return clients[id]; }
  std::string get_alias() { return host_alias; }
  void set_compression(float comp)
  {
    if (comp < 0 || comp > 1)
    {
      qDebug("Compression set outside bound: %f", comp);
      return;
    }
    compression = comp;
  }
  bool get_compression() { return compression; }

  /*
  Sends a message to every client.
  */
  void send_hstp_message_to_all_clients(QByteArray msg);

public slots:
  /*
  This will be connected to the newConnection signal the QTcpServer emits. It
  will acquire the next connected QTcpSocket*, and then create a new client
  struct to add to the list.
  */
  void add_new_client();

  /*
  Forcibly disconnects a client given their id. Sends a closing message
  before removing them from the client list.
  */
  void force_disconnect_client(int id);

  /*
  Called when the client disconnects themself or when the server disconnects it.
  */
  void disconnect_client(int id);

  /*
  This is connected to the readyRead signal of every QTcpSocket. When some
  HSTP message is received from any client, it triggers, identifies the options,
  and then calls the relevant "handle" helpers to handle them.
  */
  void process_received_hstp_messages(int id);

  /*
  Serializes a audio/video payload and then distributes it to every
  client in the client list. Returns the number of clients it tried to send the
  frame too.
  */
  int send_frame(char const *source, QByteArray &audio, QVideoFrame video);

  /*
  Handles a client's initial establishment message and validates the client,
  enabling it to receive MFTP data.
  */
  void validate_client(int id, bool is_start, std::string alias, int mftp_port,
                       const QByteArray &password = nullptr);

  /*
  This slot will be connected to every client's processor and go off whenever a
  chat message is received. This will then forward it to every other client, and
  emit the chat_message_received signal.
  */
  void forward_chat_message(int sender_id, std::string alias,
                            std::string message);

  void forward_annotations(int sender_id, HSTP_Annotation annotation);

signals:
  /*
  This is emitted whenever a QTcpSocket emits stateChanged (connect signal to
  signal).
  */
  void client_disconnected(int id, std::string alias);

  /*
  This is emitted when a client sends its initial establishment start
  message, and we obtain its alias. Note it will have been added to
  the client list before this when it initially connects.
  */
  void client_connected(int id, std::string alias);

  /*
  This signal will be emitted anytime a chat message is received, from any
  client.
  */
  void chat_message_received(std::string alias, std::string message);

  /*
  This signal will be emitted anytime an annotation is received, from any
  client.
   */
  void annotation_received(std::string alias, HSTP_Annotation annotation);

private:
  /*
  Private method to help set up the signals/slots of the server.
  */
  void connect_signals_and_slots();

  // The host's alias
  std::string host_alias;

  // The host's hashed password
  QByteArray host_pass;

  // Global UDP socket that all clients will be contacted with
  std::shared_ptr<QUdpSocket> mftp_sock = nullptr;

  // Ports the server is running on
  int tcp_port = -1;
  int udp_port = -1;

  // Timestamp for server starting to calculate for RTP
  std::chrono::system_clock::time_point server_start;

  // Client list for this server
  std::unordered_map<int, Client> clients;
  int id_counter = 0;

  // Frame counter
  uint32_t frame_seq_num = 1;

  // Compression level - bounded [0 least, 1 most]
  float compression = 0;
};
