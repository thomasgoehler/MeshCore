#include "SerialForward.h"

#ifdef WITH_SERIAL_FORWARD

#ifndef WITH_SERIAL_FORWARD_TX
#error "WITH_SERIAL_FORWARD_TX must be defined"
#endif

void SerialForward::begin() {
#if defined(RP2040_PLATFORM)
  ((SerialUART *)&WITH_SERIAL_FORWARD)->setTX(WITH_SERIAL_FORWARD_TX);
#ifdef WITH_SERIAL_FORWARD_RX
  ((SerialUART *)&WITH_SERIAL_FORWARD)->setRX(WITH_SERIAL_FORWARD_RX);
#endif
#elif defined(ESP32)
  int rx_pin = -1;
#ifdef WITH_SERIAL_FORWARD_RX
  rx_pin = WITH_SERIAL_FORWARD_RX;
#endif
  ((HardwareSerial *)&WITH_SERIAL_FORWARD)->setPins(rx_pin, WITH_SERIAL_FORWARD_TX);
#endif
  WITH_SERIAL_FORWARD.begin(115200);
}

void SerialForward::forwardDM(const char* from_name, uint32_t timestamp, const char* text) {
#ifdef WITH_SERIAL_FORWARD_DM
  WITH_SERIAL_FORWARD.print("DM|");
  WITH_SERIAL_FORWARD.print(from_name);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.print(timestamp);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.println(text);
#endif
}

void SerialForward::forwardChannel(const char* channel_name, uint32_t timestamp, const char* text) {
#ifdef WITH_SERIAL_FORWARD_CHANNEL
  if (strcmp(channel_name, WITH_SERIAL_FORWARD_CHANNEL) != 0) return;
#endif
  WITH_SERIAL_FORWARD.print("CH|");
  WITH_SERIAL_FORWARD.print(channel_name);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.print(timestamp);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.println(text);
}

SerialForward serial_forward;

#endif
