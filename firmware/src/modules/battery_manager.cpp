#include "battery_manager.h"

static float battery_voltage = 0.0;
static uint8_t battery_percentage = 100;
static unsigned long last_update = 0;
static bool is_low = false;
static bool battery_connected = false;

void battery_init() {
  #ifdef ESP8266
    pinMode(BATTERY_ADC_PIN, INPUT);
  #elif defined(ESP32)
    analogReadResolution(12);  // 12-bit ADC
    analogSetAttenuation(ADC_11db);  // Full range ~3.3V
  #endif
  battery_update();
}

void battery_update() {
  unsigned long now = millis();
  if (now - last_update < BATTERY_UPDATE_MS && last_update != 0) return;
  last_update = now;
  
  // Read ADC with averaging
  uint32_t adc_sum = 0;
  for (int i = 0; i < BATTERY_SAMPLES; i++) {
    adc_sum += analogRead(BATTERY_ADC_PIN);
    delay(10);
  }
  float adc_average = adc_sum / (float)BATTERY_SAMPLES;
  
  // Convert ADC to voltage
  #ifdef ESP8266
    // ESP8266 ADC is 0-1V, with voltage divider it can measure higher
    battery_voltage = (adc_average / BATTERY_ADC_MAX) * BATTERY_VOLTAGE_DIVIDER;
  #elif defined(ESP32)
    // ESP32 ADC with 11dB attenuation can measure ~0-3.3V
    // Adjust for voltage divider if using one
    battery_voltage = (adc_average / BATTERY_ADC_MAX) * 3.3 * BATTERY_VOLTAGE_DIVIDER;
  #endif
  
  // Check if battery is connected (voltage should be between reasonable range)
  // If voltage is too low (< 2V) or exactly 0, battery is likely not connected
  if (battery_voltage < 2.0) {
    battery_connected = false;
    battery_percentage = 0;
    is_low = false;
    return;
  }
  
  battery_connected = true;
  
  // Clamp voltage to valid range
  if (battery_voltage > BATTERY_VOLTAGE_MAX) battery_voltage = BATTERY_VOLTAGE_MAX;
  if (battery_voltage < BATTERY_VOLTAGE_MIN) battery_voltage = BATTERY_VOLTAGE_MIN;
  
  // Calculate percentage (Li-ion curve approximation)
  float voltage_range = BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN;
  float voltage_normalized = (battery_voltage - BATTERY_VOLTAGE_MIN) / voltage_range;
  battery_percentage = (uint8_t)(voltage_normalized * 100.0);
  
  // Clamp percentage
  if (battery_percentage > 100) battery_percentage = 100;
  if (battery_percentage < 0) battery_percentage = 0;
  
  // Check low battery (below 20%)
  is_low = (battery_percentage < 20);
}

float battery_get_voltage() {
  return battery_voltage;
}

uint8_t battery_get_percentage() {
  return battery_percentage;
}

bool battery_is_low() {
  return is_low;
}

bool battery_is_charging() {
  // For now, return false. Implement charging detection if needed
  // Could check for voltage rising or use a charge detect pin
  return false;
}

bool battery_is_connected() {
  return battery_connected;
}