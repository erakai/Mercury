#pragma once

#include <chrono>
#include <filesystem>
#include <source_location>
#include <string>

/*
=========================================================================
This file lets us log issues and include time/source location.

The last argument must be a log level. Example call:

  log("Test: %d", val, ll::WARNING);
=========================================================================
*/

extern std::chrono::time_point<std::chrono::system_clock> boot_time;
std::string get_time_since_boot();

// Log levels
extern int MINIMUM_LOG_LEVEL; // increase this to filter what is printed
enum class ll
{
  NOTE = 0,
  WARNING = 1,
  ERROR = 2,
  CRITICAL = 3,
};
std::string get_log_level_string(ll lev);

// This struct is necessary to have a function accept variable arg count,
// while also providing the source information of the caller.
struct FormatWithLocation
{
  const char *value;
  std::source_location loc;

  FormatWithLocation(const char *s, const std::source_location &l =
                                        std::source_location::current())
      : value(s), loc(l)
  {
  }
};

template <typename... Args> void log(FormatWithLocation fmt, Args &&...args)
{
  // Retrieve last argument and check if it is a log level.
  // This will fail on compile if not properly provided by user.
  ll level = ll::WARNING;
  auto &&last = std::get<sizeof...(Args) - 1>(std::forward_as_tuple(args...));
  level = last;

  // Filter out logs we don't want
  if (static_cast<int>(level) < MINIMUM_LOG_LEVEL)
    return;

  // Print out log message
  std::filesystem::path path(fmt.loc.file_name());
  std::string current_time = get_time_since_boot();
  std::string path_str = path.filename().string();

  printf("[%s (%s) (%03d:%02d %s)] ", current_time.c_str(),
         get_log_level_string(level).c_str(), fmt.loc.line(), fmt.loc.column(),
         path_str.c_str());
  printf(fmt.value, args...);
  printf("\n");
}
