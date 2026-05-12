#pragma once

#ifdef WITH_SERIAL_FORWARD

#include <Arduino.h>

class SerialForward {
public:
  void begin();
  void loop();
  void forwardDM(const char* from_name, uint32_t timestamp, const char* text);
  void forwardChannel(const char* channel_name, uint32_t timestamp, const char* text);

private:
  void processCommand(const char* cmd);

  char _channel_filter[32];
  bool _channel_enabled;
  bool _dm_enabled;
  char _rx_buf[64];
  uint8_t _rx_pos;
};

extern SerialForward serial_forward;

#endif
