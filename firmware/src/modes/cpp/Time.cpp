#include "../h/Time.h"
#include "config.h"
#include "hal/h/Display.h"
#include "modules/h/NtpClient.h"
#include "modules/h/ConfigStore.h"
#include "modules/h/WeatherManager.h"
#include "modules/h/BatteryManager.h"
#include "modules/h/StatusIcons.h"
#include "modules/h/WifiManager.h"
#include <Adafruit_GFX.h>

#include "assets/fonts/Org_01.h"

static WeatherManager weatherManager;
static WeatherData weatherData;
static char weatherApiKey[65];
static char weatherLocation[65];
static unsigned long lastWeatherUpdate = 0;
static DisplayTheme_t currentTheme = THEME_DEFAULT;  // Default theme

void TimeInit() {
  NtpInit();
  
  // Load and apply timezone offset
  int tz_offset = NTP_OFFSET_SEC;
  if (ConfigLoadTimezone(&tz_offset)) {
    NtpSetTimezone(tz_offset);
  }
  
  BatteryInit();
  
  if (!ConfigLoadWeather(weatherApiKey, weatherLocation)) {
    // No saved weather config - initialize to empty
    weatherApiKey[0] = '\0';
    weatherLocation[0] = '\0';
  }
  
  // Load saved theme
  uint8_t savedTheme = 0;
  if (ConfigLoadTheme(&savedTheme)) {
    currentTheme = (DisplayTheme_t)savedTheme;
  }
}

void TimeUpdate() {
  NtpUpdate();
  BatteryUpdate();
  if (millis() - lastWeatherUpdate > 900000) { // 15 minutes
    lastWeatherUpdate = millis();
    weatherData = weatherManager.getWeatherData(weatherApiKey, weatherLocation);
  }
}

// THEME 1: Elaborate Theme (Default)
void TimeRenderElaborate() {
  int hour = NtpGetHour();
  int minute = NtpGetMinute();
  String dateStr = NtpGetDate();
  String dayStr = NtpGetDay();
  
  uint8_t batteryPct = BatteryGetPercentage();
  int rssi = WifiGetRssi();
  bool wifiConnected = WifiIsConnected();
  bool btConnected = false; // Placeholder until BT manager exists
  
  uint8_t weatherCode = WEATHER_CLOUD_SUNNY; // Placeholder
  const char* tempStr = "25.6C";
  const char* condStr = "Cloudy";
  
  DefaultThemeRender(hour, minute, dateStr.c_str(), dayStr.c_str(), 
                    batteryPct, rssi, wifiConnected, btConnected,
                    weatherCode, tempStr, condStr);
}

// THEME 2: Compact Theme
void TimeRenderCompact() {
  int hour = NtpGetHour();
  int minute = NtpGetMinute();
  String dateStr = NtpGetDate();
  String dayStr = NtpGetDay();
  
  uint8_t batteryPct = BatteryGetPercentage();
  int rssi = WifiGetRssi();
  bool wifiConnected = WifiIsConnected();
  bool btConnected = false;
  
  uint8_t weatherCode = WEATHER_CLOUD_RAIN; // Placeholder
  const char* tempStr = "25.6c";
  const char* condStr = "Rainy";
  
  CompactThemeRender(hour, minute, dateStr.c_str(), dayStr.c_str(), 
                    batteryPct, rssi, wifiConnected, btConnected,
                    weatherCode, tempStr, condStr);
}

void TimeRender() {
  if (currentTheme == THEME_COMPACT) {
    TimeRenderCompact();
  } else {
    TimeRenderElaborate();  // Default
  }
}

void TimeForceRender() {
  TimeRender();
}

void TimeSetTheme(DisplayTheme_t theme) {
  currentTheme = theme;
  ConfigSaveTheme((uint8_t)theme);
}