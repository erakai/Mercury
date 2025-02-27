#include "streamwindow.hpp"
#include "hosttoolbar.hpp"
#include "logger.hpp"
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

  initialize_primary_ui_widgets();
  configure_menu_and_tool_bar();

  display->setLayout(main_layout);
  setCentralWidget(display);

  main_layout->addWidget(stream_display, 0, 0);
  main_layout->addWidget(side_pane, 0, 1, 2, 1);
  main_layout->addLayout(below_stream_layout, 1, 0, 1, 2);

  below_stream_layout->addWidget(stream_title, 0, 0);
  below_stream_layout->addWidget(viewer_count, 0, 2);

  if (is_host())
    addToolBar(toolbar);

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
    connect(servc->client.get(), &MercuryClient::chat_message_received, this,
            &StreamWindow::new_chat_message);

  // connect chat message sent on side pane
  connect(side_pane, &SidePane::send_chat_message, this,
          &StreamWindow::send_chat_message);

  // connect viewer count updated for host
  if (is_host())
  {
    connect(servh->server.get(), &MercuryServer::client_connected, this,
            [&]()
            {
              servh->viewer_count++;
              viewer_count_updated(servh->viewer_count);
            });
    connect(servh->server.get(), &MercuryServer::client_disconnected, this,
            [&]()
            {
              servh->viewer_count--;
              viewer_count_updated(servh->viewer_count);
            });
  }

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
  main_layout->setColumnMinimumWidth(0, 1280);
  main_layout->setColumnMinimumWidth(1, 400);
  main_layout->setRowMinimumHeight(0, 720);
  main_layout->setRowMinimumHeight(1, 75);

  display = new QWidget(this);
  side_pane = new SidePane(this);

  std::function<bool(QImage &)> video_func = std::bind(
      &StreamWindow::provide_next_video_frame, this, std::placeholders::_1);
  std::function<bool(QBuffer &)> audio_func = std::bind(
      &StreamWindow::provide_next_audio_frame, this, std::placeholders::_1);
  stream_display = new StreamDisplay(this, video_func, audio_func);

  below_stream_layout = new QGridLayout();
  below_stream_layout->setRowMinimumHeight(0, 100);

  stream_title = new QLabel("Stream Title", this);
  if (is_host() && servh->stream_name.size() > 0)
    stream_title->setText(servh->stream_name.c_str());
  if (is_host())
    viewer_count = new QLabel(
        std::format("Viewers: {}", servh->viewer_count).c_str(), this);
  else
    viewer_count = new QLabel("Viewers: 1", this);

  if (is_host())
    toolbar = new HostToolBar(this);
}

void StreamWindow::stream_fully_initialized()
{
  log("Beginning stream playback.", ll::NOTE);
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
  log("Gracefully shutting down...", ll::NOTE);

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
    if (VideoManager::instance().GetVideoImage(img) ==
        VideoManager::VideoImageStatus::SUCCESS)
    {
      servh->server->send_frame("desktop", QAudioBuffer(), QVideoFrame(img));
      next_video = img;
      return true;
    }
    else
    {
      log("Unable to retrieve video image from manager.", ll::WARNING);
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
      log("Jitter buffer empty when frame desired.", ll::WARNING);
      return false;
    }

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
    log("failed to load audio file", ll::ERROR);
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
}

void StreamWindow::stream_name_changed(string new_name)
{
  stream_title->setText(new_name.c_str());
}

void StreamWindow::new_chat_message(string alias, string msg)
{
  side_pane->new_chat_message({alias, msg});
}
