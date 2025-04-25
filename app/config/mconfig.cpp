#include "mconfig.hpp"
#include "filedownloader.hpp"

#include <fstream>
#include <QDir>
#include <QSettings>
#include <sys/stat.h>

int FPS = 5;

/* Checks to see if config file is present */
bool mercury::check_config_file_presence()
{

  const std::string &configFilePath = "config/MercuryClientSettings.ini";
  struct stat buffer;
  return (stat(configFilePath.c_str(), &buffer) == 0);
}

/* Driver function for curl_default_settings */
size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
  std::ofstream *outFile = static_cast<std::ofstream *>(userdata);
  outFile->write(static_cast<const char *>(ptr), size * nmemb);
  return size * nmemb;
}

/* Curls default settings file from github */
void mercury::curl_default_config()
{

  const QString &url = "https://raw.githubusercontent.com/erakai/Mercury/"
                       "main/config/DefaultClientSettings.ini";
  const QString &outputFilename = "config/MercuryClientSettings.ini";

  QDir dir("config");

  if (!dir.exists())
  {
    if (dir.mkpath("."))
    {
      qInfo("Created config directory successfully.");
    }
    else
    {
      qFatal("Unable to create config directory.");
    }
  }

  mercury::download_file(url, outputFilename);

  return;
}

bool mercury::get_dark_mode(QSettings &settings)
{

  return settings.value("GeneralSettings/DarkMode").toBool();
}

QString mercury::get_alias(QSettings &settings)
{

  return settings.value("GeneralSettings/Alias").toString();
}

int mercury::get_client_stream_res(QSettings &settings)
{

  return settings.value("ClientSettings/StreamResolution").toInt();
}

int mercury::get_host_stream_res(QSettings &settings)
{

  return settings.value("HostSettings/StreamResolution").toInt();
}

int mercury::get_host_stream_fps(QSettings &settings)
{

  return settings.value("HostSettings/FrameRate").toInt();
}

int mercury::get_host_max_viewers(QSettings &settings)
{

  return settings.value("HostSettings/MaxViewers").toInt();
}

int mercury::get_defaultClientUdpPort(QSettings &settings)
{
  return settings.value("ClientSettings/DefaultClientUdpPort").toInt();
}

int mercury::get_defaultHostTcpPort(QSettings &settings)
{
  return settings.value("HostSettings/DefaultHostTcpPort").toInt();
}

int mercury::get_defaultHostUdpPort(QSettings &settings)
{
  return settings.value("HostSettings/DefaultHostUdpPort").toInt();
}

void mercury::save_all_settings(QString displayName, bool darkMode,
                                int clientStreamResOption,
                                int hostStreamResOption,
                                int hostFramerateOption, int maxViewerCount,
                                int defaultClientUdpPort,
                                int defaultHostTcpPort, int defaultHostUdpPort)
{
  QSettings settings("config/MercuryClientSettings.ini", QSettings::IniFormat);

  settings.setValue("GeneralSettings/Alias", displayName);
  settings.setValue("GeneralSettings/DarkMode", darkMode);
  settings.setValue("ClientSettings/StreamResolution", clientStreamResOption);
  settings.setValue("ClientSettings/DefaultClientUdpPort",
                    defaultClientUdpPort);
  settings.setValue("HostSettings/StreamResolution", hostStreamResOption);
  settings.setValue("HostSettings/FrameRate", hostFramerateOption);
  settings.setValue("HostSettings/MaxViewers", maxViewerCount);
  settings.setValue("HostSettings/DefaultHostTcpPort", defaultHostTcpPort);
  settings.setValue("HostSettings/DefaultHostUdpPort", defaultHostUdpPort);
}
