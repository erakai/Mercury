#include "mconfig.hpp"


#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <QSettings>
#include <sys/stat.h>

/* Checks to see if config file is present */
bool mercury::check_config_file_presence()
{

  const std::string& configFilePath = "../../config/MercuryClientSettings.ini";
  struct stat buffer;
  return (stat(configFilePath.c_str(), &buffer) == 0);

}


/* Driver function for curl_default_settings */
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  std::ofstream* outFile = static_cast<std::ofstream*>(userdata);
  outFile->write(static_cast<const char*>(ptr), size * nmemb);
  return size * nmemb;
}


/* Curls default settings file from github */
void mercury::curl_default_config()
{

  const std::string url = "https://raw.githubusercontent.com/erakai/Mercury/main/config/MercuryClientSettings.ini";
  const std::string outputFilename = "../../config/MercuryClientSettings.ini";

  CURL* curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Error: curl_easy_init() failed\n";
  }

  std::ofstream outFile(outputFilename, std::ios::binary);
  if (!outFile) {
    std::cerr << "Error: Could not open file for writing\n";
    curl_easy_cleanup(curl);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
  }

  outFile.close();
  curl_easy_cleanup(curl);

}

void mercury::change_host_settings()
{

  QSettings settings("../../config/MercuryClientSettings.ini", QSettings::IniFormat);

  std::cout << "Stream Resolution: " << settings.value("HostSettings/StreamResolution").toInt() << std::endl;
  settings.setValue("HostSettings/StreamResolution", 1440);

}
