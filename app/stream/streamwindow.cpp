#include "streamwindow.hpp"
#include "hstp.hpp"
#include "singleton/videomanager.h"
#include <QApplication>
#include <QMenuBar>
#include <QScreen>
#include <QtDebug>
#include <QAudioSink>

StreamWindow::StreamWindow(std::string alias, shared_ptr<HostService> host_data,
                           QWidget *parent)
    : QMainWindow(parent), mode(MercuryMode::HOST), alias(alias),
      servh(host_data)
{
  set_up();
}

StreamWindow::StreamWindow(std::string alias,
                           shared_ptr<ClientService> client_data,
                           QWidget *parent)
    : QMainWindow(parent), mode(MercuryMode::CLIENT), alias(alias),
      servc(client_data)
{
  set_up();
}

void StreamWindow::set_up()
{
  setWindowTitle("Mercury");
  setAttribute(Qt::WA_DeleteOnClose);

  this->showMaximized(); // Sets Window size to max

  initialize_primary_ui_widgets();
  configure_menu_and_tool_bar();

  display->setLayout(main_layout);
  setCentralWidget(display);

  main_layout->addWidget(stream_display, 0, 0);
  main_layout->addWidget(side_pane, 0, 1, 2, 1);
  main_layout->addLayout(below_stream_layout, 1, 0);

  below_stream_layout->addWidget(stream_title, 0, 0);
  below_stream_layout->addWidget(host_name, 1, 0);
  below_stream_layout->addWidget(viewer_count, 0, 2);

  /*
  |---------------------------|-------------|
  |75% - Stream Window Across | 25% - Chat  |
  |                           |             |
  |                           |             |
  |                           |             |
  |                           |             |
  |-80% display---------------|             |
  |                           |             |
  |                           |             |
  |-20% info------------------|-------------|
  */

  main_layout->setRowStretch(0, 80);
  main_layout->setRowStretch(1, 20);

  main_layout->setColumnStretch(0, 75);
  main_layout->setColumnStretch(1, 25);

  // Center this window
  move(QGuiApplication::screens().at(0)->geometry().center() -
       frameGeometry().center());

  connect_signals_and_slots();
}

void StreamWindow::configure_menu_and_tool_bar()
{
  stream_menu = menuBar()->addMenu(tr("&Stream"));

  std::string label = "&End Stream";
  if (is_client())
    label = "&Leave Stream";
  stop_or_leave_stream_action = new QAction(tr(label.c_str()), this);
  stream_menu->addAction(stop_or_leave_stream_action);
}

void StreamWindow::connect_signals_and_slots()
{
  connect(stop_or_leave_stream_action, &QAction::triggered, this,
          &StreamWindow::shut_down_window);

  // connect socket disconnected to this window closing
  if (is_client())
    connect(servc->client.get(), &MercuryClient::client_disconnected, this,
            &StreamWindow::shut_down_window);

  // connect chat message received to chat
  if (is_host())
    connect(servh->server.get(), &MercuryServer::chat_message_received, this,
            &StreamWindow::new_chat_message);
  if (is_client())
    connect(servc->client->hstp_processor().get(),
            &HstpProcessor::received_chat, this,
            [=, this](const char alias[ALIAS_SIZE],
                      const char alias_of_chatter[ALIAS_SIZE],
                      const std::string &chat)
            { this->new_chat_message(std::string(alias_of_chatter), chat); });

  // connect chat message sent on side pane
  connect(side_pane->get_chat_tab(), &ChatTab::send_chat_message, this,
          &StreamWindow::send_chat_message);

  // connect viewer count updated for host
  if (is_host())
  {
    connect(servh->server.get(), &MercuryServer::client_connected, this,
            &StreamWindow::viewer_connected);
    connect(servh->server.get(), &MercuryServer::client_disconnected, this,
            &StreamWindow::viewer_disconnected);
  }

  // connect viewer count updated for client
  if (is_client())
    connect(servc->client->hstp_processor().get(),
            &HstpProcessor::received_viewer_count, this,
            [=, this](const char alias[ALIAS_SIZE], uint32_t viewers)
            { this->viewer_count_updated(viewers); });

  // connect stream title for client
  if (is_client())
    connect(servc->client->hstp_processor().get(),
            &HstpProcessor::received_stream_title, this,
            [=, this](const char alias[ALIAS_SIZE], const char *stream_title)
            {
              this->stream_name_changed(std::string(alias),
                                        std::string(stream_title));
            });

  // connect new fps for client
  if (is_client())
    connect(servc->client->hstp_processor().get(), &HstpProcessor::received_fps,
            this, [=, this](const char alias[ALIAS_SIZE], uint32_t new_fps)
            { stream_display->set_new_fps(new_fps); });

  // connect stream fully initialized to client jitter buffer full signal
  if (is_host())
    stream_fully_initialized();
  if (is_client())
    connect(servc->client.get(),
            &MercuryClient::jitter_buffer_sufficiently_full, this,
            &StreamWindow::stream_fully_initialized);
}

void StreamWindow::initialize_primary_ui_widgets()
{
  main_layout = new QGridLayout();

  display = new QWidget(this);

  side_pane = new SidePane(this);
  side_pane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  side_pane->initialize_chat_tab(alias);

  if (is_host())
  {
    side_pane->initialize_viewer_list_tab(alias);
    side_pane->initialize_server_performance_tab(servh->server);
  }

  if (is_client())
    side_pane->initialize_client_performance_tab(servc->client);

  std::function<bool(QImage &)> video_func = std::bind(
      &StreamWindow::provide_next_video_frame, this, std::placeholders::_1);
  std::function<bool(QBuffer &)> audio_func = std::bind(
      &StreamWindow::provide_next_audio_frame, this, std::placeholders::_1);
  stream_display = new StreamDisplay(this, video_func, audio_func);
  stream_display->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  below_stream_layout = new QGridLayout();

  stream_title = new QLabel("Stream Title", this);
  if (is_host() && servh->stream_name.size() > 0)
    stream_title->setText(servh->stream_name.c_str());
  if (is_host())
    viewer_count = new QLabel(
        std::format("Viewers: {}", servh->viewer_count).c_str(), this);
  else
    viewer_count = new QLabel("Viewers: 1", this);

  if (is_host())
    host_name = new QLabel(std::format("Host: {}", alias).c_str(), this);
  if (is_client())
    host_name = new QLabel(std::format("Host: {}", "Host").c_str(), this);
}

void StreamWindow::stream_fully_initialized()
{
  qInfo("Beginning stream playback.");
  stream_display->begin_playback();
}

void StreamWindow::closeEvent(QCloseEvent *event)
{
  // Client already calls this from the disconnected signal on the socket
  if (is_host())
    shut_down_window();

  QWidget::closeEvent(event);
}

void StreamWindow::shut_down_window()
{
  if (is_host())
    servh->server->close_server();

  if (is_client())
    servc->client->disconnect();

  close();
}

bool StreamWindow::provide_next_video_frame(QImage &next_video)
{
  if (is_host())
  {
    // acquire video frame from desktop
    QImage img;
    VideoManager::VideoImageStatus status =
        VideoManager::instance().GetVideoImage(img);
    if (status == VideoManager::VideoImageStatus::SUCCESS)
    {
      servh->server->send_frame("desktop", QAudioBuffer(), QVideoFrame(img));
      next_video = img;
      return true;
    }
    else
    {
      qWarning("Unable to retrieve video image from manager (status: %d).",
               status);
      return false;
    }

    return true;
  }
  else
  {
    // acquire video frame from jitter buffer
    JitterEntry jitter = servc->client->retrieve_next_frame();

    if (jitter.seq_num == -1)
    {
      qInfo("Jitter buffer empty when frame desired.");
      return false;
    }

    servc->client->metrics().register_frame();
    next_video = jitter.video;
    return true;
  }

  return false;
}

bool StreamWindow::provide_next_audio_frame(QBuffer &next_audio)
{
  if (is_host())
  {
    // acquire audio frame from desktop
  }
  else
  {
    // acquire audio frame from jitter buffer
  }

  // TESTING!:
  /*
  QFile fart_file("assets/fart.mp3");
  if (!fart_file.open(QIODevice::ReadOnly))
  {
    // Handle error opening file
    qInfo("failed to load audio file");
    return false;
  }

  QByteArray audio_byte_array = fart_file.readAll();
  fart_file.close();
  next_audio.open(QIODevice::WriteOnly);
  next_audio.write(audio_byte_array);
  next_audio.close();
  return true;
  */

  return false;
}

void StreamWindow::send_chat_message(string message)
{
  if (is_host())
    servh->server->forward_chat_message(-1, alias, message);
  if (is_client())
    servc->client->send_chat_message(message);
}

void StreamWindow::viewer_count_updated(int new_count)
{
  viewer_count->setText(("Viewers: " + std::to_string(new_count)).c_str());

  // This slot is called whenever a viewer connects/disconnects for the host,
  // who then must propagate it to the clients
  if (is_host() && servh->viewer_count > 0)
  {
    HstpHandler temp_handler;
    temp_handler.init_msg(alias.c_str());
    temp_handler.add_option_viewer_count(new_count);
    std::shared_ptr<QByteArray> bytes = temp_handler.output_msg();
    servh->server->send_hstp_message_to_all_clients(*bytes);
  }
}

void StreamWindow::stream_name_changed(string host_alias, string new_name)
{
  stream_title->setText(new_name.c_str());

  if (is_host() && servh->viewer_count > 0)
  {
    HstpHandler temp_handler;
    temp_handler.init_msg(alias.c_str());
    temp_handler.add_option_stream_title(new_name.c_str());
    std::shared_ptr<QByteArray> bytes = temp_handler.output_msg();
    servh->server->send_hstp_message_to_all_clients(*bytes);
  }

  if (is_client())
  {
    host_name->setText(std::format("Host: {}", host_alias).c_str());
  }
}

void StreamWindow::new_chat_message(string alias, string msg)
{
  side_pane->get_chat_tab()->new_chat_message({alias, msg});
}

void StreamWindow::viewer_connected(int id, std::string _alias)
{
  servh->viewer_count++;
  viewer_count_updated(servh->viewer_count);

  side_pane->get_viewer_list_tab()->viewer_joined(_alias);

  Client &client = servh->server->get_client(id);
  client.handler.init_msg(alias.c_str());
  client.handler.add_option_stream_title(
      stream_title->text().toStdString().c_str());
  client.handler.add_option_viewer_count(servh->viewer_count);
  // TODO: client.handler.add_option_fps(FPS);
  client.handler.output_msg_to_socket(client.hstp_sock);
}

void StreamWindow::viewer_disconnected(int id, std::string _alias)
{
  servh->viewer_count--;
  viewer_count_updated(servh->viewer_count);

  side_pane->get_viewer_list_tab()->viewer_left(_alias);
}
