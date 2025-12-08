#include "../h/Time.h"
#include "config.h"
#include "hal/h/Display.h"
#include "modules/h/NtpClient.h"
#include "modules/h/ConfigStore.h"
#include "modules/h/WeatherManager.h"
#include "modules/h/BatteryManager.h"
#include "modules/h/StatusIcons.h"
#include "modules/h/WifiManager.h"
#include "modules/h/BleServer.h"
#include <Adafruit_GFX.h>

#include "assets/fonts/Org_01.h"
#include "../../themes/DefaultTheme.h"
#include "../../themes/CompactTheme.h"

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

// Helper to convert weather condition to icon code
static uint8_t GetWeatherCode(const String& condition) {
  String cond = condition;
  cond.toLowerCase();
  if (cond.indexOf("rain") >= 0 || cond.indexOf("drizzle") >= 0) return WEATHER_CLOUD_RAIN;
  if (cond.indexOf("snow") >= 0 || cond.indexOf("sleet") >= 0) return WEATHER_CLOUD_SNOW;
  if (cond.indexOf("thunder") >= 0 || cond.indexOf("storm") >= 0) return WEATHER_CLOUD_LIGHTNING;
  if (cond.indexOf("wind") >= 0) return WEATHER_WIND;
  if (cond.indexOf("cloud") >= 0 || cond.indexOf("overcast") >= 0) return WEATHER_CLOUD_SUNNY;
  if (cond.indexOf("sun") >= 0 || cond.indexOf("clear") >= 0) return WEATHER_SUN;
  return WEATHER_CLOUD_SUNNY; // Default
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
  bool btConnected = BleIsConnected();
  
  // Use actual weather data
  uint8_t weatherCode = GetWeatherCode(weatherData.condition);
  char tempStr[10];
  snprintf(tempStr, sizeof(tempStr), "%.1fC", weatherData.temperature_c);
  const char* condStr = weatherData.success ? weatherData.condition.c_str() : "--";
  
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
  bool btConnected = BleIsConnected();
  
  // Use actual weather data
  uint8_t weatherCode = GetWeatherCode(weatherData.condition);
  char tempStr[10];
  snprintf(tempStr, sizeof(tempStr), "%.1fc", weatherData.temperature_c);
  const char* condStr = weatherData.success ? weatherData.condition.c_str() : "--";
  
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