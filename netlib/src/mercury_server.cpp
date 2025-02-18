#include "mercury_server.hpp"
#include "logger.hpp"
#include "mftp.hpp"

#include <QNetworkInterface>
#include <QTcpSocket>
#include <Qbuffer>

void MercuryServer::connect_signals_and_slots()
{
  connect(this, &QTcpServer::newConnection, this,
          &MercuryServer::add_new_client);
}

void MercuryServer::start_server()
{
  mftp_sock = acquire_mftp_socket(udp_port);

  if (!listen(QHostAddress::Any, tcp_port))
  {
    log("Error: Unable to start server", ll::CRITICAL);
    close();
    return;
  }

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

  log("Started server at %s on port %d.", address.data(), tcp_port, ll::NOTE);
  server_start = std::chrono::system_clock::now();
  id_counter = 0;
}

void MercuryServer::close_server()
{
  auto it = clients.begin();
  while (it != clients.end())
  {
    int id = it->first;
    force_disconnect_client(id);
  }

  clients.clear();
  close();
  mftp_sock->close();

  log("Server closed.", ll::NOTE);
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

void MercuryServer::add_new_client()
{
  // Add the new client
  if (!hasPendingConnections())
    return;
  Client new_client;

  QTcpSocket *connection = nextPendingConnection();
  new_client.id = ++id_counter;
  new_client.hstp_sock = std::shared_ptr<QTcpSocket>(connection);
  new_client.hstp_messager = HstpHandler();

  // Connect relevant functions
  connect(connection, &QAbstractSocket::readyRead, this,
          [=, this]() { this->process_received_hstp_messages(new_client.id); });
  connect(connection, &QAbstractSocket::disconnected, this,
          [=, this]() { this->disconnect_client(new_client.id); });

  clients[new_client.id] = new_client;
}

void MercuryServer::force_disconnect_client(int id)
{
  // Waiting on HSTP to be implemnted.
  Client client = clients[id];
  client.hstp_messager.init_msg(client.alias);
  // client.hstp_messager.add_option_establishing(0);
  // client.hstp_messager.send_msg();

  disconnect_client(id);
}

void MercuryServer::disconnect_client(int id)
{
  Client client = clients[id];

  if (client.hstp_sock->state() == QAbstractSocket::ConnectedState)
    client.hstp_sock->disconnectFromHost();
  clients.erase(id);

  emit client_disconnect(id, client.alias);

  //  free(client.hstp_sock);
  log("Client %s has disconnected.", client.alias, ll::NOTE);
}

void MercuryServer::process_received_hstp_messages(int id)
{
  // Remember to emit client connect if this is an establishment message,
  // and call the appropriate handlers otherwise.
  // Waiting on HSTP to be implemented.
}

void MercuryServer::send_frame(char source[12], QAudioBuffer audio,
                               QPixmap video)
{
  // Send to every client and increment their frame seq num
  MFTP_Header header;
  AV_Frame payload;

  // Serialize data to payload
  payload.audio.append(audio.constData<char>(), audio.byteCount());
  QBuffer buffer(&payload.video);
  buffer.open(QIODevice::WriteOnly);
  video.save(&buffer, "PNG");

  // Set up header fields
  header.version = MFTP_VERSION;
  header.payload_type = 0;

  auto current_time = std::chrono::system_clock::now();
  std::chrono::duration<double, std::milli> delta = current_time - server_start;
  auto delta_ms_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(delta);

  header.timestamp = delta_ms_duration.count();
  header.audio_len = payload.audio.size();
  header.video_len = payload.video.size();
  strcpy(header.source_name, source);

  // Send data
  for (auto &[id, client] : clients)
  {
    if (client.mftp_port == 0 || id == -1)
      continue;

    header.seq_num = client.frame_seq_num;
    client.frame_seq_num++;

    send_datagram(mftp_sock, client.hstp_sock->peerAddress(), client.mftp_port,
                  header, payload);
  }
}
