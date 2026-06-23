#pragma once

#include <helpers/ESP32Board.h>
#include <Arduino.h>

class SuperMiniBoard : public ESP32Board {
public:
  void begin() {
    ESP32Board::begin();

  #ifdef PIN_VBAT_READ
    pinMode(PIN_VBAT_READ, INPUT);
  #endif

  #ifdef LORA_TX_BOOST_PIN
    pinMode(LORA_TX_BOOST_PIN, OUTPUT);
    digitalWrite(LORA_TX_BOOST_PIN, HIGH);
  #endif

  #ifdef P_LORA_TX_LED
    pinMode(P_LORA_TX_LED, OUTPUT);
    digitalWrite(P_LORA_TX_LED, HIGH);  // both LEDs active LOW → HIGH = off
  #endif
  }

  void onBeforeTransmit() override {
  #ifdef P_LORA_TX_LED
    digitalWrite(P_LORA_TX_LED, LOW);   // on
  #endif
  }
  void onAfterTransmit() override {
  #ifdef P_LORA_TX_LED
    digitalWrite(P_LORA_TX_LED, HIGH);  // off
  #endif
  }

  // R2=220k / R3=100k voltage divider → factor 3.2
  uint16_t getBattMilliVolts() override {
  #ifdef PIN_VBAT_READ
    analogReadResolution(12);
    uint32_t raw = 0;
    for (int i = 0; i < 4; i++) {
      raw += analogReadMilliVolts(PIN_VBAT_READ);
    }
    raw = raw / 4;

    return (uint16_t)(raw * 3.2);
  #else
    return 0;
  #endif
  }

  const char* getManufacturerName() const override {
    return "ESP32-C3 SuperMini";
  }
};
