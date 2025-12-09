#pragma once
#include <Arduino.h>

// Icon dimensions
#define BATTERY_ICON_WIDTH 24
#define BATTERY_ICON_HEIGHT 16
#define WIFI_ICON_WIDTH 19
#define WIFI_ICON_HEIGHT 16
#define WEATHER_ICON_MAX_WIDTH 34
#define WEATHER_ICON_MAX_HEIGHT 26

// Weather codes
#define WEATHER_SUN 0
#define WEATHER_CLOUD_SUNNY 1
#define WEATHER_CLOUD_RAIN 2
#define WEATHER_CLOUD_SNOW 3
#define WEATHER_CLOUD_LIGHTNING 4
#define WEATHER_WIND 5

void StatusIconsDrawBattery(int16_t x, int16_t y, uint8_t percentage);
void StatusIconsDrawWifi(int16_t x, int16_t y, int rssi);
void StatusIconsDrawWeather(int16_t x, int16_t y, uint8_t weatherCode);
