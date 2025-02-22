#include "mconfig.hpp"

#include <iostream>
#include <sys/stat.h>

bool mercury::check_config_file_presence()
{

  const std::string& configFilePath = "../../config/MercuryClientSettingss.ini";
  struct stat buffer;
  return (stat(configFilePath.c_str(), &buffer) == 0);

}
