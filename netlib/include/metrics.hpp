#pragma once

#include "netlib_global.h"
#include <QDateTime>
#include <cstdint>

class NETLIB_EXPORT Metrics
{
public:
  void init_metrics() { last_reset = QDateTime::currentMSecsSinceEpoch(); }

  void reset()
  {
    init_metrics();
    frames = 0;
    throughput = 0;
    expected_datagrams = 0;
    datagrams_received = 0;
  }

  float retrieve_fps()
  {
    uint64_t current = QDateTime::currentMSecsSinceEpoch();
    double elapsed_s = (current - last_reset) / 1000.0;

    return frames / elapsed_s;
  }

  float retrieve_loss()
  {
    return (expected_datagrams - datagrams_received) /
           ((float) expected_datagrams);
  }

  uint16_t retrieve_throughput()
  {
    uint64_t current = QDateTime::currentMSecsSinceEpoch();
    double elapsed_s = (current - last_reset) / 1000.0;

    return throughput / elapsed_s;
  }

  void register_frame() { frames++; }
  void register_data_received(int bits) { throughput += bits; }

  void register_datagram_received() { datagrams_received++; }
  void register_expected_datagrams(int fragments)
  {
    expected_datagrams += fragments;
  }

private:
  // In ms since epoch
  uint64_t last_reset;

  uint16_t frames = 0;
  uint32_t throughput = 0;

  // For loss calculation
  uint16_t expected_datagrams = 0;
  uint16_t datagrams_received = 0;
};
