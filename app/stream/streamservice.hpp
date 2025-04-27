#pragma once

#include "mercury_server.hpp"
#include "mercury_client.hpp"
#include <QHostAddress>
#include <string>
#include <memory>

using namespace std;

class HostService
{
public:
  HostService(std::string alias, int tcp_port, int udp_port,
              const QByteArray &pass = nullptr)
      : server(make_shared<MercuryServer>(alias, tcp_port, udp_port, pass)),
        tcp_port(tcp_port)
  {
  }

  shared_ptr<MercuryServer> server;

  int viewer_count = 0;
  std::string stream_name;
  std::string ip_address;
  int tcp_port = 0;
  bool public_stream = false;
  uint32_t start_timestamp = 0;
  bool reactions_enabled = false;
};

class ClientService
{
public:
  ClientService(std::string alias) : client(make_shared<MercuryClient>(alias))
  {
  }

  shared_ptr<MercuryClient> client;
};
