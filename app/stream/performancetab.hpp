#pragma once

#include "mercury_client.hpp"
#include "mercury_server.hpp"

#include <QValueAxis>
#include <QVBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QWidget>
#include <QChartView>
#include <QSlider>
#include <QChart>
#include <QListWidget>
#include <QLineSeries>
#include <QStackedWidget>
#include <QLabel>

#include <vector>
#include <memory>

using namespace std;

#define MAX_DATA_POINTS 10

class ServerPerformanceTab : public QWidget
{
  Q_OBJECT;

public:
  ServerPerformanceTab(shared_ptr<MercuryServer> server,
                       QWidget *parent = nullptr);

  void update_request_frequency(int seconds_in_between);

public slots:
  void request_metrics();

  void on_client_connect(int id, string alias);
  void on_client_disconnect(int id, string alias);

  void on_fps_slider_update(int value);
  void on_fps_slider_release();
  void on_request_slider_update(int value);

private:
  shared_ptr<MercuryServer> server;

  // How often to send out a request - connected to timer
  int seconds_in_between_requests = 5;
  QTimer *update_timer;

  // Maintain a client performance tab for every client
  QListWidget *alias_list;
  QStackedWidget *performance_tabs;

  // Adjust compression / max fps / seconds_in_between with sliders
  QLabel *fps_label;
  QSlider *fps_slider;
  QLabel *request_frequency_label;
  QSlider *request_frequency_slider;
};

class ClientPerformanceTab : public QWidget
{
  Q_OBJECT;

public:
  ClientPerformanceTab(shared_ptr<MercuryClient> client, string alias,
                       QWidget *parent = nullptr);

  // Either called manually by ServerPerformanceTab, or called every
  // time we reply to a performance metrics if this is a client
  void update_charts(const char alias[ALIAS_SIZE], uint16_t latency,
                     uint32_t throughput, float loss, float fps);

  double calculate_jitter();
  void reset_charts();
  string get_alias() { return alias; }

public slots:
  // Only used if client* is non-null
  void reply_with_performance_metrics(const char *alias,
                                      uint64_t request_received_time);

private:
  void update_jitter_label(double jitter);
  void update_latency_label(uint16_t latency);
  void update_fps_label(float fps);
  void update_throughput_label(uint32_t throughput);
  void update_loss_label(float loss);

  shared_ptr<MercuryClient> client;
  string alias;

  // For calculating jitter
  vector<uint16_t> latencies;

  // Actual data display
  uint64_t last_reset_seconds = -1;
  QLabel *jitter_label;
  QLabel *latency_label;
  QLabel *loss_label;
  QLabel *fps_label;
  QLabel *throughput_label;

  // Charts
  // https://doc.qt.io/qt-6/qtcharts-chartsgallery-example.html
  QChartView *latency_chart_view;
  QChart *latency_chart;
  QLineSeries *latency_series;
};
