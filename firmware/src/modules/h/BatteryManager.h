#pragma once

#include <Arduino.h>

// Battery voltage monitoring for ESP8266/ESP32
// ESP8266: A0 pin (0-1V, use voltage divider for higher voltages)
// ESP32: ADC1_CHANNEL_0 (GPIO36) or any ADC pin

#define BATTERY_SAMPLES 10
#define BATTERY_UPDATE_MS 30000  // Update every 30 seconds

#ifdef ESP8266
  #define BATTERY_ADC_PIN A0
  #define BATTERY_ADC_MAX 1024.0
  #define BATTERY_VOLTAGE_MAX 4.2  // Max Li-ion voltage
  #define BATTERY_VOLTAGE_MIN 3.0  // Min safe Li-ion voltage
  #define BATTERY_VOLTAGE_DIVIDER 4.2  // Adjust based on your voltage divider (R1+R2)/R2
#elif defined(ESP32)
  #define BATTERY_ADC_PIN 36  // GPIO36 (ADC1_CH0)
  #define BATTERY_ADC_MAX 4095.0
  #define BATTERY_VOLTAGE_MAX 4.2
  #define BATTERY_VOLTAGE_MIN 3.0
  #define BATTERY_VOLTAGE_DIVIDER 2.0
#endif

void battery_init();
void battery_update();
float battery_get_voltage();
uint8_t battery_get_percentage();
bool battery_is_low();
bool battery_is_charging();
bool battery_is_connected();
