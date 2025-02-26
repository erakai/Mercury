#include "streamwindow.hpp"
#include "hosttoolbar.hpp"
#include "logger.hpp"
#include <QApplication>
#include <QMenuBar>
#include <QScreen>

StreamWindow::StreamWindow(std::string alias, shared_ptr<HostService> host_data,
                           QWidget *parent)
    : QMainWindow(parent), mode(MercuryMode::HOST), alias(alias),
      host(host_data)
{
  set_up();
}

StreamWindow::StreamWindow(std::string alias,
                           shared_ptr<ClientService> client_data,
                           QWidget *parent)
    : QMainWindow(parent), mode(MercuryMode::CLIENT), alias(alias),
      client(client_data)
{
  set_up();
}

void StreamWindow::set_up()
{
  setWindowTitle("Mercury");
  setAttribute(Qt::WA_DeleteOnClose);

  initialize_primary_ui_widgets();
  configure_menu_and_tool_bar();
  connect_signals_and_slots();

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
  std::function<bool(QAudioBuffer &)> audio_func = std::bind(
      &StreamWindow::provide_next_audio_frame, this, std::placeholders::_1);
  stream_display = new StreamDisplay(this, video_func, audio_func);

  below_stream_layout = new QGridLayout();
  below_stream_layout->setRowMinimumHeight(0, 100);

  stream_title = new QLabel("Stream Title", this);
  if (is_host() && host->stream_name.size() > 0)
    stream_title->setText(host->stream_name.c_str());
  viewer_count = new QLabel("Viewers: 1", this);

  if (is_host())
    toolbar = new HostToolBar(this);
}

void StreamWindow::stream_fully_initialized()
{
  stream_display->begin_playback();
}

void StreamWindow::shut_down_window()
{
  /*
  if (is_host())
    // host->server->fully_disconnect_clients();

  if (is_client())
    // client->client->disconnect();
  */

  close();
}

bool StreamWindow::provide_next_video_frame(QImage &next_video)
{
  if (is_host())
  {
    // acquire video frame from desktop
  }
  else
  {
    // acquire video frame from jitter buffer
  }

  // Testing
  if (!next_video.load("assets/iamcomingtokillyou.jpg", "JPG"))
  {
    log("failed to load picture", ll::ERROR);
    return false;
  };
  return true;

  return false;
}

bool StreamWindow::provide_next_audio_frame(QAudioBuffer &next_audio)
{
  if (is_host())
  {
    // acquire audio frame from desktop
  }
  else
  {
    // acquire audio frame from jitter buffer
  }

  return false;
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
