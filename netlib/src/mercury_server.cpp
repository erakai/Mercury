#include "mercury_server.hpp"
#include "hstp.hpp"
#include "mftp.hpp"

#include <QNetworkInterface>
#include <QTcpSocket>
#include <Qbuffer>
#include <QtCore/qdebug.h>
#include <QtCore/qlogging.h>
#include <vector>

void MercuryServer::connect_signals_and_slots()
{
  connect(this, &QTcpServer::newConnection, this,
          &MercuryServer::add_new_client);
}

bool MercuryServer::start_server()
{
  if (udp_port == -1 || tcp_port == -1)
  {
    qCritical("Server ports are not set, cannot start.");
    return false;
  }

  if (!listen(QHostAddress::Any, tcp_port))
  {
    qCritical("Error: Unable to start server.");
    close();
    return false;
  }

  mftp_sock = acquire_mftp_socket(udp_port);
  if (mftp_sock == nullptr)
  {
    qCritical("Unable to construct MFTP socket.");
    return false;
  }

  connect_signals_and_slots();

  QString address;
  for (const QHostAddress &entry : QNetworkInterface::allAddresses())
  {
    if (entry != QHostAddress::LocalHost && entry.toIPv4Address())
    {
      address = entry.toString();
      break;
    }
  }

  if (address.isEmpty())
    address = QHostAddress(QHostAddress::LocalHost).toString();

  qInfo("Started server at %s with HSTP port %d.",
        address.toStdString().c_str(), tcp_port);
  server_start = std::chrono::system_clock::now();
  id_counter = 0;

  return true;
}

void MercuryServer::close_server()
{
  auto it = clients.begin();
  while (it != clients.end())
  {
    int id = it->first;
    force_disconnect_client(id);
    it = clients.begin();
  }

  clients.clear();

  if (isListening() || mftp_sock->isOpen())
  {
    mftp_sock->close();
    close();

    qInfo("Gracefully shutting down...");
    qInfo("Server closed.");
  }

  id_counter = 0;
}

int MercuryServer::convert_alias_to_id(char *alias)
{
  for (const auto &[id, client] : clients)
  {
    if (strcmp(client.alias, alias) == 0)
      return id;
  }
  return -1;
}

std::unordered_map<int, Client> MercuryServer::get_clients()
{
  return clients;
}

void MercuryServer::set_ports(int tcp, int udp)
{
  // Don't change ports if server is started
  if (isListening())
    return;

  tcp_port = tcp;
  udp_port = udp;
}

void MercuryServer::add_new_client()
{
  // Add the new client
  if (!hasPendingConnections())
    return;
  Client new_client;

  // TODO: VERIFY CLIENT NOT ON BLACKLIST BEFORE ADDING TO LIST
  QTcpSocket *connection = nextPendingConnection();
  new_client.id = ++id_counter;
  new_client.hstp_sock = std::shared_ptr<QTcpSocket>(connection);
  new_client.handler = HstpHandler();
  new_client.processor = std::make_shared<HstpProcessor>();

  // NOT VALIDATED until first establishing HSTP message received.
  // Please read validate_client().
  new_client.validated = false;

  // Connect relevant functions
  connect(connection, &QAbstractSocket::readyRead, this,
          [=, this]() { this->process_received_hstp_messages(new_client.id); });
  connect(new_client.hstp_sock.get(), &QAbstractSocket::disconnected, this,
          [=, this]() { this->disconnect_client(new_client.id); });

  // Allow for validation if we receive an establishment message
  connect(new_client.processor.get(), &HstpProcessor::received_establishment,
          this,
          [=, this](const char alias[ALIAS_SIZE], bool is_start,
                    uint16_t mftp_port, const QByteArray &pass)
          {
            this->validate_client(new_client.id, is_start, std::string(alias),
                                  mftp_port, pass);
          });

  clients[new_client.id] = new_client;
}

void MercuryServer::validate_client(int id, bool is_start, std::string alias,
                                    int mftp_port, const QByteArray &password)
{
  Client &new_client = clients[id];

  // TODO: VERIFY PASSWORD HERE (ADD EXTRA PARAMETER)
  if (!host_pass.isNull() && !host_pass.isEmpty())
  {
    if (host_pass != password)
    {
      force_disconnect_client(id);
      return;
    }
  }

  new_client.validated = true;
  new_client.mftp_port = mftp_port;
  strcpy(new_client.alias, alias.c_str());
  qInfo("Client \"%s\" has connected.", new_client.alias);

  // Connect relevant signals to slots
  connect(new_client.processor.get(), &HstpProcessor::received_chat, this,
          [=, this](const char alias[ALIAS_SIZE],
                    const char alias_of_chatter[ALIAS_SIZE],
                    const std::string &chat)
          {
            this->forward_chat_message(new_client.id,
                                       std::string(alias_of_chatter), chat);
            emit chat_message_received(std::string(alias), chat);
          });

  emit client_connected(id, std::string(alias));
}

void MercuryServer::force_disconnect_client(int id)
{
  // Inform the client that they have been disconnected
  Client &client = clients[id];
  client.handler.init_msg(host_alias.c_str());
  client.handler.add_option_establishment(false, 0);
  client.handler.output_msg_to_socket(client.hstp_sock);

  disconnect_client(id);
}

void MercuryServer::disconnect_client(int id)
{
  // TODO: Hi Kai, we added this cool if statement cause at
  // force_disconnect_client() it sends an establishment message that will
  // call this again even though force_disconnect_client() calls it.
  // ask Alex if confused
  if (clients.count(id) == 0)
  {
    return;
  }
  Client client = clients[id];

  if (client.hstp_sock->state() == QAbstractSocket::ConnectedState)
    client.hstp_sock->disconnectFromHost();
  clients.erase(id);

  // If this was a "real" client
  if (client.validated)
  {
    emit client_disconnected(id, std::string(client.alias));
    qInfo("Client \"%s\" has disconnected.", client.alias);
  }
}

void MercuryServer::process_received_hstp_messages(int id)
{
  Client &client = clients[id];

  if (client.hstp_sock->state() == QAbstractSocket::ConnectedState)
  {
    QByteArray data = client.hstp_sock->readAll();
    client.processor->process(data);
    return;
  }

  qCritical("Issue processing received hstp message.");
}

void MercuryServer::forward_chat_message(int sender_id, std::string alias,
                                         std::string message)
{
  for (auto &[id, client] : clients)
  {
    if (id != sender_id)
    {
      client.handler.init_msg(host_alias.c_str());
      client.handler.add_option_chat(alias.c_str(), message.c_str());
      client.hstp_sock->write(*(client.handler.output_msg()));
    }
  }
}

int MercuryServer::send_frame(const char *source, QAudioBuffer audio,
                              QVideoFrame video)
{
  if (strlen(source) > 12)
  {
    qCritical("Source passed to send_frame too long - should be <= 12, instead "
              "is %ld",
              static_cast<long>(strlen(source)));
    return -1;
  }

  // Send to every client and increment their frame seq num
  MFTP_Header header;

  // Set up header fields
  header.version = MFTP_VERSION;
  header.payload_type = 0;
  header.seq_num = frame_seq_num;
  frame_seq_num++;

  auto current_time = std::chrono::system_clock::now();
  std::chrono::duration<double, std::milli> delta = current_time - server_start;
  auto delta_ms_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(delta);

  header.timestamp = delta_ms_duration.count();
  strcpy(header.source_name, source);

  // Send data
  int client_sent_to_count = 0;

  std::vector<QHostAddress> dest_addr;
  std::vector<int> dest_ports;

  for (auto &[id, client] : clients)
  {
    if (!client.validated || client.mftp_port == 0 || id == -1)
      continue;

    client_sent_to_count++;

    dest_addr.push_back(client.hstp_sock->peerAddress());
    dest_ports.push_back(client.mftp_port);
  }

  send_datagram(mftp_sock, dest_addr, dest_ports, header, video.toImage(),
                audio);

  return client_sent_to_count;
}

void MercuryServer::send_hstp_message_to_all_clients(QByteArray msg)
{
  for (auto &[id, client] : clients)
  {
    client.hstp_sock->write(msg);
  }
}
