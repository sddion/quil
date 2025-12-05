#pragma once

#include <Arduino.h>

// Battery voltage monitoring for ESP32
// ESP32: ADC1_CHANNEL_0 (GPIO36) or any ADC pin

#define BATTERY_SAMPLES 10
#define BATTERY_UPDATE_MS 30000  // Update every 30 seconds

#define BATTERY_ADC_PIN 36  // GPIO36 (ADC1_CH0)
#define BATTERY_ADC_MAX 4095.0
#define BATTERY_VOLTAGE_MAX 4.2
#define BATTERY_VOLTAGE_MIN 3.0
#define BATTERY_VOLTAGE_DIVIDER 2.0

void BatteryInit();
void BatteryUpdate();
float BatteryGetVoltage();
uint8_t BatteryGetPercentage();
bool BatteryIsLow();
bool BatteryIsCharging();
bool BatteryIsConnected();