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
  HostService(std::string alias, int tcp_port, int udp_port)
      : server(make_shared<MercuryServer>(alias, tcp_port, udp_port))
  {
  }

  shared_ptr<MercuryServer> server;

  int viewer_count = 0;
  std::string stream_name;
};

class ClientService
{
public:
  ClientService(std::string alias) : client(make_shared<MercuryClient>(alias))
  {
  }

  shared_ptr<MercuryClient> client;
};
