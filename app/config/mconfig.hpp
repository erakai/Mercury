#ifndef MCONFIG_HPP
#define MCONFIG_HPP

#include <QSettings>
#include <QList>

// TODO: Make settings set this as default FPS
// Right now this is pulled by streamdisplay, and updated
// by performance tab.
extern int FPS;

using namespace std;

namespace mercury
{

bool check_config_file_presence();

void curl_default_config();

bool get_dark_mode(QSettings &settings);

QString get_alias(QSettings &settings);

int get_host_stream_res(QSettings &settings);

int get_host_stream_fps(QSettings &settings);

int get_host_max_viewers(QSettings &settings);

int get_defaultClientUdpPort(QSettings &settings);

int get_defaultHostTcpPort(QSettings &settings);

int get_defaultHostUdpPort(QSettings &settings);

QList<QString> get_blacklist(QSettings &settings);
QList<QString> get_whitelist(QSettings &settings);

bool get_blacklist_enabled(QSettings &settings);
bool get_whitelist_enabled(QSettings &settings);

void save_all_settings(QString displayName, bool darkMode,
                       int hostStreamResOption, int hostFramerateOption,
                       int maxViewerCount, int defaultClientUdpPort,
                       int defaultHostTcpPort, int defaultHostUdpPort,
                       bool blacklistEnabled, bool whitelistEnabled,
                       QString blacklist, QString whitelist);
} // namespace mercury

#endif // MCONFIG_HPP
