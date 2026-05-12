#pragma once

#ifdef WITH_SERIAL_FORWARD

#include <Arduino.h>

class SerialForward {
public:
  void begin();
  void forwardDM(const char* from_name, uint32_t timestamp, const char* text);
  void forwardChannel(const char* channel_name, uint32_t timestamp, const char* text);
};

extern SerialForward serial_forward;

#endif
