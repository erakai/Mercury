#include "mconfig.hpp"

#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <sys/stat.h>

bool mercury::check_config_file_presence()
{

  const std::string& configFilePath = "../../config/MercuryClientSettings.ini";
  struct stat buffer;
  return (stat(configFilePath.c_str(), &buffer) == 0);

}

size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  std::ofstream* outFile = static_cast<std::ofstream*>(userdata);
  outFile->write(static_cast<const char*>(ptr), size * nmemb);
  return size * nmemb;
}

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
