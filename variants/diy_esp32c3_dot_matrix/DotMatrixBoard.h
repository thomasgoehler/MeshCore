#pragma once

#include <helpers/ESP32Board.h>
#include <Arduino.h>

#include <driver/rtc_io.h>
#include <driver/uart.h>

// Vbus ADC voltage divider: R2=220k (high side), R3=100k (low side)
// Vbus = ADC * (220k + 100k) / 100k * Vref / resolution
#define VBUS_ADC_MULTIPLIER  3.2  // (220+100)/100 = 3.2

class DotMatrixBoard : public ESP32Board {
public:
  void begin() {
    ESP32Board::begin();

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_DEEPSLEEP) {
      long wakeup_source = esp_sleep_get_gpio_wakeup_status();
      if (wakeup_source & (1 << P_LORA_DIO_1)) {
        startup_reason = BD_STARTUP_RX_PACKET;
      }
    }

#ifdef PIN_VBAT_READ
    pinMode(PIN_VBAT_READ, INPUT);
#endif

#ifdef P_LORA_TX_LED
    pinMode(P_LORA_TX_LED, OUTPUT);
    digitalWrite(P_LORA_TX_LED, LOW);
#endif
  }

  void enterDeepSleep(uint32_t secs, int8_t wake_pin = -1) {
    gpio_set_direction(gpio_num_t(P_LORA_DIO_1), GPIO_MODE_INPUT);
    if (wake_pin >= 0) {
      gpio_set_direction((gpio_num_t)wake_pin, GPIO_MODE_INPUT);
    }

    gpio_deep_sleep_hold_en();
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    if (wake_pin >= 0) {
      esp_deep_sleep_enable_gpio_wakeup((1 << P_LORA_DIO_1) | (1 << wake_pin), ESP_GPIO_WAKEUP_GPIO_HIGH);
    } else {
      esp_deep_sleep_enable_gpio_wakeup(1 << P_LORA_DIO_1, ESP_GPIO_WAKEUP_GPIO_HIGH);
    }

    if (secs > 0) {
      esp_sleep_enable_timer_wakeup(secs * 1000000);
    }

    esp_deep_sleep_start();
  }

#ifdef P_LORA_TX_LED
  void onBeforeTransmit() override {
    digitalWrite(P_LORA_TX_LED, HIGH);
  }
  void onAfterTransmit() override {
    digitalWrite(P_LORA_TX_LED, LOW);
  }
#endif

  uint16_t getBattMilliVolts() override {
#ifdef PIN_VBAT_READ
    analogReadResolution(12);
    uint32_t raw = 0;
    for (int i = 0; i < 8; i++) {
      raw += analogRead(PIN_VBAT_READ);
    }
    raw = raw / 8;
    return (VBUS_ADC_MULTIPLIER * 3.3 * raw * 1000) / 4096;
#else
    return 0;
#endif
  }

  const char* getManufacturerName() const override {
    return "DIY ESP32C3 Dot-Matrix";
  }
};
