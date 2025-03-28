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

  // should be able to adjust compression
  // as well as max fps with sliders
  // also be able to change frequency of performance requests

  // should also calculate jitter for each client
  // calculate delay-throughput product?
  // use Qt Charts
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

  // just need to display latency, fps, throughput
  // maybe as charts if care enough

  // update charts with new thing everytime respond to performance request
};
