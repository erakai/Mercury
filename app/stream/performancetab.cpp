#include "performancetab.hpp"

#include "config/mconfig.hpp"

ServerPerformanceTab::ServerPerformanceTab(shared_ptr<MercuryServer> server,
                                           QWidget *parent)
    : QWidget(parent), server(server)
{
  // Layout UI
  QVBoxLayout *layout = new QVBoxLayout(this);

  fps_label = new QLabel(QString("Frames Per Second: %1").arg(FPS));
  fps_slider = new QSlider(Qt::Horizontal);
  fps_slider->setTickPosition(QSlider::TicksBothSides);
  fps_slider->setSingleStep(1);
  fps_slider->setPageStep(1);
  fps_slider->setMinimum(1);
  fps_slider->setMaximum(12);
  fps_slider->setValue(FPS);
  connect(fps_slider, &QAbstractSlider::sliderReleased, this,
          &ServerPerformanceTab::on_fps_slider_release);
  connect(fps_slider, &QAbstractSlider::valueChanged, this,
          &ServerPerformanceTab::on_fps_slider_update);

  request_frequency_label =
      new QLabel(QString("Performance Update Frequency: %1s")
                     .arg(seconds_in_between_requests));
  request_frequency_slider = new QSlider(Qt::Horizontal);
  request_frequency_slider->setTickPosition(QSlider::TicksBothSides);
  request_frequency_slider->setSingleStep(1);
  request_frequency_slider->setPageStep(1);
  request_frequency_slider->setMinimum(1);
  request_frequency_slider->setMaximum(15);
  request_frequency_slider->setValue(seconds_in_between_requests);
  connect(request_frequency_slider, &QAbstractSlider::valueChanged, this,
          &ServerPerformanceTab::on_request_slider_update);

  layout->addWidget(fps_label);
  layout->addWidget(fps_slider);
  layout->addWidget(request_frequency_label);
  layout->addWidget(request_frequency_slider);

  QLabel *client_label = new QLabel("Clients:");
  layout->addWidget(client_label);

  alias_list = new QListWidget;
  QPalette list_palette = alias_list->palette();
  list_palette.setColor(QPalette::Base, Qt::white);
  list_palette.setColor(QPalette::Text, Qt::black);
  alias_list->setPalette(list_palette);

  performance_tabs = new QStackedWidget;
  performance_tabs->addWidget(new QWidget);

  layout->addWidget(alias_list);
  layout->setStretch(5, 12);
  layout->addWidget(performance_tabs);
  layout->setStretch(6, 68);

  connect(alias_list, &QListWidget::currentRowChanged, this,
          [=, this](int current_row)
          { performance_tabs->setCurrentIndex(current_row + 1); });

  // Set up timer
  update_timer = new QTimer(this);
  connect(update_timer, &QTimer::timeout, this,
          &ServerPerformanceTab::request_metrics);
  update_request_frequency(seconds_in_between_requests);

  // Connect to server client join/leave
  connect(server.get(), &MercuryServer::client_connected, this,
          &ServerPerformanceTab::on_client_connect);
  connect(server.get(), &MercuryServer::client_disconnected, this,
          &ServerPerformanceTab::on_client_disconnect);
};

void ServerPerformanceTab::update_request_frequency(int seconds_in_between)
{
  seconds_in_between_requests = seconds_in_between;
  update_timer->start(seconds_in_between_requests * 1000);

  for (int i = 1; i < performance_tabs->count(); i++)
  {
    ClientPerformanceTab *tab =
        dynamic_cast<ClientPerformanceTab *>(performance_tabs->widget(i));

    if (tab != nullptr)
      tab->reset_charts();
  }
}

void ServerPerformanceTab::request_metrics()
{
  HstpHandler temp;
  temp.init_msg(server->get_alias().c_str());
  temp.add_option_performance_request(QDateTime::currentMSecsSinceEpoch());
  server->send_hstp_message_to_all_clients(*(temp.output_msg().get()));
}

void ServerPerformanceTab::on_client_connect(int id, string alias)
{
  alias_list->addItem(QString::fromStdString(alias));

  ClientPerformanceTab *tab = new ClientPerformanceTab(nullptr, alias);

  Client &client = server->get_client(id);
  connect(client.processor.get(), &HstpProcessor::received_performance_metrics,
          tab, &ClientPerformanceTab::update_charts);

  performance_tabs->addWidget(tab);
}

void ServerPerformanceTab::on_client_disconnect(int id, string alias)
{
  // We have to add 1 to the client size because we already removed it in the
  // server
  for (int i = 0; i < server->get_clients().size() + 1; i++)
  {
    QListWidgetItem *item = alias_list->item(i);
    if (item && item->text() == QString::fromStdString(alias))
    {
      // Dummy widget in first index of performance tab
      i += 1;
      if (performance_tabs->currentIndex() == i)
        performance_tabs->setCurrentIndex(0);
      QWidget *tab = performance_tabs->widget(i);
      performance_tabs->removeWidget(tab);
      delete tab;

      alias_list->takeItem(i - 1);
      delete item;
      return;
    }
  }

  qWarning("Client \"%s\" disconnected but unable to identify widgets.",
           alias.c_str());
}

void ServerPerformanceTab::on_fps_slider_update(int value)
{
  fps_label->setText(QString("Frames Per Second: %1").arg(value));
}

void ServerPerformanceTab::on_fps_slider_release()
{
  int value = fps_slider->value();
  FPS = value;

  // Tell all clients to now watch at this fps
  // We do this on release, not update, to not spam everyone
  HstpHandler temp;
  temp.init_msg(server->get_alias().c_str());
  temp.add_option_fps(value);
  server->send_hstp_message_to_all_clients(*(temp.output_msg().get()));
}

void ServerPerformanceTab::on_request_slider_update(int value)
{
  update_request_frequency(value);
  request_frequency_label->setText(
      QString("Performance Update Frequency: %1s").arg(value));
}

ClientPerformanceTab::ClientPerformanceTab(shared_ptr<MercuryClient> client,
                                           string alias, QWidget *parent)
    : QWidget(parent), client(client), alias(alias)
{
  if (client)
  {
    connect(client->hstp_processor().get(),
            &HstpProcessor::received_performance_request, this,
            &ClientPerformanceTab::reply_with_performance_metrics);
  }

  // Set up UI
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setAlignment(Qt::AlignTop);

  if (!client)
  {
    QLabel *titleLabel =
        new QLabel(QString("Network Performance: %1").arg(alias.c_str()));
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setPointSize(20);
    font.setBold(true);
    titleLabel->setFont(font);
    layout->addWidget(titleLabel);
    layout->addWidget(new QWidget);
  }

  jitter_label = new QLabel;
  latency_label = new QLabel;
  loss_label = new QLabel;
  fps_label = new QLabel;
  throughput_label = new QLabel;

  layout->addWidget(fps_label);
  layout->addWidget(latency_label);
  layout->addWidget(jitter_label);
  layout->addWidget(loss_label);
  layout->addWidget(throughput_label);
  layout->addWidget(new QWidget);

  history = new QTableWidget(MAX_HISTORY_LEN, 6);
  history->setHorizontalHeaderLabels(
      {"t", "FPS", "Latency", "Jitter", "Loss", "Throughput"});

  history->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  history->verticalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  history->verticalHeader()->setVisible(false);

  history->setSelectionMode(QAbstractItemView::NoSelection);
  history->setEditTriggers(QAbstractItemView::NoEditTriggers);
  history->setFocusPolicy(Qt::NoFocus);
  history->setSizePolicy(QSizePolicy::MinimumExpanding,
                         QSizePolicy::MinimumExpanding);

  layout->addWidget(history);

  reset_charts();
};

double ClientPerformanceTab::calculate_jitter()
{
  // Compute the mean latency
  double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
  double mean = sum / latencies.size();

  // Compute variance
  double varianceSum = 0.0;
  for (uint16_t latency : latencies)
  {
    double diff = latency - mean;
    varianceSum += (diff * diff);
  }
  double variance = varianceSum / latencies.size();

  // Standard deviation as jitter
  return std::sqrt(variance);
}

void ClientPerformanceTab::update_charts(const char alias[ALIAS_SIZE],
                                         uint16_t latency, uint32_t throughput,
                                         float loss, float fps)
{
  // Retrieve updated jitter
  latencies.push_back(latency);
  if (latencies.size() > MAX_DATA_POINTS)
  {
    latencies.erase(latencies.begin());
  }
  double jitter = calculate_jitter();

  uint64_t curr_secs = QDateTime::currentSecsSinceEpoch();
  uint64_t time = curr_secs - last_reset_seconds;

  update_jitter_label(jitter);
  update_latency_label(latency);
  update_fps_label(fps);
  update_throughput_label(throughput);
  update_loss_label(loss);

  // "Time (s)", "FPS", "Latency", "Jitter", "Loss", "Throughput"
  history->insertRow(0);

  QTableWidgetItem *i1 = new QTableWidgetItem(tr("%1s").arg(time));
  QTableWidgetItem *i2 = new QTableWidgetItem(QString::asprintf("%.2f", fps));
  QTableWidgetItem *i3 = new QTableWidgetItem(tr("%1").arg(latency));
  QTableWidgetItem *i4 =
      new QTableWidgetItem(QString::asprintf("%.2f", jitter));
  QTableWidgetItem *i5 = new QTableWidgetItem(QString::asprintf("%.2f", loss));
  QTableWidgetItem *i6 =
      new QTableWidgetItem(QString::asprintf("%.3f", throughput / 1000000.0));

  history->setItem(0, 0, i1);
  history->setItem(0, 1, i2);
  history->setItem(0, 2, i3);
  history->setItem(0, 3, i4);
  history->setItem(0, 4, i5);
  history->setItem(0, 5, i6);

  history->removeRow(history->rowCount() - 1);
}

void ClientPerformanceTab::reset_charts()
{
  latencies.clear();

  update_jitter_label(0);
  update_latency_label(0);
  update_fps_label(0);
  update_throughput_label(0);
  update_loss_label(0);

  last_reset_seconds = QDateTime::currentSecsSinceEpoch();
  history->clearContents();
}

void ClientPerformanceTab::reply_with_performance_metrics(
    const char *_alias, uint64_t request_sent_time)
{
  Metrics &metrics = client->metrics();
  HstpHandler &handler = client->hstp_handler();
  uint64_t current_time = QDateTime::currentMSecsSinceEpoch();

  handler.init_msg(client->get_alias().c_str());
  uint16_t latency = static_cast<uint16_t>(current_time - request_sent_time);
  uint32_t throughput = metrics.retrieve_throughput();
  float loss = metrics.retrieve_loss();
  float fps = metrics.retrieve_fps();
  handler.add_option_performance_metrics(latency, throughput, loss, fps);

  handler.output_msg_to_socket(client->hstp_sock());
  metrics.reset();

  update_charts(alias.c_str(), latency, throughput, loss, fps);
}

void ClientPerformanceTab::update_jitter_label(double jitter)
{
  jitter_label->setText(QString::asprintf("Jitter (stddev): %.2fms", jitter));
}

void ClientPerformanceTab::update_latency_label(uint16_t latency)
{
  latency_label->setText(QString("TCP Latency: %1ms").arg(latency));
}

void ClientPerformanceTab::update_fps_label(float fps)
{
  fps_label->setText(QString::asprintf("True Frames Per Second: %.2f", fps));
}

void ClientPerformanceTab::update_throughput_label(uint32_t throughput)
{
  throughput_label->setText(QString::asprintf("Frame Throughput: %.3f mbps",
                                              (throughput / 1000000.0)));
}

void ClientPerformanceTab::update_loss_label(float loss)
{
  loss_label->setText(QString::asprintf("UDP Datagram Loss: %.2f%%", loss));
}
