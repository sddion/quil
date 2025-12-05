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

// THEME 1: Elaborate Theme (Default) - Previous design
void TimeRenderElaborate() {
  Adafruit_SSD1306& display = DisplayGetDisplay();
  display.clearDisplay();
  
  int hour = NtpGetHour();
  int minute = NtpGetMinute();
  String dateStr = NtpGetDate();
  String dayStr = NtpGetDay();
  
  // Status icons (top row)
  if (WifiIsConnected()) {
    StatusIconsDrawWifi(4, 1, WifiGetRssi());
  }
  StatusIconsDrawBluetooth(27, 1, false);
  if (BatteryIsConnected()) {
    StatusIconsDrawBattery(48, 1, BatteryGetPercentage());
  }
  
  uint8_t weatherCode = WEATHER_CLOUD_SUNNY;
  StatusIconsDrawWeather(108, 1, weatherCode);
  
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(80, 6);
  display.print("Cloudy");
  display.setCursor(80, 16);
  display.print("25.6C");
  
  // Separator lines
  display.drawLine(0, 18, 107, 18, 1);
  display.drawLine(0, 19, 96, 19, 1);
  
  // Big time display
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(5);
  
  char hourStr[3], minStr[3];
  snprintf(hourStr, sizeof(hourStr), "%02d", hour);
  snprintf(minStr, sizeof(minStr), "%02d", minute);
  
  display.setCursor(4, 45);
  display.print(hourStr);
  
  display.setTextSize(3);
  display.setCursor(57, 27);
  display.print(":");
  
  display.setTextSize(5);
  display.setCursor(69, 45);
  display.print(minStr);
  
  // Date bars
  int year = 0, month = 0, day = 0;
  sscanf(dateStr.c_str(), "%d/%d/%d", &year, &month, &day);
  
  const char* monthNames[] = {"", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
                               "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  
  display.fillRect(4, 53, 64, 9, 1);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(5, 54);
  if (month >= 1 && month <= 12) {
    display.print(monthNames[month]);
  }
  
  display.fillRect(71, 53, 53, 9, 1);
  display.setCursor(56, 54);
  display.print(day);
  display.setCursor(76, 54);
  display.print(year);
  display.setCursor(104, 54);
  display.print(dayStr);
  
  DisplayUpdate();
}

// THEME 2: Compact Theme - New simpler design
void TimeRenderCompact() {
  Adafruit_SSD1306& display = DisplayGetDisplay();
  display.clearDisplay();
  
  int hour = NtpGetHour();
  int minute = NtpGetMinute();
  String dateStr = NtpGetDate();
  String dayStr = NtpGetDay();
  
  // WiFi + Bluetooth at top left
  if (WifiIsConnected()) {
    StatusIconsDrawWifi(2, 2, WifiGetRssi());
  }
  StatusIconsDrawBluetooth(25, 2, false);
  
  // Battery at top right
  if (BatteryIsConnected()) {
    StatusIconsDrawBattery(103, 1, BatteryGetPercentage());
  }
  
  // Big time with Org_01 font
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(4);
  display.setFont(&Org_01);
  
  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
  display.setCursor(7, 42);
  display.print(timeStr);
  
  // Date text below time
  int year = 0, month = 0, day = 0;
  sscanf(dateStr.c_str(), "%d/%d/%d", &year, &month, &day);
  
  const char* monthNamesShort[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  
  display.setTextSize(1);
  display.setCursor(22, 56);
  if (month >= 1 && month <= 12) {
    display.print(day);
    display.print(" ");
    display.print(monthNamesShort[month]);
    display.print(" ");
    display.print(dayStr);
  }
  
  // Weather icon at bottom right
  uint8_t weatherCode = WEATHER_CLOUD_RAIN;
  StatusIconsDrawWeather(106, 29, weatherCode);
  
  // Temperature text
  display.setCursor(99, 56);
  display.print("25.6c");
  
  display.setFont();  // Reset to default font
  DisplayUpdate();
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