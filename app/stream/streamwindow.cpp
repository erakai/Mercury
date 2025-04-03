#include "streamwindow.hpp"
#include "config/mconfig.hpp"
#include "hstp.hpp"
#include "singleton/videomanager.h"
#include <QApplication>
#include <QMenuBar>
#include <QScreen>
#include <QtDebug>
#include <QMouseEvent>
#include <QAudioSink>

StreamWindow::StreamWindow(std::string alias, shared_ptr<HostService> host_data,
                           QWidget *parent)
    : QMainWindow(parent), mode(MercuryMode::HOST), alias(alias),
      servh(host_data)
{
}

StreamWindow::StreamWindow(std::string alias,
                           shared_ptr<ClientService> client_data,
                           QWidget *parent)
    : QMainWindow(parent), mode(MercuryMode::CLIENT), alias(alias),
      servc(client_data)
{
}

bool StreamWindow::set_up()
{
  setWindowTitle("Mercury");
  setAttribute(Qt::WA_DeleteOnClose);

  initialize_primary_ui_widgets();
  configure_menu_and_tool_bar();

  display->setLayout(main_layout);
  display->setStyleSheet("background-color: #3C4143;");
  setCentralWidget(display);

  // Create a container widget to stack the stream display and the annotation
  // display.
  QWidget *videoAnnotationContainer = new QWidget(this);
  QGridLayout *containerLayout = new QGridLayout(videoAnnotationContainer);
  containerLayout->setContentsMargins(0, 0, 0, 0);
  containerLayout->setSpacing(0);
  // Add all widgets in the same cell.
  containerLayout->addWidget(stream_display, 0, 0);
  containerLayout->addWidget(reaction_display, 0, 0);
  containerLayout->addWidget(annotation_display, 0, 0);

  stream_display->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  annotation_display->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);

  annotation_display->raise();

  // Create the PaintToolWidget and add it at the top of the display.
  paint_tool = new PaintToolWidget(this);
  main_layout->addWidget(paint_tool, 0, 0);
  main_layout->addWidget(videoAnnotationContainer, 1, 0);
  main_layout->addWidget(side_pane, 0, 1, 3, 1);
  main_layout->addWidget(stream_info, 2, 0);

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

  const int height_of_paint_bar = 1;
  main_layout->setRowStretch(0, height_of_paint_bar);
  main_layout->setRowStretch(1, 80 - height_of_paint_bar);
  main_layout->setRowStretch(2, 20);

  main_layout->setColumnStretch(0, 75);
  main_layout->setColumnStretch(1, 25);

  // Center this window
  move(QGuiApplication::screens().at(0)->geometry().center() -
       frameGeometry().center());

  connect_signals_and_slots();

  if (is_client() && servc->client->hstp_sock()->waitForDisconnected(1000))
  {
    qCritical("Client disconnected by host, likely due to incorrect password.");
    return false;
  }

  this->showMaximized(); // Sets Window size to max
  return true;
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

  // connect annotation received to annotation display
  if (is_host())
    connect(servh->server.get(), &MercuryServer::annotation_received, this,
            &StreamWindow::new_annotation);

  if (is_client())
    connect(servc->client->hstp_processor().get(),
            &HstpProcessor::received_annotation, this,
            &StreamWindow::new_annotation);

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

  // connect reaction sent from stream info
  connect(stream_info, &StreamInfo::renderAndSendReaction, this,
          [=, this](ReactionPanel::Reaction reaction)
          {
            reaction_display->addReaction(reaction);
            this->send_reaction(reaction);
          });

  // connect host to receive reactions
  if (is_host())
    connect(servh->server.get(), &MercuryServer::reaction_received, this,
            &StreamWindow::new_reaction);

  // connect reaction received by client (to then display for client)
  if (is_client())
    connect(servc->client->hstp_processor().get(),
            &HstpProcessor::received_reaction, this,
            &StreamWindow::new_reaction);

  // connect stream title for client
  if (is_client())
    connect(servc->client->hstp_processor().get(),
            &HstpProcessor::received_stream_title, this,
            [=, this](const char alias[ALIAS_SIZE], const char *stream_title)
            {
              this->stream_name_changed(std::string(alias),
                                        std::string(stream_title));
            });

  // connect stream start time for client
  if (is_client())
    connect(servc->client->hstp_processor().get(),
            &HstpProcessor::received_stream_start_time, this,
            [=, this](const char alias[ALIAS_SIZE], uint32_t timestamp)
            { this->stream_start_time_changed(timestamp); });

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

  // connect unstable network signal to icon on client
  if (is_client())
    connect(servc->client.get(), &MercuryClient::connection_stablity_updated,
            this,
            [=, this](bool new_status)
            {
              stream_info->setUnstableNetworkIndicator(
                  !new_status, servc->client->get_connection_reason());
            });
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

  reaction_display = new ReactionDisplay(this);

  annotation_display = new AnnotationDisplay(this);
  annotation_display->installEventFilter(this);

  stream_info = new StreamInfo(this, "Host\'s Stream", "Host");

  if (is_host() && servh->stream_name.size() > 0)
    stream_info->setStreamTitle(servh->stream_name.c_str());
  if (is_host())
  {
    stream_info->setViewerCount(servh->viewer_count);
    stream_info->setHostName(alias.c_str());
    stream_info->setStreamStartTime(servh->start_timestamp);
  }
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

void StreamWindow::send_reaction(ReactionPanel::Reaction reaction)
{
  qDebug() << "reaction received in streamwindow.cpp";
  if (is_host())
    servh->server->forward_reaction(-1, static_cast<uint32_t>(reaction));
  if (is_client())
    servc->client->send_reaction(static_cast<uint32_t>(reaction));
}

void StreamWindow::send_annotation(HSTP_Annotation annotation)
{
  if (is_host())
  {
    // qDebug() << "Sending annotation as host";
    servh->server->forward_annotations(-1, annotation);
  }

  if (is_client())
  {
    // qDebug() << "Sending annotation as client";
    servc->client->send_annotations(annotation);
  }
}

void StreamWindow::viewer_count_updated(int new_count)
{
  stream_info->setViewerCount(new_count);

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
  stream_info->setStreamTitle((new_name).c_str());

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
    stream_info->setHostName(host_alias.c_str());
  }
}

void StreamWindow::stream_start_time_changed(uint32_t timestamp)
{
  // timestamp is in seconds since epoch
  if (is_client())
  {
    stream_info->setStreamStartTime(timestamp);
  }
}

void StreamWindow::new_chat_message(string alias, string msg)
{
  side_pane->get_chat_tab()->new_chat_message({alias, msg});
}

void StreamWindow::new_reaction(string alias, uint32_t reaction)
{
  reaction_display->addReaction(static_cast<ReactionPanel::Reaction>(reaction));
}

void StreamWindow::viewer_connected(int id, std::string _alias)
{
  servh->viewer_count++;
  viewer_count_updated(servh->viewer_count);

  side_pane->get_viewer_list_tab()->viewer_joined(_alias);

  Client &client = servh->server->get_client(id);
  client.handler.init_msg(alias.c_str());
  client.handler.add_option_stream_title(stream_info->getStreamTitle().c_str());
  client.handler.add_option_stream_start_time(
      stream_info->getStreamStartTime());
  client.handler.add_option_viewer_count(servh->viewer_count);
  client.handler.add_option_fps(FPS);
  client.handler.output_msg_to_socket(client.hstp_sock);
}

void StreamWindow::viewer_disconnected(int id, std::string _alias)
{
  servh->viewer_count--;
  viewer_count_updated(servh->viewer_count);

  side_pane->get_viewer_list_tab()->viewer_left(_alias);
}

void StreamWindow::new_annotation(string alias, HSTP_Annotation annotation)
{
  if (annotation.points.size() < 1)
    return;
  else if (annotation.points.size() == 1)
  {
    QPoint p(annotation.points[0].x, annotation.points[0].y);
    annotation_display->addLine(
        p, p, QColor(annotation.red, annotation.green, annotation.blue),
        annotation.thickness);
  }
  else
  {
    for (int i = 1; i < (int) annotation.points.size(); i++)
    {
      QPoint q(annotation.points[i].x, annotation.points[i].y);
      QPoint p(annotation.points[i - 1].x, annotation.points[i - 1].y);
      annotation_display->addLine(
          p, q, QColor(annotation.red, annotation.green, annotation.blue),
          annotation.thickness);
    }
  }
}

bool StreamWindow::eventFilter(QObject *watched, QEvent *event)
{
  if (watched == annotation_display)
  {
    if (event->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      onAnnotationDisplayMousePressed(mouseEvent);
      return true;
    }
    else if (event->type() == QEvent::MouseMove)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      onAnnotationDisplayMouseMoved(mouseEvent);
      return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      onAnnotationDisplayMouseReleased(mouseEvent);
      return true;
    }
  }
  return QMainWindow::eventFilter(watched, event);
}

void StreamWindow::onAnnotationDisplayMousePressed(QMouseEvent *event)
{
  // Start a new annotation by clearing previous points.
  points.clear();
  QPoint pos = event->pos();
  old_point = pos;
  points.push_back(pos);
}

void StreamWindow::onAnnotationDisplayMouseMoved(QMouseEvent *event)
{
  QPoint pos = event->pos();

  PaintToolWidget *toolWidget = paint_tool;
  QColor currentColor = toolWidget->selectedColor();
  int thickness = toolWidget->brushSize();

  // TODO add erase support
  if (toolWidget->isEraseMode())
  {
    thickness *= -1;
  }

  annotation_display->addLine(old_point, pos, currentColor, thickness);
  old_point = pos;
  points.push_back(pos);
}

void StreamWindow::onAnnotationDisplayMouseReleased(QMouseEvent *event)
{
  Q_UNUSED(event);

  PaintToolWidget *toolWidget = paint_tool;
  QColor currentColor = toolWidget->selectedColor();
  int thickness = toolWidget->brushSize();

  // TODO add erase support
  if (toolWidget->isEraseMode())
  {
    thickness *= -1;
  }

  send_annotation(HSTP_Annotation(points, currentColor.red(),
                                  currentColor.green(), currentColor.blue(),
                                  thickness));
  points.clear();
}