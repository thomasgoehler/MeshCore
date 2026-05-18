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

#ifdef WITH_SERIAL_FORWARD_CHANNEL
  strncpy(_channel_filter, WITH_SERIAL_FORWARD_CHANNEL, sizeof(_channel_filter) - 1);
  _channel_filter[sizeof(_channel_filter) - 1] = '\0';
  _channel_enabled = true;
#else
  _channel_filter[0] = '\0';
  _channel_enabled = true;
#endif

#ifdef WITH_SERIAL_FORWARD_DM
  _dm_enabled = true;
#else
  _dm_enabled = false;
#endif

  _rx_pos = 0;
}

void SerialForward::loop() {
#ifdef WITH_SERIAL_FORWARD_RX
  while (WITH_SERIAL_FORWARD.available()) {
    char c = (char)WITH_SERIAL_FORWARD.read();
    if (c == '\n') {
      _rx_buf[_rx_pos] = '\0';
      if (_rx_pos > 0) processCommand(_rx_buf);
      _rx_pos = 0;
    } else if (c != '\r' && _rx_pos < sizeof(_rx_buf) - 1) {
      _rx_buf[_rx_pos++] = c;
    }
  }
#endif
}

// Commands:
//   SET_CHANNEL|Alarm   → forward only "Alarm" channel
//   SET_CHANNEL|        → forward all channels
//   SET_DM|1            → enable DM forwarding
//   SET_DM|0            → disable DM forwarding
void SerialForward::processCommand(const char* cmd) {
  const char* sep = strchr(cmd, '|');
  if (!sep) return;

  char key[32];
  size_t key_len = sep - cmd;
  if (key_len >= sizeof(key)) return;
  memcpy(key, cmd, key_len);
  key[key_len] = '\0';

  const char* value = sep + 1;

  if (strcmp(key, "SET_CHANNEL") == 0) {
    if (strcmp(value, "0") == 0) {
      _channel_enabled = false;
    } else if (strcmp(value, "*") == 0) {
      _channel_enabled = true;
      _channel_filter[0] = '\0';
    } else {
      _channel_enabled = true;
      strncpy(_channel_filter, value, sizeof(_channel_filter) - 1);
      _channel_filter[sizeof(_channel_filter) - 1] = '\0';
    }
  } else if (strcmp(key, "SET_DM") == 0) {
    _dm_enabled = (value[0] == '1');
  }
}

void SerialForward::forwardDM(const char* from_name, uint32_t timestamp, const char* text) {
  if (!_dm_enabled) return;
  WITH_SERIAL_FORWARD.print("DM|");
  WITH_SERIAL_FORWARD.print(from_name);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.print(timestamp);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.println(text);
}

void SerialForward::forwardChannel(const char* channel_name, uint32_t timestamp, const char* text) {
  if (!_channel_enabled) return;
  if (_channel_filter[0] != '\0' && strcmp(channel_name, _channel_filter) != 0) return;
  WITH_SERIAL_FORWARD.print("CH|");
  WITH_SERIAL_FORWARD.print(channel_name);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.print(timestamp);
  WITH_SERIAL_FORWARD.print("|");
  WITH_SERIAL_FORWARD.println(text);
}

SerialForward serial_forward;

#endif
