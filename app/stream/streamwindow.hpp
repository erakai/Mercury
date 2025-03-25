#pragma once

#include "hosttoolbar.hpp"
#include "sidepane.hpp"
#include "streamdisplay.hpp"
#include "streamservice.hpp"
#include <QAudioBuffer>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qwidget.h>

using namespace std;

enum MercuryMode
{
  HOST,
  CLIENT
};

class StreamWindow : public QMainWindow
{
  Q_OBJECT;

public:
  StreamWindow(string alias, shared_ptr<HostService> host_data,
               QWidget *parent = nullptr);
  StreamWindow(string alias, shared_ptr<ClientService> client_data,
               QWidget *parent = nullptr);

  bool is_host() { return mode == MercuryMode::HOST; }
  bool is_client() { return mode == MercuryMode::CLIENT; }

  bool provide_next_video_frame(QImage &next_video);
  bool provide_next_audio_frame(QBuffer &next_audio);

  void closeEvent(QCloseEvent *event) override;

public slots:
  // This should be connected to a signal from either MercuryClient (if this is
  // a client stream) that signals it has received enough information to fill
  // its jitter buffer, or to a signal from some window manager that signals it
  // has started to record the desktop. It won't play until this has been
  // called.
  void stream_fully_initialized();

  // Called to close the stream (if hosting) or leave (if client)
  void shut_down_window();

  // Sends a chat message out
  void send_chat_message(string message);

  // The rest are generic update slots that should be connected to a signal
  // either in the client or server

  void viewer_count_updated(int new_count);
  void stream_name_changed(string host_alias, string new_name);
  void new_chat_message(string alias, string msg);

  // These next two are host-only
  void viewer_disconnected(int id, std::string alias);
  // This is called after a client is validated
  void viewer_connected(int id, std::string _alias);

private:
  /*
  Helper functions for setting the ui up.
  */
  void set_up();
  void configure_menu_and_tool_bar();
  void connect_signals_and_slots();
  void initialize_primary_ui_widgets();

  // All important widgets we will be using to set up UI
  // Because a main window only has one central widget, we will be creating a
  // dummy one (display) to paste a layout onto.
  // https://doc.qt.io/qt-6/qtwidgets-tutorials-widgets-nestedlayouts-example.html.
  QWidget *display;
  QGridLayout *main_layout;
  SidePane *side_pane;
  StreamDisplay *stream_display;
  QGridLayout *below_stream_layout;

  QLabel *stream_title;
  QLabel *host_name;
  QLabel *viewer_count;

  QMenu *stream_menu;
  QAction *stop_or_leave_stream_action;

  HostToolBar *toolbar;

  // Determines whether or not this window is a client or a host
  const MercuryMode mode;

  // Must be <18 characters
  string alias;

  // Only one of these will not be null - check mode first
  // These contain all specific data to being a host/client (networking, viewer
  // count, stream name, etc)
  const shared_ptr<HostService> servh = nullptr;
  const shared_ptr<ClientService> servc = nullptr;
};
