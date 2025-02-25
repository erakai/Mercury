#ifndef MCONFIG_HPP
#define MCONFIG_HPP

#include <QSettings>

namespace mercury
{

bool check_config_file_presence();

void curl_default_config();

void change_host_settings();

bool get_visual_mode(QSettings &settings);

QString get_alias(QSettings &settings);

int get_client_stream_res(QSettings &settings);

int get_host_stream_res(QSettings &settings);

int get_host_stream_fps(QSettings &settings);

int get_host_max_viewers(QSettings &settings);
}

#endif // MCONFIG_HPP
