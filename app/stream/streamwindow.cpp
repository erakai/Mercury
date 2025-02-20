#include "streamwindow.hpp"
#include <QDebug>

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
  connect_signals_and_slots();
  configure_menu_bar();

  display->setLayout(main_layout);
  setCentralWidget(display);

  main_layout->addWidget(stream_display, 0, 0, 2, 2);
  main_layout->addWidget(chat_pane, 0, 2, 1, 3);
  main_layout->addLayout(below_stream_layout, 2, 0, 1, 2);

  below_stream_layout->addWidget(stream_title, 0, 0);
}

void StreamWindow::configure_menu_bar()
{
}

void StreamWindow::connect_signals_and_slots()
{
}

void StreamWindow::initialize_primary_ui_widgets()
{
  main_layout = new QGridLayout();
  main_layout->setColumnMinimumWidth(0, 400);
  main_layout->setColumnMinimumWidth(1, 400);
  main_layout->setColumnMinimumWidth(2, 150);
  main_layout->setRowMinimumHeight(0, 200);
  main_layout->setRowMinimumHeight(1, 200);

  display = new QWidget(this);
  chat_pane = new ChatPane(this);
  stream_display = new StreamDisplay(this);

  below_stream_layout = new QGridLayout();
  below_stream_layout->setRowMinimumHeight(0, 100);

  stream_title = new QLabel("Stream Title", this);
  if (is_host() && host->stream_name.size() > 0)
    stream_title->setText(host->stream_name.c_str());
}
