#ifndef MCONFIG_HPP
#define MCONFIG_HPP

#include <QSettings>

// TODO: Move into settings as default FPS
// Right now this is pulled by streamdisplay, and updated
// by performance tab.
extern int FPS;

namespace mercury
{

bool check_config_file_presence();

void curl_default_config();

bool get_dark_mode(QSettings &settings);

QString get_alias(QSettings &settings);

int get_client_stream_res(QSettings &settings);

int get_host_stream_res(QSettings &settings);

int get_host_stream_fps(QSettings &settings);

int get_host_max_viewers(QSettings &settings);

int get_defaultClientUdpPort(QSettings &settings);

int get_defaultHostTcpPort(QSettings &settings);

int get_defaultHostUdpPort(QSettings &settings);

void save_all_settings(QString displayName, bool darkMode,
                       int clientStreamResOption, int hostStreamResOption,
                       int hostFramerateOption, int maxViewerCount,
                       int defaultClientUdpPort, int defaultHostTcpPort,
                       int defaultHostUdpPort);
} // namespace mercury

#endif // MCONFIG_HPP
