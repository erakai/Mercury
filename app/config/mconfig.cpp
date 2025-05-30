#include "mconfig.hpp"
#include "filedownloader.hpp"

#include <fstream>
#include <QRegularExpression>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QApplication>
#include <QCoreApplication>

int FPS = 5;
std::shared_ptr<QSettings> mercury_settings;

/* Checks to see if config file is present */
bool mercury::check_config_file_presence()
{
  QString app_dir = QCoreApplication::applicationDirPath();
  QFile config_path(app_dir + "/config/MercuryClientSettings.ini");
  return config_path.exists();
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

  QString app_dir = QCoreApplication::applicationDirPath();
  QDir dir(app_dir + "/config");

  if (!dir.exists())
  {
    if (QDir().mkpath(dir.path()))
    {
      qInfo("Created config directory successfully.");
    }
    else
    {
      qFatal("Unable to create config directory.");
    }
  }

  const QString &outputFilename = app_dir + "/config/MercuryClientSettings.ini";
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

QList<QString> mercury::get_blacklist(QSettings &settings)
{
  QList<QString> ret;
  QString val = settings.value("HostSettings/Blacklist").toString();

  return val.split(u';').toVector();
}

bool mercury::get_blacklist_enabled(QSettings &settings)
{
  return settings.value("HostSettings/BlacklistEnabled").toBool();
}

bool mercury::get_whitelist_enabled(QSettings &settings)
{
  return settings.value("HostSettings/WhitelistEnabled").toBool();
}

QList<QString> mercury::get_whitelist(QSettings &settings)
{
  QList<QString> ret;
  QString val = settings.value("HostSettings/Whitelist").toString();

  return val.split(u';').toVector();
}

void mercury::save_all_settings(QString displayName, bool darkMode,
                                int hostStreamResOption,
                                int hostFramerateOption, int maxViewerCount,
                                int defaultClientUdpPort,
                                int defaultHostTcpPort, int defaultHostUdpPort,
                                bool blacklistEnabled, bool whitelistEnabled,
                                QString blacklist, QString whitelist)
{
  mercury_settings->setValue("GeneralSettings/Alias", displayName);
  mercury_settings->setValue("GeneralSettings/DarkMode", darkMode);
  mercury_settings->setValue("ClientSettings/DefaultClientUdpPort",
                             defaultClientUdpPort);
  mercury_settings->setValue("HostSettings/StreamResolution",
                             hostStreamResOption);
  mercury_settings->setValue("HostSettings/FrameRate", hostFramerateOption);
  mercury_settings->setValue("HostSettings/MaxViewers", maxViewerCount);
  mercury_settings->setValue("HostSettings/DefaultHostTcpPort",
                             defaultHostTcpPort);
  mercury_settings->setValue("HostSettings/DefaultHostUdpPort",
                             defaultHostUdpPort);

  mercury_settings->setValue("HostSettings/WhitelistEnabled", whitelistEnabled);
  mercury_settings->setValue("HostSettings/BlacklistEnabled", blacklistEnabled);

  mercury_settings->setValue("HostSettings/Whitelist", whitelist);
  mercury_settings->setValue("HostSettings/Blacklist", blacklist);

  update_styles(darkMode);
}

void mercury::update_styles(bool darkMode)
{
  QString app_dir = QCoreApplication::applicationDirPath();
  QString styleSheetFilename = app_dir + "/styles/light.qss";
  if (mercury::get_dark_mode(*mercury_settings))
  {
    styleSheetFilename = app_dir + "/styles/dark.qss";
  }
  QFile styleSheetFile(styleSheetFilename);
  if (styleSheetFile.open(QFile::ReadOnly))
  {
    QString styles = styleSheetFile.readAll();
    QApplication *app = qobject_cast<QApplication *>(QApplication::instance());
    app->setStyleSheet(styles);
  }
  else
  {
    qWarning("Failed to load QSS stylesheet.");
  }
}
