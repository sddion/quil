#include "../h/Time.h"
#include "config.h"
#include "hal/h/Display.h"
#include "modules/h/NtpClient.h"
#include "modules/h/ConfigStore.h"
#include "modules/h/WeatherManager.h"
#include "modules/h/BatteryManager.h"

static WeatherManager weatherManager;
static WeatherData weatherData;
static char weatherApiKey[65];
static char weatherLocation[65];
static unsigned long lastWeatherUpdate = 0;

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
}

void TimeUpdate() {
  NtpUpdate();
  BatteryUpdate();
  if (millis() - lastWeatherUpdate > 900000) { // 15 minutes
    lastWeatherUpdate = millis();
    weatherData = weatherManager.getWeatherData(weatherApiKey, weatherLocation);
  }
}

void TimeRender() {
  DisplayClear();
  
  String time_str = NtpGetTime();
  String date_str = NtpGetDate();
  String day_str = NtpGetDay();
  
  Adafruit_SSD1306& disp = DisplayGetDisplay(); 
  
  // Placeholder rendering until new themes are added
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);
  disp.setCursor(0, 0);
  disp.println(time_str);
  disp.println(date_str);
  disp.println(day_str);

  // Render battery status only if battery is connected
  if (BatteryIsConnected()) {
    uint8_t battery_pct = BatteryGetPercentage();
    // Battery rendering removed as it was part of theme
    disp.setCursor(100, 0);
    disp.print(battery_pct);
    disp.print("%");
  }
  
  DisplayUpdate();
}

void TimeForceRender() {
  TimeRender();
}

void TimeSetTheme(DisplayTheme_t theme) {
  // Theme switching disabled
  ConfigSaveTheme((uint8_t)theme);
}