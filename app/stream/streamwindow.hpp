#pragma once

#include "chatpane.hpp"
#include "streamdisplay.hpp"
#include "streamservice.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>

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
  StreamWindow(std::string alias, shared_ptr<HostService> host_data,
               QWidget *parent = nullptr);
  StreamWindow(std::string alias, shared_ptr<ClientService> client_data,
               QWidget *parent = nullptr);

  bool is_host() { return mode == MercuryMode::HOST; }
  bool is_client() { return mode == MercuryMode::CLIENT; }

public slots:

private:
  /*
  Helper functions for setting the ui up.
  */
  void set_up();
  void configure_menu_bar();
  void connect_signals_and_slots();
  void initialize_primary_ui_widgets();

  // All important widgets we will be using to set up UI
  // Because a main window only has one central widget, we will be creating a
  // dummy one (display) to paste a layout onto.
  // https://doc.qt.io/qt-6/qtwidgets-tutorials-widgets-nestedlayouts-example.html.
  QWidget *display;
  QGridLayout *main_layout;
  ChatPane *chat_pane;
  StreamDisplay *stream_display;
  QGridLayout *below_stream_layout;

  QLabel *stream_title;
  QLabel *viewer_count;

  // Determines whether or not this window is a client or a host
  const MercuryMode mode;

  // Must be <18 characters
  std::string alias;

  // Only one of these will not be null - check mode first
  // These contain all specific data to being a host/client (networking, viewer
  // count, stream name, etc)
  const shared_ptr<HostService> host = nullptr;
  const shared_ptr<ClientService> client = nullptr;
};
