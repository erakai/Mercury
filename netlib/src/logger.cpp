#include "logger.hpp"

using namespace std::chrono;

time_point<system_clock> boot_time = system_clock::now();

std::string get_time_since_boot()
{
  auto current = system_clock::now();
  auto dur = current - boot_time;

  auto d = duration_cast<duration<long, std::ratio<3600 * 24>>>(dur);
  auto h = duration_cast<hours>(dur -= d);
  auto m = duration_cast<minutes>(dur -= h);
  auto s = duration_cast<seconds>(dur -= m);
  auto ms = duration_cast<milliseconds>(dur -= s);

  return std::format("{:02d}:{:02d}:{:02d}:{:04d}", h.count(), m.count(),
                     s.count(), ms.count());
}

std::string get_log_level_string(ll lev)
{
  if (lev == ll::NOTE)
    return "NOTE";
  if (lev == ll::WARNING)
    return "WARNING";
  if (lev == ll::ERROR)
    return "ERROR";
  return "CRITICAL";
}
