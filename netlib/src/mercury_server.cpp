#include "mercury_server.hpp"

void MercuryServer::connect_signals_and_slots()
{
}

void MercuryServer::start_server()
{
}

void MercuryServer::close_server()
{
}

int MercuryServer::convert_alias_to_id(char *alias)
{
  return -1;
}

void MercuryServer::new_client_connection()
{
}

void MercuryServer::force_disconnect_client(int id)
{
}

void MercuryServer::process_received_hstp_messages(int id)
{
}

void MercuryServer::send_frame(char source[12], QAudioBuffer audio,
                               QPixmap video)
{
}
