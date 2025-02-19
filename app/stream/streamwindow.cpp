#include "streamwindow.hpp"
#include <QtCore/qnamespace.h>

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

  configure_menu_bar();
  connect_signals_and_slots();
  initialize_primary_ui_widgets();

  // setCentralWidget(...)
}

void StreamWindow::configure_menu_bar()
{
}

void StreamWindow::connect_signals_and_slots()
{
}

void StreamWindow::initialize_primary_ui_widgets()
{
}
