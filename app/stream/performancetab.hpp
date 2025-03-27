#pragma once

#include "mercury_client.hpp"
#include "mercury_server.hpp"

#include <QVBoxLayout>
#include <QWidget>

#include <memory>

using namespace std;

class ServerPerformanceTab : public QWidget
{
  Q_OBJECT;

public:
  ServerPerformanceTab(shared_ptr<MercuryServer> server,
                       QWidget *parent = nullptr)
      : QWidget(parent), server(server) {};

private:
  // https://doc.qt.io/qt-6/qtcharts-chartsgallery-example.html
  shared_ptr<MercuryServer> server;
};

class ClientPerformanceTab : public QWidget
{
  Q_OBJECT;

public:
  ClientPerformanceTab(shared_ptr<MercuryClient> server,
                       QWidget *parent = nullptr)
      : QWidget(parent), server(server) {};

private:
  // https://doc.qt.io/qt-6/qtcharts-chartsgallery-example.html
  shared_ptr<MercuryClient> server;
};
